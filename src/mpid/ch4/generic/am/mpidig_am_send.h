/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDIG_AM_SEND_H_INCLUDED
#define MPIDIG_AM_SEND_H_INCLUDED

#include "ch4_impl.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_CH4_EAGER_MAX_MSG_SIZE
      category    : CH4
      type        : int
      default     : -1
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_LOCAL
      description : >-
        If set to positive number, this cvar controls the message size at which CH4 switches from eager to rendezvous mode.
        If the number is negative, underlying netmod or shmmod automatically uses an optimal number depending on
        the underlying fabric or shared memory architecture.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

#define MPIDIG_AM_SEND_HDR_SIZE  sizeof(MPIDIG_hdr_t)

static inline int MPIDIG_eager_limit(int is_local, int payload_am_hdr_sz)
{
    if (MPIR_CVAR_CH4_EAGER_MAX_MSG_SIZE > 0) {
        return MPIR_CVAR_CH4_EAGER_MAX_MSG_SIZE;
    }

    int thresh;
#ifdef MPIDI_CH4_DIRECT_NETMOD
    thresh = MPIDI_NM_am_eager_limit();
#else
    if (is_local) {
        thresh = MPIDI_SHM_am_eager_limit();
    } else {
        thresh = MPIDI_NM_am_eager_limit();
    }
#endif
    thresh -= MPIDIG_AM_SEND_HDR_SIZE;
    thresh -= payload_am_hdr_sz;
    MPIR_Assert(thresh > 0);

    return thresh;
}

static inline int MPIDIG_do_eager_send_new(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                           MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                           int context_offset, MPIDI_av_entry_t * addr,
                                           MPIR_Request ** request, MPIR_Errflag_t errflag,
                                           int payload_handler_id, const void *payload_am_hdr,
                                           size_t payload_am_hdr_sz);
static inline int MPIDIG_do_pipeline_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                          MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                          int context_offset, MPIDI_av_entry_t * addr,
                                          MPIR_Request ** request, MPIR_Errflag_t errflag,
                                          int payload_handler_id, const void *payload_am_hdr,
                                          size_t payload_am_hdr_sz);
static inline int MPIDIG_do_rdma_read_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                           MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                           int context_offset, MPIDI_av_entry_t * addr,
                                           MPIR_Request ** request, MPIR_Errflag_t errflag,
                                           int payload_handler_id, const void *payload_am_hdr,
                                           size_t payload_am_hdr_sz);
static inline int MPIDIG_isend_impl_new(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                        int rank, int tag, MPIR_Comm * comm, int context_offset,
                                        MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                        MPIR_Errflag_t errflag, int payload_handler_id,
                                        const void *payload_am_hdr, size_t payload_am_hdr_sz,
                                        int protocol);

static inline int MPIDIG_do_ssend(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                  int rank, int tag, MPIR_Comm * comm, int context_offset,
                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                  MPIR_Errflag_t errflag, int protocol);

static inline int MPIDIG_do_ssend(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                  int rank, int tag, MPIR_Comm * comm, int context_offset,
                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                  MPIR_Errflag_t errflag, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIR_Request *sreq = *request;

    if (sreq == NULL) {
        sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 2);
        MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    } else {
        MPIDIG_request_init(sreq, MPIR_REQUEST_KIND__SEND);
    }

    *request = sreq;

    MPIDIG_ssend_req_msg_t am_hdr;
    am_hdr.sreq_ptr = sreq;

#ifdef HAVE_DEBUGGER_SUPPORT
    MPIDIG_REQUEST(sreq, datatype) = datatype;
    MPIDIG_REQUEST(sreq, buffer) = (char *) buf;
    MPIDIG_REQUEST(sreq, count) = count;
