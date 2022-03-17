/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef UCX_SEND_STREAM_H_INCLUDED
#define UCX_SEND_STREAM_H_INCLUDED
#include <ucp/api/ucp.h>
#include "mpir_func.h"
#include "ucx_impl.h"
#include "ucx_types.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_UCX_send_stream_prepare(const void *buf,
                                                           MPI_Aint count,
                                                           MPI_Datatype datatype,
                                                           int rank,
                                                           int tag,
                                                           MPIR_Comm * comm,
                                                           int context_offset,
                                                           MPIDI_av_entry_t * addr,
                                                           MPIR_Request ** request,
                                                           int vni_src, int vni_dst,
                                                           int have_request, int is_sync,
                                                           MPIDI_UCX_deferred_send_t ** send_op)
{
    int dt_contig;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *req = *request;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;

    MPIR_FUNC_ENTER;

    ucp_request_param_t param = {
        .op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK,
        .cb.send = MPIDI_UCX_send_cmpl_cb,
    };

    ep = MPIDI_UCX_AV_TO_EP(addr, vni_src, vni_dst);
    ucx_tag = MPIDI_UCX_init_tag(comm->context_id + context_offset, comm->rank, tag);
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    const void *send_buf;
    size_t send_count;
    if (dt_contig) {
        send_buf = MPIR_get_contig_ptr(buf, dt_true_lb);
        send_count = data_sz;
    } else {
        send_buf = buf;
        send_count = count;
        param.op_attr_mask |= UCP_OP_ATTR_FIELD_DATATYPE;
        param.datatype = dt_ptr->dev.netmod.ucx.ucp_datatype;
        MPIR_Datatype_ptr_add_ref(dt_ptr);
    }

    ucp_request = ucp_request_alloc(MPIDI_UCX_global.ctx[vni_src].worker);
    param.request = ucp_request;
    if (req == NULL) {
        req = MPIR_Request_create_from_pool(MPIR_REQUEST_KIND__SEND, vni_src, 2);
    } else {
        MPIR_Request_add_ref(req);
    }
    ucp_request->req = req;
    MPIDI_UCX_REQ(req).ucp_request = ucp_request;
    *request = req;

    *send_op =
        (MPIDI_UCX_deferred_send_t *) MPL_malloc(sizeof(MPIDI_UCX_deferred_send_t), MPL_MEM_OTHER);
    (*send_op)->ep = ep;
    (*send_op)->send_buf = send_buf;
    (*send_op)->send_count = send_count;
    (*send_op)->ucx_tag = ucx_tag;
    (*send_op)->param = param;
    (*send_op)->vni_src = vni_src;

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_UCX_send_stream_trigger(void *data)
{
    MPIDI_UCX_ucp_request_t *ucp_request = NULL;
    MPIDI_UCX_deferred_send_t *send_op = (MPIDI_UCX_deferred_send_t *) data;

    MPIR_FUNC_ENTER;

    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(send_op->vni_src).lock);
    ucp_request =
        (MPIDI_UCX_ucp_request_t *) ucp_tag_send_nbx(send_op->ep, send_op->send_buf,
                                                     send_op->send_count, send_op->ucx_tag,
                                                     &(send_op->param));
    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(send_op->vni_src).lock);

    MPL_free(send_op);

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_isend_stream(const void *buf,
                                                       MPI_Aint count,
                                                       MPI_Datatype datatype,
                                                       int rank,
                                                       int tag,
                                                       MPIR_Comm * comm, int context_offset,
                                                       MPIDI_av_entry_t * addr,
                                                       MPL_gpu_stream_t stream,
                                                       MPIR_Request ** request)
{
    int mpi_errno;
    int vni_src = MPIDI_UCX_get_vni(SRC_VCI_FROM_SENDER, comm, comm->rank, rank, tag);
    int vni_dst = MPIDI_UCX_get_vni(DST_VCI_FROM_SENDER, comm, comm->rank, rank, tag);
    MPIDI_UCX_deferred_send_t *send_op;

    MPIR_FUNC_ENTER;

    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(vni_src).lock);
    mpi_errno = MPIDI_UCX_send_stream_prepare(buf, count, datatype, rank, tag, comm, context_offset,
                                              addr, request, vni_src, vni_dst, 1, 0, &send_op);
    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(vni_src).lock);
    /* queue on stream */
    MPL_gpu_stream_launch_host_fn(stream, MPIDI_UCX_send_stream_trigger, send_op);

    MPIR_FUNC_EXIT;
    return mpi_errno;
}

#endif /* UCX_SEND_STREAM_H_INCLUDED */
