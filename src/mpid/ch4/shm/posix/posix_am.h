/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_AM_H_INCLUDED
#define POSIX_AM_H_INCLUDED

#include "posix_impl.h"
#include "posix_am_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_isend(int rank,
                                                  MPIR_Comm * comm,
                                                  MPIDI_POSIX_am_header_kind_t kind,
                                                  int handler_id,
                                                  const void *am_hdr,
                                                  size_t am_hdr_sz,
                                                  const void *data,
                                                  MPI_Count count,
                                                  MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_am_header_t msg_hdr;
    const int grank = MPIDIU_rank_to_lpid(rank, comm);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_ISEND);

    msg_hdr.kind = kind;
    msg_hdr.handler_id = handler_id;
    msg_hdr.am_hdr_sz = am_hdr_sz;

    mpi_errno = MPIDI_POSIX_eager_send(data, count, datatype, grank, &msg_hdr, am_hdr, 0, sreq);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_isendv(int rank,
                                                   MPIR_Comm * comm,
                                                   MPIDI_POSIX_am_header_kind_t kind,
                                                   int handler_id,
                                                   struct iovec *am_hdr,
                                                   size_t iov_len,
                                                   const void *data,
                                                   MPI_Count count,
                                                   MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    int is_allocated;
    size_t am_hdr_sz = 0;
    int i;
    uint8_t *am_hdr_buf = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_ISENDV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_ISENDV);

    for (i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    if (am_hdr_sz > MPIDI_POSIX_AM_HDR_POOL_CELL_SIZE) {
        am_hdr_buf = (uint8_t *) MPL_malloc(am_hdr_sz, MPL_MEM_SHM);
        is_allocated = 1;
    } else {
        MPIDU_genq_private_pool_alloc_cell(MPIDI_POSIX_global.am_hdr_buf_pool,
                                           (void **) &am_hdr_buf);
        MPIR_Assert(am_hdr_buf);
        is_allocated = 0;
    }

    MPIR_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for (i = 0; i < iov_len; i++) {
        MPIR_Typerep_copy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_POSIX_am_isend(rank, comm, kind, handler_id, am_hdr_buf, am_hdr_sz,
                                     data, count, datatype, sreq);

    if (is_allocated)
        MPL_free(am_hdr_buf);
    else
        MPIDU_genq_private_pool_free_cell(MPIDI_POSIX_global.am_hdr_buf_pool, am_hdr_buf);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_ISENDV);

    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_isend_reply(MPIR_Context_id_t context_id, int src_rank,
                                                        MPIDI_POSIX_am_header_kind_t kind,
                                                        int handler_id,
                                                        const void *am_hdr,
                                                        size_t am_hdr_sz,
                                                        const void *data,
                                                        MPI_Count count,
                                                        MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_ISEND_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_ISEND_REPLY);

    mpi_errno = MPIDI_POSIX_am_isend(src_rank, MPIDIG_context_id_to_comm(context_id), kind,
                                     handler_id, am_hdr, am_hdr_sz, data, count, datatype, sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_ISEND_REPLY);

    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_isend_pipeline(MPIR_Context_id_t context_id,
                                                           int src_rank,
                                                           MPIDI_POSIX_am_header_kind_t kind,
                                                           int handler_id,
                                                           const void *am_hdr,
                                                           size_t am_hdr_sz,
                                                           const void *data,
                                                           MPI_Count count,
                                                           MPI_Datatype datatype,
                                                           MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_POSIX_am_header_t msg_hdr;
    const int grank = MPIDIU_rank_to_lpid(src_rank, MPIDIG_context_id_to_comm(context_id));

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_ISEND_PIPELINE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_ISEND_PIPELINE);

    msg_hdr.kind = kind;
    msg_hdr.handler_id = handler_id;
    msg_hdr.am_hdr_sz = am_hdr_sz;

    MPIDIG_REQUEST(sreq, req->lreq).parent_req = sreq;

    mpi_errno = MPIDI_POSIX_pipeline_send(data, count, datatype, grank, &msg_hdr, am_hdr, sreq);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_ISEND_PIPELINE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_get_avail_long_protocol(const void *data, int count,
                                                                    MPI_Datatype datatype,
                                                                    int *avail_protocol_bits)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_GET_AVAIL_LONG_PROTOCOL);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_GET_AVAIL_LONG_PROTOCOL);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_GET_AVAIL_LONG_PROTOCOL);
    return mpi_errno;
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_am_hdr_max_sz(void)
{
    /* Maximum size that fits in short send */

    size_t max_shortsend =
        MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_CELL_SIZE - sizeof(MPIDI_POSIX_eager_iqueue_cell_t);

    /* Maximum payload size representable by MPIDI_POSIX_am_header_t::am_hdr_sz field */

    size_t max_representable = (1 << MPIDI_POSIX_AM_HDR_SZ_BITS) - 1;

    return MPL_MIN(max_shortsend, max_representable);
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_am_eager_limit(void)
{
    return MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_CELL_SIZE - sizeof(MPIDI_POSIX_eager_iqueue_cell_t);
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_am_eager_buf_limit(void)
{
    return MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_CELL_SIZE;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_send_hdr(int rank,
                                                     MPIR_Comm * comm,
                                                     MPIDI_POSIX_am_header_kind_t kind,
                                                     int handler_id,
                                                     const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;
    int result = MPIDI_POSIX_OK;
    MPIDI_POSIX_am_header_t msg_hdr;
    MPIDI_POSIX_am_header_t *msg_hdr_p = &msg_hdr;
    struct iovec iov_left[1];
    struct iovec *iov_left_ptr = iov_left;
    size_t iov_num_left = 1;
    const int grank = MPIDIU_rank_to_lpid(rank, comm);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR);

    msg_hdr.kind = kind;
    msg_hdr.handler_id = handler_id;
    msg_hdr.am_hdr_sz = am_hdr_sz;
    msg_hdr.data_sz = 0;

    mpi_errno = MPIDI_POSIX_eager_send(NULL, 0, MPI_DATATYPE_NULL, grank, &msg_hdr, am_hdr, 1,
                                       NULL);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_am_send_hdr_reply(MPIR_Context_id_t context_id,
                                                           int src_rank,
                                                           MPIDI_POSIX_am_header_kind_t kind,
                                                           int handler_id, const void *am_hdr,
                                                           size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR_REPLY);

    mpi_errno = MPIDI_POSIX_am_send_hdr(src_rank,
                                        MPIDIG_context_id_to_comm(context_id),
                                        kind, handler_id, am_hdr, am_hdr_sz);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_POSIX_AM_SEND_HDR_REPLY);

    return mpi_errno;
}

#endif /* POSIX_AM_H_INCLUDED */
