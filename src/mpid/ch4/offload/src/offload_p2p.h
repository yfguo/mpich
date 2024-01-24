/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef OFL_P2P_H_INCLUDED
#define OFL_P2P_H_INCLUDED

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_isend(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank, int tag,
                                                 MPIR_Comm * comm, int attr,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_cancel_send(MPIR_Request * sreq)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_irecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                 int rank, int tag, MPIR_Comm * comm,
                                                 int context_offset, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                  MPIR_Request * message)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_cancel_recv(MPIR_Request * rreq)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_improbe(int source, int tag, MPIR_Comm * comm,
                                                   int context_offset, int *flag,
                                                   MPIR_Request ** message, MPI_Status * status)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iprobe(int source, int tag, MPIR_Comm * comm,
                                                  int context_offset, int *flag,
                                                  MPI_Status * status)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

#endif /* OFL_P2P_H_INCLUDED */
