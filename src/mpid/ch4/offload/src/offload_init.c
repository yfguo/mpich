/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "offload_types.h"
#include "ch4_types.h"

int MPIDI_OFL_init_local(int *tag_bits)
{
    int mpi_errno = MPI_SUCCESS;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFL_init_world(void)
{
    int mpi_errno = MPI_SUCCESS;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFL_post_init(void)
{
    int mpi_errno = MPI_SUCCESS;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_OFL_mpi_finalize_hook(void)
{
    int ret;

    MPIR_FUNC_ENTER;

  fn_exit:
    MPIR_FUNC_EXIT;
    return ret;
  fn_fail:
    goto fn_exit;
}
