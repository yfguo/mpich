/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_IQUEUE_PROGRESS_H_INCLUDED
#define POSIX_EAGER_IQUEUE_PROGRESS_H_INCLUDED

#include "iqueue_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_eager_progress(int vci, int *made_progress)
{
    return MPI_SUCCESS;
}

#endif /* POSIX_EAGER_IQUEUE_PROGRESS_H_INCLUDED */
