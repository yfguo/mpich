/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef STUBNM_AM_H_INCLUDED
#define STUBNM_AM_H_INCLUDED

#include "stubnm_impl.h"

static inline int MPIDI_NM_am_isend(int rank,
                                    MPIR_Comm * comm,
                                    int handler_id,
                                    const void *am_hdr,
                                    size_t am_hdr_sz,
                                    const void *data,
                                    MPI_Count count, MPI_Datatype datatype, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_isendv(int rank,
                                     MPIR_Comm * comm,
                                     int handler_id,
                                     struct iovec *am_hdr,
                                     size_t iov_len,
                                     const void *data,
                                     MPI_Count count, MPI_Datatype datatype, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_isend_reply(MPIR_Context_id_t context_id, int src_rank,
                                          int handler_id,
                                          const void *am_hdr,
                                          size_t am_hdr_sz,
                                          const void *data,
                                          MPI_Count count,
                                          MPI_Datatype datatype, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_NM_am_hdr_max_sz(void)
{
    MPIR_Assert(0);
    return 0;
}

static inline size_t MPIDI_NM_am_eager_limit(void)
{
    MPIR_Assert(0);
    return 0;
}

static inline size_t MPIDI_NM_am_eager_buf_limit(void)
{
    MPIR_Assert(0);
    return 0;
}

static inline int MPIDI_NM_am_send_hdr(int rank,
                                       MPIR_Comm * comm,
                                       int handler_id, const void *am_hdr, size_t am_hdr_sz)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_send_hdr_reply(MPIR_Context_id_t context_id, int src_rank,
                                             int handler_id, const void *am_hdr, size_t am_hdr_sz)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_isend_pipeline_rts(int rank, MPIR_Comm * comm, int handler_id,
                                                 const void *am_hdr, size_t am_hdr_sz,
                                                 const void *data, MPI_Count count,
                                                 MPI_Datatype datatype, MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_isend_pipeline_seg(MPIR_Context_id_t context_id, int src_rank,
                                                 int handler_id, const void *am_hdr,
                                                 size_t am_hdr_sz, const void *data,
                                                 MPI_Count count, MPI_Datatype datatype,
                                                 MPIR_Request * sreq)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_am_choose_protocol(const void *buf, MPI_Count count,
                                              MPI_Datatype datatype, size_t am_ext_sz,
                                              int handler_id)
{
    int protocol = 0;

    MPIR_Assert(0);

    return protocol;
}

#endif /* STUBNM_AM_H_INCLUDED */
