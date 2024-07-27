/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_FBOX_NOINLINE_H_INCLUDED
#define POSIX_EAGER_FBOX_NOINLINE_H_INCLUDED

#include "fbox_types.h"
#include "fbox_impl.h"

int MPIDI_POSIX_fbox_init(int rank, int size);
int MPIDI_POSIX_fbox_post_init(void);
int MPIDI_POSIX_fbox_finalize(void);

#ifdef POSIX_EAGER_INLINE
#define MPIDI_POSIX_eager_init MPIDI_POSIX_fbox_init
#define MPIDI_POSIX_eager_post_init MPIDI_POSIX_fbox_post_init
#define MPIDI_POSIX_eager_finalize MPIDI_POSIX_fbox_finalize
#endif

#endif /* POSIX_EAGER_FBOX_NOINLINE_H_INCLUDED */
