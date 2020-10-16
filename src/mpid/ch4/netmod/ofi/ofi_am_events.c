/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "ofi_am_events.h"

int MPIDI_OFI_am_rdma_read_ack_handler(int handler_id, void *am_hdr, void *data,
                                       MPI_Aint in_data_sz, int is_local, int is_async,
                                       MPIR_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq;
    MPIDI_OFI_am_rdma_read_ack_msg_t *ack_msg;
    int src_handler_id;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_ACK_HANDLER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_ACK_HANDLER);

    ack_msg = (MPIDI_OFI_am_rdma_read_ack_msg_t *) am_hdr;
    sreq = ack_msg->sreq_ptr;

    if (!MPIDI_OFI_ENABLE_MR_PROV_KEY) {
        uint64_t mr_key = fi_mr_key(MPIDI_OFI_AMREQUEST_HDR(sreq, lmt_mr));
        MPIDI_OFI_mr_key_free(mr_key);
    }
    MPIDI_OFI_CALL(fi_close(&MPIDI_OFI_AMREQUEST_HDR(sreq, lmt_mr)->fid), mr_unreg);
    MPL_atomic_fetch_sub_int(&MPIDI_OFI_global.am_inflight_rma_send_mrs, 1);

    MPL_gpu_free_host(MPIDI_OFI_AMREQUEST_HDR(sreq, pack_buffer));

    /* retrieve the handler_id of the original send request for origin cb. Note the handler_id
     * parameter is MPIDI_OFI_AM_RDMA_READ_ACK and should never be called with origin_cbs */
    src_handler_id = MPIDI_OFI_AMREQUEST_HDR(sreq, msg_hdr).handler_id;
    mpi_errno = MPIDIG_global.origin_cbs[src_handler_id] (sreq);
    MPIR_ERR_CHECK(mpi_errno);
    MPL_free(MPIDI_OFI_AMREQUEST(sreq, saved_req));
    MPID_Request_complete(sreq);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_ACK_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_fail;
}

int MPIDI_OFI_am_rdma_read_reject_handler(int handler_id, void *am_hdr, void *data,
                                          MPI_Aint in_data_sz, int is_local, int is_async,
                                          MPIR_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq;
    MPIDI_OFI_am_rdma_read_reject_msg_t *reject_msg;
    MPIDI_OFI_am_resend_msg_t resend_msg;
    int src_handler_id;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_REJECT_HANDLER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_REJECT_HANDLER);

    reject_msg = (MPIDI_OFI_am_rdma_read_reject_msg_t *) am_hdr;
    sreq = reject_msg->sreq_ptr;

    /* if RDMA_READ is rejected by the receiver, we first cleanup the memory registration for send
     * buffer. Then resend the data with pipeline using the information saved inside the sreq. */
    if (!MPIDI_OFI_ENABLE_MR_PROV_KEY) {
        uint64_t mr_key = fi_mr_key(MPIDI_OFI_AMREQUEST_HDR(sreq, lmt_mr));
        MPIDI_OFI_mr_key_free(mr_key);
    }
    MPIDI_OFI_CALL(fi_close(&MPIDI_OFI_AMREQUEST_HDR(sreq, lmt_mr)->fid), mr_unreg);
    MPL_atomic_fetch_sub_int(&MPIDI_OFI_global.am_inflight_rma_send_mrs, 1);

    MPL_gpu_free_host(MPIDI_OFI_AMREQUEST_HDR(sreq, pack_buffer));

    resend_msg.sreq_ptr = sreq;
    resend_msg.rreq_ptr = reject_msg->rreq_ptr;

    mpi_errno = MPIDI_OFI_do_am_isend_pipeline(MPIDI_OFI_AMREQUEST(sreq, saved_req)->rank,
                                               MPIDI_OFI_AMREQUEST(sreq, saved_req)->comm,
                                               MPIDI_OFI_AM_RESEND, &resend_msg, sizeof(resend_msg),
                                               MPIDI_OFI_AMREQUEST(sreq, saved_req)->buf,
                                               MPIDI_OFI_AMREQUEST(sreq, saved_req)->count,
                                               MPIDI_OFI_AMREQUEST(sreq, saved_req)->datatype,
                                               sreq, MPIDI_OFI_AMREQUEST(sreq, saved_req)->data_sz,
                                               false);
    MPL_free(MPIDI_OFI_AMREQUEST(sreq, saved_req));

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_AM_RDMA_READ_REJECT_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFI_am_resend_handler(int handler_id, void *am_hdr, void *data, MPI_Aint in_data_sz,
                                int is_local, int is_async, MPIR_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *rreq;
    MPIDI_OFI_am_resend_msg_t *resend_hdr = (MPIDIG_send_data_msg_t *) am_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OFI_AM_RESEND_HANDLER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OFI_AM_RESEND_HANDLER);

    rreq = (MPIR_Request *) resend_hdr->rreq_ptr;
    MPIR_Assert(rreq);

    if (is_async) {
        *req = rreq;
    } else {
        MPIDIG_recv_copy(data, rreq);
        MPIDIG_REQUEST(rreq, req->target_cmpl_cb) (rreq);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OFI_AM_RESEND_HANDLER);
    return mpi_errno;
}