#endif

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, errflag, MPIDIG_SSEND_REQ, &am_hdr, sizeof(am_hdr),
                                      protocol);

    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_eager_send_new(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                           MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                           int context_offset, MPIDI_av_entry_t * addr,
                                           MPIR_Request ** request, MPIR_Errflag_t errflag,
                                           int payload_handler_id, const void *payload_am_hdr,
                                           size_t payload_am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *payload_req = NULL;
    MPIR_Request *sreq = NULL;
    MPIDIG_hdr_t am_hdr;
    MPIDIG_hdr_t *carrier_am_hdr = &am_hdr;
    void *send_am_hdr = &am_hdr;
    size_t send_am_hdr_sz = sizeof(MPIDIG_hdr_t);

    if (payload_handler_id != -1) {
        /* this is not a non-regular AM send */
        payload_req = *request;
        sreq = *request;
        int c;
        MPIR_cc_incr(sreq->cc_ptr, &c);
        /* allocate a am_hdr buffer large enough for the long req message and the payload am_hdr */
        send_am_hdr_sz = sizeof(MPIDIG_hdr_t) + payload_am_hdr_sz;
        send_am_hdr = MPL_malloc(send_am_hdr_sz, MPL_MEM_OTHER);
        MPIR_ERR_CHKANDSTMT((send_am_hdr) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                            "**nomemreq");
        /* set am_hdr for carrier request */
        carrier_am_hdr = (MPIDIG_hdr_t *) send_am_hdr;
        carrier_am_hdr->payload_handler_id = payload_handler_id;
        /* copy payload am_hdr */
        memcpy((char *) send_am_hdr + sizeof(MPIDIG_hdr_t), payload_am_hdr, payload_am_hdr_sz);
    } else {
        /* This is a regular MPIDIG_SEND message, prepare message hdr */
        sreq = *request;
        if (sreq == NULL) {
            sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 2);
            MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                                "**nomemreq");
            *request = sreq;
        } else {
            MPIDIG_request_init(sreq, MPIR_REQUEST_KIND__SEND);
        }

        carrier_am_hdr->payload_handler_id = -1;
    }

    carrier_am_hdr->src_rank = comm->rank;
    carrier_am_hdr->tag = tag;
    carrier_am_hdr->context_id = comm->context_id + context_offset;
    carrier_am_hdr->error_bits = errflag;
    MPIDIG_REQUEST(sreq, data_sz_left) = data_sz;
    MPIDIG_REQUEST(sreq, offset) = 0;

#ifdef HAVE_DEBUGGER_SUPPORT
    MPIDIG_REQUEST(sreq, datatype) = datatype;
    MPIDIG_REQUEST(sreq, buffer) = (char *) buf;
    MPIDIG_REQUEST(sreq, count) = count;
#endif

#ifndef MPIDI_CH4_DIRECT_NETMOD
    if (payload_req) {
        MPIDI_REQUEST(payload_req, is_local) = MPIDI_av_is_local(addr);
    }
    MPIDI_REQUEST(sreq, is_local) = MPIDI_av_is_local(addr);

    if (MPIDI_av_is_local(addr)) {
        mpi_errno = MPIDI_SHM_am_isend(rank, comm, MPIDIG_SEND, send_am_hdr, send_am_hdr_sz, buf,
                                       count, datatype, sreq);
    } else
#endif
    {
        mpi_errno = MPIDI_NM_am_isend(rank, comm, MPIDIG_SEND, send_am_hdr, send_am_hdr_sz, buf,
                                      count, datatype, sreq);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (payload_handler_id != -1) {
        MPL_free(send_am_hdr);
    }
    return mpi_errno;

  fn_fail:
    goto fn_exit;

}

