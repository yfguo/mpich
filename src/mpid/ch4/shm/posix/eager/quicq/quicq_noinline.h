/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_QUICQ_NOINLINE_H_INCLUDED
#define POSIX_EAGER_QUICQ_NOINLINE_H_INCLUDED

#include "quicq_types.h"
#include "quicq_impl.h"

int MPIDI_POSIX_quicq_init(int rank, int size);
int MPIDI_POSIX_quicq_post_init(void);
int MPIDI_POSIX_quicq_finalize(void);

#ifdef POSIX_EAGER_INLINE
#define MPIDI_POSIX_eager_init MPIDI_POSIX_quicq_init
#define MPIDI_POSIX_eager_post_init MPIDI_POSIX_quicq_post_init
#define MPIDI_POSIX_eager_finalize MPIDI_POSIX_quicq_finalize
#endif

#endif /* POSIX_EAGER_QUICQ_NOINLINE_H_INCLUDED */
