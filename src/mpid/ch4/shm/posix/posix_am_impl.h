/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_AM_IMPL_H_INCLUDED
#define POSIX_AM_IMPL_H_INCLUDED

#include "posix_types.h"
#include "posix_impl.h"
#include "mpidu_genq.h"

int MPIDI_POSIX_deferred_am_isend_issue(MPIDI_POSIX_deferred_am_isend_req_t * dreq);
void MPIDI_POSIX_deferred_am_isend_dequeue(MPIDI_POSIX_deferred_am_isend_req_t * dreq);

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

static inline int MPIDI_POSIX_deferred_am_isend_enqueue(int op, int grank, const void *buf,
                                                        MPI_Aint count, MPI_Datatype datatype,
                                                        MPIDI_POSIX_am_header_t * msg_hdr,
                                                        const void *am_hdr, int header_only,
                                                        MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_deferred_am_isend_req_t *dreq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ENQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ENQUEUE);

    dreq = (MPIDI_POSIX_deferred_am_isend_req_t *)
        MPL_malloc(sizeof(MPIDI_POSIX_deferred_am_isend_req_t), MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!dreq, mpi_errno, MPI_ERR_OTHER, "**nomem");

    dreq->op = op;
    dreq->grank = grank;
    dreq->buf = buf;
    dreq->count = count;
    dreq->datatype = datatype;
    dreq->sreq = sreq;
    dreq->header_only = header_only;
    memcpy(&(dreq->msg_hdr), msg_hdr, sizeof(MPIDI_POSIX_am_header_t));
    dreq->am_hdr = MPL_malloc(msg_hdr->am_hdr_sz, MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!dreq->am_hdr, mpi_errno, MPI_ERR_OTHER, "**nomem");
    memcpy(dreq->am_hdr, am_hdr, msg_hdr->am_hdr_sz);

    DL_APPEND(MPIDI_POSIX_global.deferred_am_isend_q, dreq);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ENQUEUE);
    return mpi_errno;
  fn_fail:
    MPL_free(dreq);
    goto fn_exit;
}

/* This function is for sending eager message through iqueue. The function requires the user data
 * that needs to be sent to be smaller than the eager send limit.
 * Its parameters are
 * data - pointer to user buffer
 * count - count of user data
 * datatype - datatype of user buffer
 * grank - translated global rank of target process
 * msg_hdr - CH4 POSIX level am message header
 * am_hdr - CH4 levelm am message header
 * MPIR_Request - send request
 */
