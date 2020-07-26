/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "ofi_impl.h"
#include "ofi_am_impl.h"
#include "ofi_noinline.h"

static int issue_deferred_am_isend_eager(MPIDI_OFI_deferred_am_isend_req_t * dreq);
static int issue_deferred_am_isend_pipeline(MPIDI_OFI_deferred_am_isend_req_t * dreq);

void MPIDI_OFI_deferred_am_isend_dequeue(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    MPIDI_OFI_deferred_am_isend_req_t *curr_req = dreq;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);

    DL_DELETE(MPIDI_OFI_global.deferred_am_isend_q, curr_req);
    MPL_free(curr_req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);
}

int issue_deferred_am_isend_eager(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;
    int dt_contig = 0;
    MPI_Aint data_sz, packed_size;
    MPI_Aint dt_true_lb, send_size;
    MPIR_Datatype *dt_ptr;
    char *send_buf;
    bool need_packing = false;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);

    MPIDI_Datatype_get_info(dreq->count, dreq->datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_size = MPIDI_OFI_DEFAULT_SHORT_SEND_SIZE - MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, am_hdr_sz)
        - sizeof(MPIDI_OFI_am_header_t);
    send_buf = (char *) dreq->buf + dt_true_lb;

    need_packing = dt_contig ? false : true;

    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(dreq->buf, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV && !MPIDI_OFI_ENABLE_HMEM) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* how much data need to be send, max eager size or what is left */
    send_size = MPL_MIN(MPIDIG_REQUEST(dreq->sreq, data_sz_left), send_size);

    if (need_packing) {
        /* FIXME: currently we always do packing, also for high density types. However,
         * we should not do packing unless needed. Also, for large low-density types
         * we should not allocate the entire buffer and do the packing at once. */
        /* TODO: (1) Skip packing for high-density datatypes;
         *       (2) Pipeline allocation for low-density datatypes; */
        MPIDU_genq_private_pool_alloc_cell(MPIDI_OFI_global.pack_buf_pool, (void **) &send_buf);
        if (send_buf == NULL) {
            /* no buffer available, suppress error and exit */
            goto fn_exit;
        }
        mpi_errno = MPIR_Typerep_pack(dreq->buf, dreq->count, dreq->datatype,
                                      MPIDIG_REQUEST(dreq->sreq, offset), send_buf, dreq->data_sz,
                                      &packed_size);
        MPIR_ERR_CHECK(mpi_errno);
        send_size = packed_size;

        MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, pack_buffer) = send_buf;
    } else {
        send_buf = (char *) send_buf + MPIDIG_REQUEST(dreq->sreq, offset);
        MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, pack_buffer) = NULL;
    }

    mpi_errno = MPIDI_OFI_am_isend_short(dreq->rank, dreq->comm, dreq->handler_id, send_buf,
                                         send_size, dreq->sreq);

    MPIR_ERR_CHECK(mpi_errno);

    MPIDIG_REQUEST(dreq->sreq, data_sz_left) -= send_size;
    MPIDIG_REQUEST(dreq->sreq, offset) += send_size;

    MPIDI_OFI_deferred_am_isend_dequeue(dreq);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static int issue_deferred_am_isend_pipeline(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;
    int c = 0;
    int dt_contig = 0;
    char *send_buf;
    MPI_Aint data_sz, packed_size;
    MPI_Aint dt_true_lb, send_size;
    MPIR_Datatype *dt_ptr;
    bool data_lt_eager_threshold = false;
    bool need_packing = false;
    MPIR_Request *child_sreq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_PIPELINE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_PIPELINE);

    MPIDI_Datatype_get_info(dreq->count, dreq->datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_size = MPIDI_OFI_DEFAULT_SHORT_SEND_SIZE - MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, am_hdr_sz)
        - sizeof(MPIDI_OFI_am_header_t);
    need_packing = dt_contig ? false : true;

    MPL_pointer_attr_t attr;
    MPIR_GPU_query_pointer_attr(dreq->buf, &attr);
    if (attr.type == MPL_GPU_POINTER_DEV && !MPIDI_OFI_ENABLE_HMEM) {
        /* Force packing of GPU buffer in host memory */
        need_packing = true;
    }

    /* check if we can get a pack buffer if needed. This is to avoid create a child request if
     * no pack buffer can be allocated. */
    if (need_packing) {
        mpi_errno = MPIDU_genq_private_pool_alloc_cell(MPIDI_OFI_global.pack_buf_pool,
                                                       (void **) &send_buf);
        if (send_buf == NULL) {
            /* no buffer available, suppress error and exit */
            mpi_errno = MPI_SUCCESS;
            goto fn_exit;
        }
        MPIR_ERR_CHECK(mpi_errno);
    }

    if (MPIDIG_REQUEST(dreq->sreq, req->plreq).seg_next == 1) {
        child_sreq = dreq->sreq;
    } else {
        MPIR_cc_incr(dreq->sreq->cc_ptr, &c);
        /* This child request is invisible to user, refcount should set to 1 */
        child_sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 1);
        MPIR_ERR_CHKANDSTMT((child_sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                            "**nomemreq");

        MPIDIG_REQUEST(child_sreq, req->plreq).parent_req = dreq->sreq;

        MPIDIG_REQUEST(child_sreq, req->plreq).src_buf = dreq->buf;
        MPIDIG_REQUEST(child_sreq, req->plreq).count = dreq->count;

        MPIR_Datatype_add_ref_if_not_builtin(dreq->datatype);
        MPIDIG_REQUEST(child_sreq, req->plreq).datatype = dreq->datatype;

        MPIDIG_REQUEST(child_sreq, req->plreq).tag = MPIDIG_REQUEST(dreq->sreq, req->plreq).tag;
        MPIDIG_REQUEST(child_sreq, req->plreq).rank = MPIDIG_REQUEST(dreq->sreq, req->plreq).rank;
        MPIDIG_REQUEST(child_sreq, req->plreq).context_id =
            MPIDIG_REQUEST(dreq->sreq, req->plreq).context_id;
        MPIDIG_REQUEST(child_sreq, rank) = MPIDIG_REQUEST(dreq->sreq, rank);

        mpi_errno = MPIDI_OFI_am_init_request(MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, am_hdr),
                                              MPIDI_OFI_AMREQUEST_HDR(dreq->sreq, am_hdr_sz),
                                              child_sreq);
        MPIR_ERR_CHECK(mpi_errno);

    }

    send_size = MPL_MIN(MPIDIG_REQUEST(dreq->sreq, data_sz_left), send_size);

    if (need_packing) {
        mpi_errno = MPIR_Typerep_pack(dreq->buf, dreq->count, dreq->datatype,
                                      MPIDIG_REQUEST(dreq->sreq, offset), send_buf, send_size,
                                      &packed_size);
        MPIR_ERR_CHECK(mpi_errno);
        send_size = packed_size;

        MPIDI_OFI_AMREQUEST_HDR(child_sreq, pack_buffer) = send_buf;
    } else {
        send_buf = (char *) (dreq->buf) + dt_true_lb + MPIDIG_REQUEST(dreq->sreq, offset);
        MPIDI_OFI_AMREQUEST_HDR(child_sreq, pack_buffer) = NULL;
    }

    mpi_errno = MPIDI_OFI_am_isend_short(dreq->rank, dreq->comm, dreq->handler_id, send_buf,
                                         send_size, child_sreq);
    MPIR_ERR_CHECK(mpi_errno);

    MPIDIG_REQUEST(dreq->sreq, data_sz_left) -= send_size;
    MPIDIG_REQUEST(dreq->sreq, offset) += send_size;
    MPIDIG_REQUEST(dreq->sreq, req->plreq).seg_next += 1;

    if (MPIDIG_REQUEST(dreq->sreq, data_sz_left) == 0) {
        /* only dequeue when all the data is sent */
        MPIDI_OFI_deferred_am_isend_dequeue(dreq);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_PIPELINE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFI_deferred_am_isend_issue(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_ISSUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_ISSUE);

    switch (dreq->op) {
        case MPIDI_OFI_DEFERRED_AM_ISEND_OP__EAGER:
            mpi_errno = issue_deferred_am_isend_eager(dreq);
            break;
        case MPIDI_OFI_DEFERRED_AM_ISEND_OP__PIPELINE:
            mpi_errno = issue_deferred_am_isend_pipeline(dreq);
            break;
        default:
            MPIR_Assert(0);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_ISSUE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
