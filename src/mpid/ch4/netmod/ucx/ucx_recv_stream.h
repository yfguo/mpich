/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef UCX_RECV_STREAM_H_INCLUDED
#define UCX_RECV_STREAM_H_INCLUDED

#include "ucx_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_UCX_recv_stream_prepare(void *buf,
                                                           MPI_Aint count,
                                                           MPI_Datatype datatype,
                                                           int rank,
                                                           int tag, MPIR_Comm * comm,
                                                           int context_offset,
                                                           MPIDI_av_entry_t * addr,
                                                           int vni_dst, MPIR_Request ** request,
                                                           MPIDI_UCX_deferred_recv_t ** recv_op)
{
    int mpi_errno = MPI_SUCCESS;
    size_t data_sz;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    uint64_t ucp_tag, tag_mask;
    MPIR_Request *req = *request;
    MPIDI_UCX_ucp_request_t *ucp_request;

    MPIR_FUNC_ENTER;

    if (req == NULL) {
        req = MPIR_Request_create_from_pool(MPIR_REQUEST_KIND__RECV, vni_dst, 2);
        MPIR_ERR_CHKANDSTMT(req == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    } else {
        MPIR_Request_add_ref(req);
    }

    ucp_request_param_t param = {
        .op_attr_mask =
            UCP_OP_ATTR_FIELD_CALLBACK | UCP_OP_ATTR_FIELD_USER_DATA | UCP_OP_ATTR_FLAG_NO_IMM_CMPL,
        .cb.recv = MPIDI_UCX_recv_cmpl_cb,
        .user_data = req,
    };

    tag_mask = MPIDI_UCX_tag_mask(tag, rank);
    ucp_tag = MPIDI_UCX_recv_tag(tag, rank, comm->recvcontext_id + context_offset);
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    void *recv_buf;
    size_t recv_count;
    if (dt_contig) {
        recv_buf = MPIR_get_contig_ptr(buf, dt_true_lb);
        recv_count = data_sz;
    } else {
        recv_buf = buf;
        recv_count = count;
        param.op_attr_mask |= UCP_OP_ATTR_FIELD_DATATYPE;
        param.datatype = dt_ptr->dev.netmod.ucx.ucp_datatype;
        MPIR_Datatype_ptr_add_ref(dt_ptr);
    }


    ucp_request = ucp_request_alloc(MPIDI_UCX_global.ctx[vni_dst].worker);
    param.request = ucp_request;

    MPIDI_UCX_REQ(req).ucp_request = ucp_request;
    *request = req;

    *recv_op =
        (MPIDI_UCX_deferred_recv_t *) MPL_malloc(sizeof(MPIDI_UCX_deferred_recv_t), MPL_MEM_OTHER);
    (*recv_op)->recv_buf = recv_buf;
    (*recv_op)->recv_count = recv_count;
    (*recv_op)->ucp_tag = ucp_tag;
    (*recv_op)->tag_mask = tag_mask;
    (*recv_op)->param = param;
    (*recv_op)->vni_dst = vni_dst;

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_UCX_recv_stream_trigger(void *data)
{
    MPIDI_UCX_ucp_request_t *ucp_request = NULL;
    MPIDI_UCX_deferred_recv_t *recv_op = (MPIDI_UCX_deferred_recv_t *) data;

    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(recv_op->vni_dst).lock);
    ucp_request =
        (MPIDI_UCX_ucp_request_t *) ucp_tag_recv_nbx(MPIDI_UCX_global.ctx[recv_op->vni_dst].worker,
                                                     recv_op->recv_buf, recv_op->recv_count,
                                                     recv_op->ucp_tag, recv_op->tag_mask,
                                                     &(recv_op->param));
    MPIDI_REQUEST_SET_LOCAL(*request, 0, partner);
    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(recv_op->vni_dst).lock);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_irecv_stream(void *buf,
                                                       MPI_Aint count,
                                                       MPI_Datatype datatype,
                                                       int rank,
                                                       int tag,
                                                       MPIR_Comm * comm, int context_offset,
                                                       MPIDI_av_entry_t * addr,
                                                       MPL_gpu_stream_t stream,
                                                       MPIR_Request ** request,
                                                       MPIR_Request * partner)
{
    int mpi_errno;

    int vni_dst = MPIDI_UCX_get_vni(DST_VCI_FROM_RECVER, comm, rank, comm->rank, tag);
    MPIDI_UCX_deferred_recv_t *recv_op;

    MPIR_FUNC_ENTER;

    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(vni_dst).lock);
    mpi_errno =
        MPIDI_UCX_recv_stream_prepare(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      vni_dst, request, &recv_op);
    recv_op->partner = partner;
    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(vni_dst).lock);
    MPL_gpu_stream_launch_host_fn(stream, MPIDI_UCX_recv_stream_trigger, recv_op);

    MPIR_FUNC_EXIT;
    return mpi_errno;
}

#endif /* UCX_RECV_STREAM_H_INCLUDED */
