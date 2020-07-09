/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "posix_impl.h"
#include "posix_types.h"

MPIDI_POSIX_global_t MPIDI_POSIX_global = { 0 };

MPL_atomic_uint64_t *MPIDI_POSIX_shm_limit_counter = NULL;

MPIDI_POSIX_eager_iqueue_transport_t MPIDI_POSIX_eager_iqueue_transport_global = { 0 };