static inline int MPIDIG_do_pipeline_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                          MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                          int context_offset, MPIDI_av_entry_t * addr,
                                          MPIR_Request ** request, MPIR_Errflag_t errflag,
                                          int payload_handler_id, const void *payload_am_hdr,
                                          size_t payload_am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *payload_req = NULL;
    MPIR_Request *sreq = NULL;
    MPIDIG_send_pipeline_rts_msg_t am_hdr;
    MPIDIG_send_pipeline_rts_msg_t *carrier_am_hdr = &am_hdr;
    void *send_am_hdr = &am_hdr;
    size_t send_am_hdr_sz = sizeof(MPIDIG_send_pipeline_rts_msg_t);

    if (payload_handler_id != -1) {
        /* this is not a non-regular AM send, create carrier request and message header */
        payload_req = *request;
        int c;
        /* create a carrier request */
        sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 1);
        MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
        /* allocate a am_hdr buffer large enough for the long req message and the payload am_hdr */
        send_am_hdr_sz = sizeof(MPIDIG_send_pipeline_rts_msg_t) + payload_am_hdr_sz;
        send_am_hdr = MPL_malloc(send_am_hdr_sz, MPL_MEM_OTHER);
        MPIR_ERR_CHKANDSTMT((send_am_hdr) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                            "**nomemreq");
        /* set am_hdr for carrier request */
        carrier_am_hdr = (MPIDIG_send_pipeline_rts_msg_t *) send_am_hdr;
        carrier_am_hdr->hdr.payload_handler_id = payload_handler_id;
        /* copy payload am_hdr */
        memcpy((char *) send_am_hdr + sizeof(MPIDIG_send_pipeline_rts_msg_t), payload_am_hdr,
               payload_am_hdr_sz);
    } else {
        sreq = *request;
        if (sreq == NULL) {
            sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 2);
            MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                                "**nomemreq");
            *request = sreq;
        } else {
            MPIDIG_request_init(sreq, MPIR_REQUEST_KIND__SEND);
        }

        carrier_am_hdr->hdr.payload_handler_id = -1;
    }

    carrier_am_hdr->hdr.src_rank = comm->rank;
    carrier_am_hdr->hdr.tag = tag;
    carrier_am_hdr->hdr.context_id = comm->context_id + context_offset;
    carrier_am_hdr->hdr.error_bits = errflag;
    carrier_am_hdr->data_sz = data_sz;
    carrier_am_hdr->sreq_ptr = sreq;

    MPIDIG_REQUEST(sreq, req->plreq).src_buf = buf;
    MPIDIG_REQUEST(sreq, req->plreq).count = count;
    MPIR_Datatype_add_ref_if_not_builtin(datatype);
    MPIDIG_REQUEST(sreq, req->plreq).datatype = datatype;
    MPIDIG_REQUEST(sreq, req->plreq).tag = carrier_am_hdr->hdr.tag;
    MPIDIG_REQUEST(sreq, req->plreq).rank = carrier_am_hdr->hdr.src_rank;
    MPIDIG_REQUEST(sreq, req->plreq).context_id = carrier_am_hdr->hdr.context_id;
    MPIDIG_REQUEST(sreq, req->plreq).seg_next = 0;
    MPIDIG_REQUEST(sreq, data_sz_left) = data_sz;
    MPIDIG_REQUEST(sreq, offset) = 0;
    MPIDIG_REQUEST(sreq, rank) = rank;

#ifdef HAVE_DEBUGGER_SUPPORT
    MPIDIG_REQUEST(sreq, datatype) = datatype;
    MPIDIG_REQUEST(sreq, buffer) = (char *) buf;
    MPIDIG_REQUEST(sreq, count) = count;
#endif

#ifndef MPIDI_CH4_DIRECT_NETMOD
    if (payload_req) {
        MPIDI_REQUEST(payload_req, is_local) = MPIDI_av_is_local(addr);
    }
    MPIDI_REQUEST(sreq, is_local) = MPIDI_av_is_local(addr);

    if (MPIDI_av_is_local(addr)) {
        mpi_errno = MPIDI_SHM_am_isend_pipeline_rts(rank, comm, MPIDIG_SEND_PIPELINE_RTS,
                                                    send_am_hdr, send_am_hdr_sz, buf, count,
                                                    datatype, sreq);
    } else
