/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDU_GENQ_SHARED_QUEUE_H_INCLUDED
#define MPIDU_GENQ_SHARED_QUEUE_H_INCLUDED

#include "mpidu_genq_shared_cell_pool.h"
#include "mpl.h"
#include "mpidu_init_shm.h"
#include <stdint.h>

/* SP and SC are implied if MP and MC is not set */
#define MPIDU_GENQ_SHARED_QUEUE_SERIAL (0x1U)
#define MPIDU_GENQ_SHARED_QUEUE_MP     (0x10U)
#define MPIDU_GENQ_SHARED_QUEUE_MC     (0x100U)

typedef struct MPIDU_genq_shared_queue {
    union {
        uintptr_t s;
        MPL_atomic_ptr_t m;
        uint8_t pad[MPIDU_SHM_CACHE_LINE_LEN];
    } head;
    union {
        uintptr_t s;
        MPL_atomic_ptr_t m;
        uint8_t pad[MPIDU_SHM_CACHE_LINE_LEN];
    } tail;
    unsigned flags;
} MPIDU_genq_shared_queue_t;

int MPIDU_genq_shared_queue_init(MPIDU_genq_shared_queue_t * queue, unsigned flags);
int MPIDU_genq_shared_queue_dequeue(MPIDU_genq_shared_queue_t * queue,
                                    MPIDU_genq_shared_cell_pool_t * pool, void **cell);
int MPIDU_genq_shared_queue_dequeue_sc(MPIDU_genq_shared_queue_t * queue,
                                       MPIDU_genq_shared_cell_pool_t * pool, void **cell);
int MPIDU_genq_shared_queue_dequeue_mc(MPIDU_genq_shared_queue_t * queue,
                                       MPIDU_genq_shared_cell_pool_t * pool, void **cell);
int MPIDU_genq_shared_queue_enqueue(MPIDU_genq_shared_queue_t * queue,
                                    MPIDU_genq_shared_cell_pool_t * pool, void *cell);
int MPIDU_genq_shared_queue_enqueue_sp(MPIDU_genq_shared_queue_t * queue,
                                       MPIDU_genq_shared_cell_pool_t * pool, void *cell);
int MPIDU_genq_shared_queue_enqueue_mp(MPIDU_genq_shared_queue_t * queue,
                                       MPIDU_genq_shared_cell_pool_t * pool, void *cell);
void *MPIDU_genq_shared_queue_head(MPIDU_genq_shared_queue_t * queue,
                                   MPIDU_genq_shared_cell_pool_t * pool);
void *MPIDU_genq_shared_queue_next(MPIDU_genq_shared_cell_pool_t * pool, void *cell);

#define MPIDU_GENQ_SHARED_QUEUE_FOREACH(queue, pool, cell) \
    for (void *tmp = MPIDU_genq_shared_queue_head((queue), (pool)), cell = tmp; tmp; \
         tmp = MPIDU_genq_shared_queue_next((pool), tmp))

#endif /* ifndef MPIDU_GENQ_SHARED_QUEUE_H_INCLUDED */
