/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"

int MPIDI_OFL_mpi_comm_commit_pre_hook(MPIR_Comm * comm)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_comm_commit_post_hook(MPIR_Comm * comm)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_comm_set_hints(MPIR_Comm * comm, MPIR_Info * info)
{
    int ret;

    MPIR_Assert(0);

    return ret;
}

int MPIDI_OFL_mpi_comm_free_hook(MPIR_Comm * comm)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_type_commit_hook(MPIR_Datatype * type)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_type_free_hook(MPIR_Datatype * type)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_op_commit_hook(MPIR_Op * op)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_op_free_hook(MPIR_Op * op)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_create_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);
    MPIR_ERR_CHECK(ret);

  fn_exit:
    MPIR_FUNC_EXIT;
    return ret;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFL_mpi_win_allocate_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_allocate_shared_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_create_dynamic_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_attach_hook(MPIR_Win * win, void *base, MPI_Aint size)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_detach_hook(MPIR_Win * win, const void *base)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

    MPIR_FUNC_EXIT;
    return ret;
}

int MPIDI_OFL_mpi_win_free_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_ENTER;

    MPIR_Assert(0);

  fn_exit:
    MPIR_FUNC_EXIT;
    return ret;
  fn_fail:
    goto fn_exit;
}