#endif
    {
        mpi_errno = MPIDI_NM_am_isend_pipeline_rts(rank, comm, MPIDIG_SEND_PIPELINE_RTS,
                                                   send_am_hdr, send_am_hdr_sz, buf, count,
                                                   datatype, sreq);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (payload_handler_id != -1) {
        MPL_free(send_am_hdr);
    }
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_rdma_read_send(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                           MPI_Aint data_sz, int rank, int tag, MPIR_Comm * comm,
                                           int context_offset, MPIDI_av_entry_t * addr,
                                           MPIR_Request ** request, MPIR_Errflag_t errflag,
                                           int payload_handler_id, const void *payload_am_hdr,
                                           size_t payload_am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *payload_req = NULL;
    MPIR_Request *sreq = NULL;
    MPIDIG_send_rdma_read_req_msg_t am_hdr;
    MPIDIG_send_rdma_read_req_msg_t *carrier_am_hdr = &am_hdr;
    void *send_am_hdr = &am_hdr;
    size_t send_am_hdr_sz = sizeof(MPIDIG_send_rdma_read_req_msg_t);

    if (payload_handler_id != -1) {
        /* this is not a non-regular AM send, create carrier request and message header */
        payload_req = *request;
        int c;
        /* create a carrier request */
        sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 1);
        MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
        /* allocate a am_hdr buffer large enough for the long req message and the payload am_hdr */
        send_am_hdr_sz = sizeof(MPIDIG_send_rdma_read_req_msg_t) + payload_am_hdr_sz;
        send_am_hdr = MPL_malloc(send_am_hdr_sz, MPL_MEM_OTHER);
        MPIR_ERR_CHKANDSTMT((send_am_hdr) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                            "**nomemreq");
        /* set am_hdr for carrier request */
        carrier_am_hdr = (MPIDIG_send_rdma_read_req_msg_t *) send_am_hdr;
        carrier_am_hdr->hdr.payload_handler_id = payload_handler_id;
        /* copy payload am_hdr */
        memcpy((char *) send_am_hdr + sizeof(MPIDIG_send_rdma_read_req_msg_t), payload_am_hdr,
               payload_am_hdr_sz);
    } else {
        sreq = *request;
        if (sreq == NULL) {
            sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 2);
            MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail,
                                "**nomemreq");
            *request = sreq;
        } else {
            MPIDIG_request_init(sreq, MPIR_REQUEST_KIND__SEND);
        }

        carrier_am_hdr->hdr.payload_handler_id = -1;
    }

    carrier_am_hdr->hdr.src_rank = comm->rank;
    carrier_am_hdr->hdr.tag = tag;
    carrier_am_hdr->hdr.context_id = comm->context_id + context_offset;
    carrier_am_hdr->hdr.error_bits = errflag;
    carrier_am_hdr->data_sz = data_sz;
    carrier_am_hdr->sreq_ptr = sreq;

    MPIDIG_REQUEST(sreq, req->rdr_req).src_buf = buf;
    MPIDIG_REQUEST(sreq, req->rdr_req).count = count;
    MPIR_Datatype_add_ref_if_not_builtin(datatype);
    MPIDIG_REQUEST(sreq, req->rdr_req).datatype = datatype;
    MPIDIG_REQUEST(sreq, req->rdr_req).tag = carrier_am_hdr->hdr.tag;
    MPIDIG_REQUEST(sreq, req->rdr_req).rank = carrier_am_hdr->hdr.src_rank;
    MPIDIG_REQUEST(sreq, req->rdr_req).context_id = carrier_am_hdr->hdr.context_id;
    MPIDIG_REQUEST(sreq, data_sz_left) = data_sz;
    MPIDIG_REQUEST(sreq, offset) = 0;
    MPIDIG_REQUEST(sreq, rank) = rank;

#ifdef HAVE_DEBUGGER_SUPPORT
    MPIDIG_REQUEST(sreq, datatype) = datatype;
    MPIDIG_REQUEST(sreq, buffer) = (char *) buf;
    MPIDIG_REQUEST(sreq, count) = count;
#endif

#ifndef MPIDI_CH4_DIRECT_NETMOD
    if (payload_req) {
        MPIDI_REQUEST(payload_req, is_local) = MPIDI_av_is_local(addr);
    }
    MPIDI_REQUEST(sreq, is_local) = MPIDI_av_is_local(addr);

