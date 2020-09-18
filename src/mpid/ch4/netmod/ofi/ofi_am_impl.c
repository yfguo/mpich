/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "ofi_impl.h"
#include "ofi_am_impl.h"
#include "ofi_noinline.h"

static int issue_deferred_am_isend_eager(MPIDI_OFI_deferred_am_isend_req_t * dreq);

void MPIDI_OFI_deferred_am_isend_dequeue(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    MPIDI_OFI_deferred_am_isend_req_t *curr_req = dreq;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);

    DL_DELETE(MPIDI_OFI_global.deferred_am_isend_q, curr_req);
    MPL_free(dreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_DEQUEUE);
}

static int issue_deferred_am_isend_eager(MPIDI_OFI_deferred_am_isend_req_t * dreq)
{
    int mpi_errno = MPI_SUCCESS;
    char *send_buf;
    MPI_Aint packed_size;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPIDI_OFI_am_send_request_t *send_req;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);

    if (dreq->need_packing) {
        /* FIXME: currently we always do packing, also for high density types. However,
         * we should not do packing unless needed. Also, for large low-density types
         * we should not allocate the entire buffer and do the packing at once. */
        /* TODO: (1) Skip packing for high-density datatypes; */
        mpi_errno = MPIDU_genq_private_pool_alloc_cell(MPIDI_OFI_global.pack_buf_pool,
                                                       (void **) &send_buf);
        if (send_buf == NULL) {
            /* no buffer available, suppress error and exit */
            mpi_errno = MPI_SUCCESS;
            goto fn_exit;
        }
        MPIR_ERR_CHECK(mpi_errno);
        mpi_errno = MPIR_Typerep_pack(dreq->buf, dreq->count, dreq->datatype, 0, send_buf,
                                      dreq->data_sz, &packed_size);
        MPIR_ERR_CHECK(mpi_errno);
        MPIR_Assert(dreq->data_sz == packed_size);

        MPIDI_OFI_am_create_send_request(dreq->sreq, dreq->am_hdr, dreq->am_hdr_sz, &send_req);
        send_req->req_hdr.pack_buffer = send_buf;
    } else {
        /* TODO: we will need to handle the unpacked dense noncontig data here */
        MPIDI_Datatype_get_ptr_lb(dreq->count, dreq->datatype, dt_ptr, dt_true_lb);
        send_buf = (char *) (dreq->buf) + dt_true_lb;
        MPIDI_OFI_am_create_send_request(dreq->sreq, dreq->am_hdr, dreq->am_hdr_sz, &send_req);
        send_req->req_hdr.pack_buffer = NULL;
    }

    MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE,
                    (MPL_DBG_FDEST,
                     "issue eager seg for req handle=0x%x send_size %ld", dreq->sreq->handle,
                     dreq->data_sz));

    mpi_errno = MPIDI_OFI_am_isend_short(dreq->rank, dreq->comm, dreq->handler_id, send_buf,
                                         dreq->data_sz, dreq->sreq, send_req);
    MPIDI_OFI_deferred_am_isend_dequeue(dreq);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_ISSUE_DEFERRED_AM_ISEND_EAGER);
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
        case MPIDI_OFI_DEFERRED_AM_OP__ISEND_EAGER:
            mpi_errno = issue_deferred_am_isend_eager(dreq);
            break;
        case MPIDI_OFI_DEFERRED_AM_OP__ISEND_PIPELINE:
        default:
            MPIR_Assert(0);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_DEFERRED_AM_ISEND_ISSUE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
