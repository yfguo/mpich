/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_AM_IMPL_H_INCLUDED
#define POSIX_AM_IMPL_H_INCLUDED

#include "posix_types.h"
#include "posix_impl.h"
#include "mpidu_genq.h"

static inline int MPIDI_POSIX_am_release_req_hdr(MPIDI_POSIX_am_request_header_t ** req_hdr_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_RELEASE_REQ_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_RELEASE_REQ_HDR);

    if ((*req_hdr_ptr)->am_hdr != &(*req_hdr_ptr)->am_hdr_buf[0]) {
        MPL_free((*req_hdr_ptr)->am_hdr);
    }
#ifndef POSIX_AM_REQUEST_INLINE
    MPIDU_genq_private_pool_free_cell(MPIDI_POSIX_global.am_hdr_buf_pool, (*req_hdr_ptr));
#endif

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_RELEASE_REQ_HDR);
    return mpi_errno;
}

static inline int MPIDI_POSIX_am_init_req_hdr(const void *am_hdr,
                                              size_t am_hdr_sz,
                                              MPIDI_POSIX_am_request_header_t ** req_hdr_ptr,
                                              MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_am_request_header_t *req_hdr = *req_hdr_ptr;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_INIT_REQ_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_INIT_REQ_HDR);

#ifdef POSIX_AM_REQUEST_INLINE
    if (req_hdr == NULL && sreq != NULL) {
        req_hdr = &MPIDI_POSIX_AMREQUEST(sreq, req_hdr_buffer);
    }
#endif /* POSIX_AM_REQUEST_INLINE */

    if (req_hdr == NULL) {
        MPIDU_genq_private_pool_alloc_cell(MPIDI_POSIX_global.am_hdr_buf_pool, (void **) &req_hdr);
        MPIR_ERR_CHKANDJUMP(!req_hdr, mpi_errno, MPI_ERR_OTHER, "**nomem");

        req_hdr->am_hdr = (void *) &req_hdr->am_hdr_buf[0];
        req_hdr->am_hdr_sz = MPIDI_POSIX_MAX_AM_HDR_SIZE;

        req_hdr->pack_buffer = NULL;

    }

    /* If the header is larger than what we'd preallocated, get rid of the preallocated buffer and
     * create a new one of the correct size. */
    if (am_hdr_sz > req_hdr->am_hdr_sz) {
        if (req_hdr->am_hdr != &req_hdr->am_hdr_buf[0])
            MPL_free(req_hdr->am_hdr);

        req_hdr->am_hdr = MPL_malloc(am_hdr_sz, MPL_MEM_SHM);
        MPIR_ERR_CHKANDJUMP(!(req_hdr->am_hdr), mpi_errno, MPI_ERR_OTHER, "**nomem");
    }

    if (am_hdr) {
        MPIR_Typerep_copy(req_hdr->am_hdr, am_hdr, am_hdr_sz);
    }

    req_hdr->am_hdr_sz = am_hdr_sz;
    *req_hdr_ptr = req_hdr;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_INIT_REQ_HDR);
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_POSIX_eager_payload_limit(void)
{
    return MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_CELL_SIZE - sizeof(MPIDI_POSIX_eager_iqueue_cell_t);
}

static inline size_t MPIDI_POSIX_eager_buf_limit(void)
{
    return MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_CELL_SIZE;
}

/* This function attempts to send the next chunk of a message via the queue. If no cells are
 * available, this function will return and the caller is expected to queue the message for later
 * and retry.
 *
 * grank   - The global rank (the rank in MPI_COMM_WORLD) of the receiving process.
 * msg_hdr - The header of the message to be sent. This can be NULL if there is no header to be sent
 *           (such as if the header was sent in a previous chunk).
 * iov     - The array of iovec entries to be sent.
 * iov_num - The number of entries in the iovec array.
 */