#endif
    mpi_errno = MPIDI_NM_am_isend_rdma_read_req(rank, comm, MPIDIG_SEND_RDMA_READ_REQ,
                                                send_am_hdr, send_am_hdr_sz, buf, count,
                                                datatype, sreq);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (payload_handler_id != -1) {
        MPL_free(send_am_hdr);
    }
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

/* this is the new entry point for sending message. The _new prefix will be removed later */
static inline int MPIDIG_isend_impl_new(const void *buf, MPI_Aint count, MPI_Datatype datatype,
                                        int rank, int tag, MPIR_Comm * comm, int context_offset,
                                        MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                        MPIR_Errflag_t errflag, int payload_handler_id,
                                        const void *payload_am_hdr, size_t payload_am_hdr_sz,
                                        int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint data_sz;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_ISEND_IMPL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_ISEND_IMPL);

    MPIDI_Datatype_check_size(datatype, count, data_sz);

    switch (protocol) {
        case MPIDIG_AM_PROTOCOL__EAGER:
            mpi_errno = MPIDIG_do_eager_send_new(buf, count, datatype, data_sz, rank, tag, comm,
                                                 context_offset, addr, request, errflag,
                                                 payload_handler_id, payload_am_hdr,
                                                 payload_am_hdr_sz);
            break;
        case MPIDIG_AM_PROTOCOL__PIPELINE:
            mpi_errno = MPIDIG_do_pipeline_send(buf, count, datatype, data_sz, rank, tag, comm,
                                                context_offset, addr, request, errflag,
                                                payload_handler_id, payload_am_hdr,
                                                payload_am_hdr_sz);
            break;
        case MPIDIG_AM_PROTOCOL__RDMA_READ:
            mpi_errno = MPIDIG_do_rdma_read_send(buf, count, datatype, data_sz, rank, tag, comm,
                                                 context_offset, addr, request, errflag,
                                                 payload_handler_id, payload_am_hdr,
                                                 payload_am_hdr_sz);
            break;
        default:
            MPIR_Assert(0);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_ISEND_IMPL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_send_new(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank,
                                                 int tag, MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                 int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_SEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, MPIR_ERR_NONE, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_SEND);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_send_coll_new(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank,
                                                  int tag, MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                  MPIR_Errflag_t * errflag, int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_SEND_COLL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_SEND_COLL);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, *errflag, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_SEND_COLL);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_isend_new(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank,
                                                  int tag, MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                  int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_ISEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, MPIR_ERR_NONE, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_ISEND);
    return mpi_errno;
}


MPL_STATIC_INLINE_PREFIX int MPIDIG_isend_coll_new(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank,
                                                   int tag, MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                   MPIR_Errflag_t * errflag, int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_ISEND_COLL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_ISEND_COLL);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, *errflag, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_ISEND_COLL);
    return mpi_errno;
}


MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_rsend_new(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank,
                                                  int tag, MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                  int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_RSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_RSEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, MPIR_ERR_NONE, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_RSEND);
    return mpi_errno;
}


MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_irsend_new(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank,
                                                   int tag, MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                   int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_IRSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_IRSEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_isend_impl_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request, MPIR_ERR_NONE, -1, NULL, 0, protocol);

    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_IRSEND);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_ssend_new(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank,
                                                  int tag, MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                  int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_SSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_SSEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_do_ssend(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, MPIR_ERR_NONE, protocol);


    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_SSEND);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_issend_new(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank,
                                                   int tag, MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                   int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_ISSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_ISSEND);
    MPID_THREAD_CS_ENTER(VCI, MPIDI_VCI(0).lock);

    mpi_errno = MPIDIG_do_ssend(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, MPIR_ERR_NONE, protocol);


    MPID_THREAD_CS_EXIT(VCI, MPIDI_VCI(0).lock);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_ISSEND);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_cancel_send(MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_CANCEL_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_CANCEL_SEND);

    /* cannot cancel send */

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_CANCEL_SEND);
    return mpi_errno;
}

#endif /* MPIDIG_AM_SEND_H_INCLUDED */
