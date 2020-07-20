/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "posix_impl.h"
#include "posix_am_impl.h"
#include "posix_noinline.h"

static int issue_deferred_am_isend_eager(MPIDI_POSIX_deferred_am_isend_req_t * dreq);
static int issue_deferred_am_isend_pipeline(MPIDI_POSIX_deferred_am_isend_req_t * dreq);

void MPIDI_POSIX_deferred_am_isend_dequeue(MPIDI_POSIX_deferred_am_isend_req_t * dreq)
{
    MPIDI_POSIX_deferred_am_isend_req_t *curr_req = dreq;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_DEQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_DEQUEUE);

    DL_DELETE(MPIDI_POSIX_global.deferred_am_isend_q, curr_req);
    MPL_free(curr_req->am_hdr);
    MPL_free(curr_req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_DEQUEUE);
}

static int issue_deferred_am_isend_eager(MPIDI_POSIX_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell = NULL;
    MPIDU_genq_shmem_queue_t terminal;
    char *payload;
    int dt_contig = 0;
    size_t data_sz = 0, send_size = 0;
    MPIR_Datatype *dt_ptr = NULL;
    MPI_Aint dt_true_lb = 0;
    uint8_t *send_buf = NULL;
    bool need_packing = false;
    bool data_lt_eager_threshold = false;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_EAGER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_EAGER);

    /* Get the transport object that holds all of the global variables. */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    /* Try to get a new cell to hold the message */
    mpi_errno = MPIDU_genq_shmem_pool_cell_alloc(transport->cell_pool, (void **) &cell);

    /* If a cell wasn't available, suppress error and return */
    if (unlikely(!cell)) {
        mpi_errno = MPI_SUCCESS;
        goto fn_exit;
    }
    MPIR_ERR_CHECK(mpi_errno);

    /* setting POSIX am header in the cell */
    cell->from = MPIDI_POSIX_global.my_local_rank;
    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    mpi_errno = MPIR_Typerep_copy(&(cell->am_header), &(dreq->msg_hdr),
                                  sizeof(MPIDI_POSIX_am_header_t));
    MPIR_ERR_CHECK(mpi_errno);

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);

    /* first piece in copy to the payload is the CH4 level AM header */
    mpi_errno = MPIR_Typerep_copy(payload, dreq->am_hdr, dreq->msg_hdr.am_hdr_sz);
    MPIR_ERR_CHECK(mpi_errno);
    payload += dreq->msg_hdr.am_hdr_sz;

    /* check user datatype */
    MPIDI_Datatype_get_info(dreq->count, dreq->datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    need_packing = dt_contig ? false : true;
    send_buf = (uint8_t *) dreq->buf + dt_true_lb;

    /* make sure the data + CH4 am_hdr can fix in the cell */
    MPIR_Assert(data_sz + dreq->msg_hdr.am_hdr_sz
                <= MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport));

    cell->am_header.data_sz = data_sz;

    /* check buffer location, we will need packing if user buffer on GPU */
    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(dreq->buf, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* next is user buffer, we pack into the payload if packing is needed.
     * otherwise just regular copy */
    if (unlikely(need_packing)) {
        MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr) = NULL;
        /* Prepare private storage with information about the pack buffer. */
        mpi_errno = MPIDI_POSIX_am_init_req_hdr(dreq->am_hdr, dreq->msg_hdr.am_hdr_sz,
                                                &MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr),
                                                dreq->sreq);
        MPIR_ERR_CHECK(mpi_errno);
        MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr)->pack_buffer = payload;

        MPI_Aint actual_pack_bytes;
        mpi_errno = MPIR_Typerep_pack(dreq->buf, dreq->count, dreq->datatype, 0, payload, data_sz,
                                      &actual_pack_bytes);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_Assert(actual_pack_bytes == data_sz);
    } else {
        mpi_errno = MPIR_Typerep_copy(payload, send_buf, data_sz);
        MPIR_ERR_CHECK(mpi_errno);
    }

    /* Find the correct queue for this rank pair. */
    terminal = &transport->terminals[MPIDI_POSIX_global.local_ranks[dreq->grank]];

    mpi_errno = MPIDU_genq_shmem_queue_enqueue(terminal, (void *) cell);
    MPIR_ERR_CHECK(mpi_errno);

    MPIDI_POSIX_deferred_am_isend_dequeue(dreq);

    /* FIXME: move to progress */
    if (!dreq->header_only) {
        mpi_errno = MPIDIG_global.origin_cbs[dreq->msg_hdr.handler_id] (dreq->sreq);
        MPIR_ERR_CHECK(mpi_errno);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_EAGER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static int issue_deferred_am_isend_pipeline(MPIDI_POSIX_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell;
    MPIDU_genq_shmem_queue_t terminal;
    char *payload;
    int dt_contig = 0;
    char *send_buf;
    MPI_Aint data_sz;
    MPI_Aint dt_true_lb, send_size;
    MPIR_Datatype *dt_ptr;
    bool data_lt_eager_threshold = false;
    bool need_packing = false;
    MPIR_Request *child_sreq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_PIPELINE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_PIPELINE);

    /* Get the transport object that holds all of the global variables. */
    transport = &MPIDI_POSIX_eager_iqueue_transport_global;

    /* Try to get a new cell to hold the message */
    mpi_errno = MPIDU_genq_shmem_pool_cell_alloc(transport->cell_pool, (void **) &cell);

    /* If a cell wasn't available, suppress error and return. */
    if (unlikely(!cell)) {
        mpi_errno = MPI_SUCCESS;
        goto fn_exit;
    }
    MPIR_ERR_CHECK(mpi_errno);

    /* setting POSIX am header in the cell */
    cell->from = MPIDI_POSIX_global.my_local_rank;
    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    mpi_errno = MPIR_Typerep_copy(&(cell->am_header), &(dreq->msg_hdr),
                                  sizeof(MPIDI_POSIX_am_header_t));
    MPIR_ERR_CHECK(mpi_errno);

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);

    /* first piece in copy to the payload is the CH4 level AM header */
    mpi_errno = MPIR_Typerep_copy(payload, dreq->am_hdr, dreq->msg_hdr.am_hdr_sz);
    MPIR_ERR_CHECK(mpi_errno);
    payload += dreq->msg_hdr.am_hdr_sz;

    /* check user datatype */
    MPIDI_Datatype_get_info(dreq->count, dreq->datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    need_packing = dt_contig ? false : true;
    send_size = MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport) - dreq->msg_hdr.am_hdr_sz;
    send_buf = (uint8_t *) dreq->buf + dt_true_lb;

    send_size = MPL_MIN(MPIDIG_REQUEST(dreq->sreq, req->lreq).data_sz_left, send_size);

    /* check buffer location, we will need packing if user buffer on GPU */
    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(dreq->buf, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* next is user buffer, we pack into the payload if packing is needed.
     * otherwise just regular copy */
    if (unlikely(need_packing)) {
        MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr) = NULL;
        /* Prepare private storage with information about the pack buffer. */
        mpi_errno = MPIDI_POSIX_am_init_req_hdr(dreq->am_hdr, dreq->msg_hdr.am_hdr_sz,
                                                &MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr),
                                                dreq->sreq);
        MPIR_ERR_CHECK(mpi_errno);
        MPIDI_POSIX_AMREQUEST(dreq->sreq, req_hdr)->pack_buffer = payload;

        MPI_Aint actual_pack_bytes;
        mpi_errno = MPIR_Typerep_pack(dreq->buf, dreq->count, dreq->datatype,
                                      MPIDIG_REQUEST(dreq->sreq, req->lreq).offset,
                                      payload, send_size, &actual_pack_bytes);
        MPIR_ERR_CHECK(mpi_errno);
        send_size = actual_pack_bytes;
    } else {
        send_buf += MPIDIG_REQUEST(dreq->sreq, req->lreq).offset;
        mpi_errno = MPIR_Typerep_copy(payload, send_buf, send_size);
        MPIR_ERR_CHECK(mpi_errno);
    }

    cell->am_header.data_sz = send_size;

    /* Find the correct queue for this rank pair. */
    terminal = &transport->terminals[MPIDI_POSIX_global.local_ranks[dreq->grank]];

    mpi_errno = MPIDU_genq_shmem_queue_enqueue(terminal, (void *) cell);
    MPIR_ERR_CHECK(mpi_errno);

    MPIDIG_REQUEST(dreq->sreq, req->lreq).data_sz_left -= send_size;
    MPIDIG_REQUEST(dreq->sreq, req->lreq).offset += send_size;

    if (MPIDIG_REQUEST(dreq->sreq, req->lreq).data_sz_left == 0) {
        /* only dequeue when all the data is sent */
        MPIDI_POSIX_deferred_am_isend_dequeue(dreq);
        /* FIXME: move to progress */
        mpi_errno = MPIDIG_global.origin_cbs[dreq->msg_hdr.handler_id] (dreq->sreq);
        MPIR_ERR_CHECK(mpi_errno);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_ISSUE_DEFERRED_AM_ISEND_PIPELINE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_POSIX_deferred_am_isend_issue(MPIDI_POSIX_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ISSUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ISSUE);

    switch (dreq->op) {
        case MPIDI_POSIX_DEFERRED_AM_ISEND_OP__EAGER:
            mpi_errno = issue_deferred_am_isend_eager(dreq);
            break;
        case MPIDI_POSIX_DEFERRED_AM_ISEND_OP__PIPELINE:
            mpi_errno = issue_deferred_am_isend_pipeline(dreq);
            break;
        default:
            MPIR_Assert(0);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_DEFERRED_AM_ISEND_ISSUE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