static inline int MPIDI_POSIX_eager_send(int grank, MPIDI_POSIX_am_header_t ** msg_hdr,
                                         struct iovec **iov, size_t * iov_num)
{
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell;
    MPIDU_genq_shmem_queue_t terminal;
    size_t i, iov_done, capacity, available;
    char *payload;
    int ret = MPIDI_POSIX_OK;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_EAGER_SEND);

    /* Get the transport object that holds all of the global variables. */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    /* TODO: This function can only send one cell at a time. For a large
     * message that require multiple cells this function has to be invoked
     * multiple times. Can we try to send all data in this call? Moreover,
     * we have to traverse the transport to find an empty cell, can we find
     * all needed cells in single traversal? When putting them into the terminal,
     * can we put all in single insertion? */

    /* Try to get a new cell to hold the message */
    MPIDU_genq_shmem_pool_cell_alloc(transport->cell_pool, (void **) &cell);

    /* If a cell wasn't available, let the caller know that we weren't able to send the message
     * immediately. */
    if (unlikely(!cell)) {
        ret = MPIDI_POSIX_NOK;
        goto fn_exit;
    }

    /* Find the correct queue for this rank pair. */
    terminal = &transport->terminals[MPIDI_POSIX_global.local_ranks[grank]];

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);

    /* Figure out the capacity of each cell */
    capacity = MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport);
    available = capacity;

    cell->from = MPIDI_POSIX_global.my_local_rank;

    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    if (*msg_hdr) {
        cell->am_header = **msg_hdr;
        *msg_hdr = NULL;        /* completed header copy */
        cell->type = MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_HDR;
    } else {
        cell->type = MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_DATA;
    }

    /* Pack the data into the cells */
    iov_done = 0;
    for (i = 0; i < *iov_num; i++) {
        /* Optimize for the case where the message will fit into the cell. */
        if (likely(available >= (*iov)[i].iov_len)) {
            MPIR_Typerep_copy(payload, (*iov)[i].iov_base, (*iov)[i].iov_len);

            payload += (*iov)[i].iov_len;
            available -= (*iov)[i].iov_len;

            iov_done++;
        } else {
            /* If the message won't fit, put as much as we can and update the iovec for the next
             * time around. */
            MPIR_Typerep_copy(payload, (*iov)[i].iov_base, available);

            (*iov)[i].iov_base = (char *) (*iov)[i].iov_base + available;
            (*iov)[i].iov_len -= available;

            available = 0;

            break;
        }
    }

    cell->payload_size = capacity - available;

    MPIDU_genq_shmem_queue_enqueue(terminal, (void *) cell);

    /* Update the user counter for number of iovecs left */
    *iov_num -= iov_done;

    /* Check to see if we finished all of the iovecs that came from the caller. If not, update
     * the iov pointer. If so, set it to NULL. Either way, the caller will know the status of
     * the operation from the value of iov. */
    if (*iov_num) {
        *iov = &((*iov)[iov_done]);
    } else {
        *iov = NULL;
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    return ret;
}

static inline int MPIDI_POSIX_eager_recv_begin(MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell = NULL;
    int ret = MPIDI_POSIX_NOK;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_EAGER_RECV_BEGIN);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_EAGER_RECV_BEGIN);

    /* Get the transport with the global variables */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    MPIDU_genq_shmem_queue_dequeue(transport->my_terminal, (void **) &cell);

    if (cell) {
        transaction->src_grank = MPIDI_POSIX_global.local_procs[cell->from];
        transaction->payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);
        transaction->payload_sz = cell->payload_size;

        if (likely(cell->type == MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_HDR)) {
            transaction->msg_hdr = &cell->am_header;
        } else {
            MPIR_Assert(cell->type == MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_DATA);
            transaction->msg_hdr = NULL;
        }

        transaction->transport.pointer_to_cell = cell;

        ret = MPIDI_POSIX_OK;
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_EAGER_RECV_BEGIN);
    return ret;
}

static inline void MPIDI_POSIX_eager_recv_memcpy(MPIDI_POSIX_eager_recv_transaction_t * transaction,
                                                 void *dst, const void *src, size_t size)
{
    MPIR_Typerep_copy(dst, src, size);
}

static inline void MPIDI_POSIX_eager_recv_commit(MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    MPIDI_POSIX_eager_iqueue_cell_t *cell;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_EAGER_RECV_COMMIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_EAGER_RECV_COMMIT);

    cell = (MPIDI_POSIX_eager_iqueue_cell_t *) transaction->transport.pointer_to_cell;
    MPIDU_genq_shmem_pool_cell_free(cell);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_EAGER_RECV_COMMIT);
}

#endif /* POSIX_AM_IMPL_H_INCLUDED */
