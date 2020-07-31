/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef SHM_AM_FALLBACK_SEND_H_INCLUDED
#define SHM_AM_FALLBACK_SEND_H_INCLUDED

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_send(const void *buf,
                                                MPI_Aint count,
                                                MPI_Datatype datatype,
                                                int rank,
                                                int tag,
                                                MPIR_Comm * comm, int context_offset,
                                                MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_mpi_send_new(buf, count, datatype, rank, tag, comm, context_offset, addr, request,
                               protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_ssend(const void *buf,
                                                 MPI_Aint count,
                                                 MPI_Datatype datatype,
                                                 int rank,
                                                 int tag,
                                                 MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SSEND_REQ, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_mpi_ssend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_isend(const void *buf,
                                                 MPI_Aint count,
                                                 MPI_Datatype datatype,
                                                 int rank,
                                                 int tag,
                                                 MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_mpi_isend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_issend(const void *buf,
                                                  MPI_Aint count,
                                                  MPI_Datatype datatype,
                                                  int rank,
                                                  int tag,
                                                  MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SSEND_REQ, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_mpi_issend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                 request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_send_coll(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank, int tag,
                                                 MPIR_Comm * comm,
                                                 int context_offset, MPIDI_av_entry_t * addr,
                                                 MPIR_Request ** request, MPIR_Errflag_t * errflag)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_send_coll_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, errflag, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_isend_coll(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank, int tag,
                                                  MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                  MPIR_Errflag_t * errflag)
{
    size_t data_sz;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_check_size(count, datatype, data_sz);
    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_SHM_am_eager_limit())) {
        protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
    }
    return MPIDIG_isend_coll_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                 request, errflag, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_cancel_send(MPIR_Request * sreq)
{
    return MPIDIG_mpi_cancel_send(sreq);
}

#endif /* SHM_AM_FALLBACK_SEND_H_INCLUDED */
