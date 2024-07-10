/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_STUB_PROGRESS_H_INCLUDED
#define POSIX_EAGER_STUB_PROGRESS_H_INCLUDED

#include "stub_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_eager_progress(int vci, int *made_progress)
{
    MPIR_Assert(0);
    return 0;
}

#endif /* POSIX_EAGER_STUB_PROGRESS_H_INCLUDED */
