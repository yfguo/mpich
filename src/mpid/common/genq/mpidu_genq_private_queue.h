/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDU_GENQ_PRIVATE_QUEUE_H_INCLUDED
#define MPIDU_GENQ_PRIVATE_QUEUE_H_INCLUDED

#include "mpidu_genq_common.h"
#include <stdint.h>

#define MPIDU_GENQ_PRIVATE_QUEUE_CONCURRENT (0x1)

typedef void *MPIDU_genq_private_queue_t;

int MPIDU_genq_private_queue_create(uintptr_t cell_size, uintptr_t num_cells_in_block,
                                    uintptr_t max_num_cells, MPIDU_genq_malloc_fn malloc_fn,
                                    MPIDU_genq_free_fn free_fn, unsigned flags,
                                    MPIDU_genq_private_queue_t ** queue);
int MPIDU_genq_private_queue_destroy(MPIDU_genq_private_queue_t * queue);
int MPIDU_genq_private_queue_alloc(MPIDU_genq_private_queue_t * queue, void **cell);
int MPIDU_genq_private_queue_free(MPIDU_genq_private_queue_t * queue, void *cell);
int MPIDU_genq_private_queue_enqueue(MPIDU_genq_private_queue_t * queue, void *cell);
int MPIDU_genq_private_queue_dequeue(MPIDU_genq_private_queue_t * queue, void **cell);
void *MPIDU_genq_private_queue_head(MPIDU_genq_private_queue_t * queue);
void *MPIDU_genq_private_queue_next(MPIDU_genq_private_queue_t * queue, void *cell);

#define MPIDU_GENQ_PRIVATE_QUEUE_FOREACH(queue, cell) \
    for (void *tmp = MPIDU_genq_private_queue_head((queue)), cell = tmp; tmp; \
         tmp = MPIDU_genq_private_queue_next((queue), tmp))

#endif /* ifndef MPIDU_GENQ_PRIVATE_QUEUE_H_INCLUDED */
