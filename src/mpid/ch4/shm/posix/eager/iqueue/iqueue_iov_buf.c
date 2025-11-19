/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include <mpidimpl.h>
#include "iqueue_iov_buf.h"

int MPIDI_POSIX_eager_iqueue_iov_buf_pool_init(void *slab, int cell_size, int num_cells,
                                               int idx_base,
                                               MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool)
{
    int ret = MPI_SUCCESS;

    pool->size = num_cells;
    pool->cell_size = cell_size;
    pool->idx_base = idx_base * num_cells;
    pool->slab = slab;
    void *base = slab + pool->idx_base * cell_size;
    memset(base, 0, num_cells * cell_size);

    pool->free_q = NULL;
    pool->headers = (MPIDI_POSIX_eager_iqueue_iov_buf_header_t *)
        MPL_malloc(num_cells * sizeof(MPIDI_POSIX_eager_iqueue_iov_buf_header_t), MPL_MEM_SHM);
    MPIR_Assert(pool->headers);
    for (int i = 0; i < num_cells; i++) {
        pool->headers[i].handle = (base + i * cell_size - slab) + 1;
        pool->headers[i].next = pool->free_q;
        pool->free_q = &pool->headers[i];
    }

    return ret;
}

void MPIDI_POSIX_eager_iqueue_iov_buf_pool_destroy(MPIDI_POSIX_eager_iqueue_iov_buf_pool_t * pool)
{
    MPL_free(pool->headers);
    pool->free_q = NULL;
}
