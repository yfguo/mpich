/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_IQUEUE_IOV_BUF_H_INCLUDED
#define POSIX_EAGER_IQUEUE_IOV_BUF_H_INCLUDED

#include <mpidimpl.h>

typedef struct MPIDI_POSIX_eager_iqueue_iov_buf_header {
    uint64_t handle;
    struct MPIDI_POSIX_eager_iqueue_iov_buf_header *next;
} MPIDI_POSIX_eager_iqueue_iov_buf_header_t;

typedef struct {
    int size;
    int cell_size;
    int idx_base;
    void *slab;
    MPIDI_POSIX_eager_iqueue_iov_buf_header_t *headers;
    MPIDI_POSIX_eager_iqueue_iov_buf_header_t *free_q;
} MPIDI_POSIX_eager_iqueue_iov_buf_pool_t;

int MPIDI_POSIX_eager_iqueue_iov_buf_pool_init(void *slab, int cell_size, int num_cells,
                                               int idx_base,
                                               MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool);
void MPIDI_POSIX_eager_iqueue_iov_buf_pool_destroy(MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool);

MPL_STATIC_INLINE_PREFIX uint64_t
MPIDI_POSIX_eager_iqueue_iov_buf_pool_alloc(MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool)
{
    uint64_t handle = 0;
    if (pool->free_q) {
        handle = pool->free_q->handle;
        pool->free_q = pool->free_q->next;
    }
    return handle;
}

MPL_STATIC_INLINE_PREFIX int
MPIDI_POSIX_eager_iqueue_iov_buf_pool_free(MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool,
                                           uint64_t handle)
{
    uint64_t idx = (handle - 1) / pool->cell_size - pool->idx_base;
    pool->headers[idx].next = pool->free_q;
    pool->free_q = &pool->headers[idx];
    return 0;
}

MPL_STATIC_INLINE_PREFIX void
*MPIDI_POSIX_eager_iqueue_iov_buf_map_handle(MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool,
                                             uint64_t handle)
{
    void *addr = NULL;
    uint64_t offset = handle - 1;
    addr = pool->slab + offset;
    /* printf("map handle %0"PRIx64", from base %p, to addr %p\n", handle, pool->slab, addr); */
    return addr;
}

#endif /* POSIX_EAGER_IQUEUE_IOV_BUF_H_INCLUDED */
