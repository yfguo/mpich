/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef NETMOD_AM_FALLBACK_SEND_H_INCLUDED
#define NETMOD_AM_FALLBACK_SEND_H_INCLUDED

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_send(const void *buf,
                                               MPI_Aint count,
                                               MPI_Datatype datatype,
                                               int rank,
                                               int tag,
                                               MPIR_Comm * comm, int context_offset,
                                               MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_mpi_send_new(buf, count, datatype, rank, tag, comm, context_offset, addr, request,
                               protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_ssend(const void *buf,
                                                MPI_Aint count,
                                                MPI_Datatype datatype,
                                                int rank,
                                                int tag,
                                                MPIR_Comm * comm, int context_offset,
                                                MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SSEND_REQ, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_mpi_ssend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_isend(const void *buf,
                                                MPI_Aint count,
                                                MPI_Datatype datatype,
                                                int rank,
                                                int tag,
                                                MPIR_Comm * comm, int context_offset,
                                                MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_mpi_isend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_issend(const void *buf,
                                                 MPI_Aint count,
                                                 MPI_Datatype datatype,
                                                 int rank,
                                                 int tag,
                                                 MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SSEND_REQ, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_mpi_ssend_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_send_coll(const void *buf, MPI_Aint count,
                                                MPI_Datatype datatype, int rank, int tag,
                                                MPIR_Comm * comm,
                                                int context_offset, MPIDI_av_entry_t * addr,
                                                MPIR_Request ** request, MPIR_Errflag_t * errflag)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_send_coll_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                request, errflag, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_isend_coll(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank, int tag,
                                                 MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request,
                                                 MPIR_Errflag_t * errflag)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPIR_Datatype *dt_ptr;
    MPL_pointer_attr_t attr;
    int protocol = MPIDIG_AM_PROTOCOL__EAGER;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIR_GPU_query_pointer_attr((char *) buf + (dt_true_lb), &attr);

    if (!MPIDIG_am_check_size_le_eager_limit(data_sz, MPIDIG_SEND, MPIDI_NM_am_eager_limit())) {
        if (!dt_contig || attr.type == MPL_GPU_POINTER_DEV) {
            protocol = MPIDIG_AM_PROTOCOL__PIPELINE;
        } else {
            protocol = MPIDIG_AM_PROTOCOL__RDMA_READ;
        }
    }
    return MPIDIG_isend_coll_new(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                 request, errflag, protocol);
}

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_mpi_cancel_send(MPIR_Request * sreq)
{
    return MPIDIG_mpi_cancel_send(sreq);
}

#endif /* NETMOD_AM_FALLBACK_SEND_H_INCLUDED */