static inline int MPIDI_POSIX_eager_send(const void *data, MPI_Aint count, MPI_Datatype datatype,
                                         int grank, MPIDI_POSIX_am_header_t * msg_hdr,
                                         const void *am_hdr, int header_only, MPIR_Request * sreq)
{
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell;
    MPIDU_genq_shmem_queue_t terminal;
    int mpi_errno = MPI_SUCCESS;
    char *payload;
    int dt_contig = 0;
    size_t data_sz = 0;
    MPIR_Datatype *dt_ptr = NULL;
    MPI_Aint dt_true_lb = 0;
    uint8_t *send_buf = NULL;
    bool need_packing = false;
    bool data_lt_eager_threshold = false;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_EAGER_SEND);

    /* Get the transport object that holds all of the global variables. */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    if (MPIDI_POSIX_global.deferred_am_isend_q) {
        /* if the deferred queue is not empty, all new ops must be deferred to maintain ordering */
        mpi_errno = MPIDI_POSIX_deferred_am_isend_enqueue(MPIDI_POSIX_DEFERRED_AM_ISEND_OP__EAGER,
                                                          grank, data, count, datatype, msg_hdr,
                                                          am_hdr, header_only, sreq);
        MPIR_ERR_CHECK(mpi_errno);
        goto fn_exit;
    }

    /* Try to get a new cell to hold the message */
    mpi_errno = MPIDU_genq_shmem_pool_cell_alloc(transport->cell_pool, (void **) &cell);

    /* If a cell wasn't available, let the caller know that we weren't able to send the message
     * immediately. */
    if (unlikely(!cell)) {
        mpi_errno = MPIDI_POSIX_deferred_am_isend_enqueue(MPIDI_POSIX_DEFERRED_AM_ISEND_OP__EAGER,
                                                          grank, data, count, datatype, msg_hdr,
                                                          am_hdr, header_only, sreq);
        MPIR_ERR_CHECK(mpi_errno);
        goto fn_exit;
    }
    MPIR_ERR_CHECK(mpi_errno);

    /* setting POSIX am header in the cell */
    cell->from = MPIDI_POSIX_global.my_local_rank;
    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    mpi_errno = MPIR_Typerep_copy(&(cell->am_header), msg_hdr, sizeof(MPIDI_POSIX_am_header_t));
    MPIR_ERR_CHECK(mpi_errno);

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);

    /* first piece in copy to the payload is the CH4 level AM header */
    mpi_errno = MPIR_Typerep_copy(payload, am_hdr, msg_hdr->am_hdr_sz);
    MPIR_ERR_CHECK(mpi_errno);
    payload += msg_hdr->am_hdr_sz;

    /* check user datatype */
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    need_packing = dt_contig ? false : true;
    send_buf = (uint8_t *) data + dt_true_lb;

    /* make sure the data + CH4 am_hdr can fix in the cell */
    MPIR_Assert(data_sz + msg_hdr->am_hdr_sz <= MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport));

    cell->am_header.data_sz = data_sz;

    /* check buffer location, we will need packing if user buffer on GPU */
    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(data, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* next is user buffer, we pack into the payload if packing is needed.
     * otherwise just regular copy */
    if (unlikely(need_packing)) {
        MPIDI_POSIX_AMREQUEST(sreq, req_hdr) = NULL;
        /* Prepare private storage with information about the pack buffer. */
        mpi_errno = MPIDI_POSIX_am_init_req_hdr(am_hdr, msg_hdr->am_hdr_sz,
                                                &MPIDI_POSIX_AMREQUEST(sreq, req_hdr), sreq);
        MPIR_ERR_CHECK(mpi_errno);
        MPIDI_POSIX_AMREQUEST(sreq, req_hdr)->pack_buffer = payload;

        MPI_Aint actual_pack_bytes;
        mpi_errno = MPIR_Typerep_pack(data, count, datatype, 0, payload, data_sz,
                                      &actual_pack_bytes);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_Assert(actual_pack_bytes == data_sz);
    } else {
        mpi_errno = MPIR_Typerep_copy(payload, send_buf, data_sz);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* Find the correct queue for this rank pair. */
    terminal = &transport->terminals[MPIDI_POSIX_global.local_ranks[grank]];

    mpi_errno = MPIDU_genq_shmem_queue_enqueue(terminal, (void *) cell);
    MPIR_ERR_CHECK(mpi_errno);

    if (!header_only) {
        mpi_errno = MPIDIG_global.origin_cbs[msg_hdr->handler_id] (sreq);
        MPIR_ERR_CHECK(mpi_errno);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_POSIX_pipeline_send(const void *data, MPI_Aint count, MPI_Datatype datatype,
                                            int grank, MPIDI_POSIX_am_header_t * msg_hdr,
                                            const void *am_hdr, MPIR_Request * sreq)
{
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell;
    MPIDU_genq_shmem_queue_t terminal;
    int mpi_errno = MPI_SUCCESS;
    int c = 0;
    char *payload;
    int dt_contig = 0;
    size_t data_sz = 0, send_size = 0;
    MPIR_Datatype *dt_ptr = NULL;
    MPI_Aint dt_true_lb = 0;
    uint8_t *send_buf = NULL;
    bool need_packing = false;
    bool data_lt_eager_threshold = false;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_EAGER_SEND);

    /* Get the transport object that holds all of the global variables. */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    if (MPIDI_POSIX_global.deferred_am_isend_q) {
        /* if the deferred queue is not empty, all new ops must be deferred to maintain ordering */
        goto op_deferred;
    }

    /* Try to get a new cell to hold the message */
    mpi_errno = MPIDU_genq_shmem_pool_cell_alloc(transport->cell_pool, (void **) &cell);

    /* If a cell wasn't available, let the caller know that we weren't able to send the message
     * immediately. */
    if (unlikely(!cell)) {
        goto op_deferred;
    }
    MPIR_ERR_CHECK(mpi_errno);

    /* setting POSIX am header in the cell */
    cell->from = MPIDI_POSIX_global.my_local_rank;
    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    mpi_errno = MPIR_Typerep_copy(&(cell->am_header), msg_hdr, sizeof(MPIDI_POSIX_am_header_t));
    MPIR_ERR_CHECK(mpi_errno);

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);

    /* first piece in copy to the payload is the CH4 level AM header */
    mpi_errno = MPIR_Typerep_copy(payload, am_hdr, msg_hdr->am_hdr_sz);
    MPIR_ERR_CHECK(mpi_errno);
    payload += msg_hdr->am_hdr_sz;

    /* check user datatype */
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    need_packing = dt_contig ? false : true;
    send_size = MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport) - msg_hdr->am_hdr_sz;
    send_buf = (uint8_t *) data + dt_true_lb;

    send_size = MPL_MIN(MPIDIG_REQUEST(sreq, req->lreq).data_sz_left, send_size);

    /* check buffer location, we will need packing if user buffer on GPU */
    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(data, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* next is user buffer, we pack into the payload if packing is needed.
     * otherwise just regular copy */
    if (unlikely(need_packing)) {
        MPIDI_POSIX_AMREQUEST(sreq, req_hdr) = NULL;
        /* Prepare private storage with information about the pack buffer. */
        mpi_errno = MPIDI_POSIX_am_init_req_hdr(am_hdr, msg_hdr->am_hdr_sz,
                                                &MPIDI_POSIX_AMREQUEST(sreq, req_hdr), sreq);
        MPIR_ERR_CHECK(mpi_errno);
        MPIDI_POSIX_AMREQUEST(sreq, req_hdr)->pack_buffer = payload;

        MPI_Aint actual_pack_bytes;
        mpi_errno = MPIR_Typerep_pack(data, count, datatype, MPIDIG_REQUEST(sreq, req->lreq).offset,
                                      payload, send_size, &actual_pack_bytes);
        MPIR_ERR_CHECK(mpi_errno);
        send_size = actual_pack_bytes;
    } else {
        mpi_errno = MPIR_Typerep_copy(payload, send_buf, send_size);
        MPIR_ERR_CHECK(mpi_errno);
    }

    cell->am_header.data_sz = send_size;

    /* Find the correct queue for this rank pair. */
    terminal = &transport->terminals[MPIDI_POSIX_global.local_ranks[grank]];

    mpi_errno = MPIDU_genq_shmem_queue_enqueue(terminal, (void *) cell);
    MPIR_ERR_CHECK(mpi_errno);

    MPIDIG_REQUEST(sreq, req->lreq).data_sz_left -= send_size;
    MPIDIG_REQUEST(sreq, req->lreq).offset += send_size;

  op_deferred:
    mpi_errno = MPIDI_POSIX_deferred_am_isend_enqueue(MPIDI_POSIX_DEFERRED_AM_ISEND_OP__PIPELINE,
                                                      grank, data, count, datatype, msg_hdr, am_hdr,
                                                      0, sreq);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_EAGER_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
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

        transaction->msg_hdr = &cell->am_header;

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
