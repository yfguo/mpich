/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef OFL_PROGRESS_H_INCLUDED
#define OFL_PROGRESS_H_INCLUDED

#include "offload_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_progress(int vci, int *made_progress)
{
    int ret = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    MPIR_FUNC_EXIT;
    return ret;
}

#endif /* OFL_PROGRESS_H_INCLUDED */
