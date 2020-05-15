/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED
#define MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED

#include "mpidu_genq_common.h"
#include <stdint.h>

typedef void *MPIDU_genq_shared_cell_pool_t;

int MPIDU_genq_shared_cell_pool_create(uintptr_t cell_size, uintptr_t cells_in_block,
                                       uintptr_t max_num_cells,
                                       MPIDU_genq_shared_cell_pool_t ** pool);
int MPIDU_genq_shared_cell_pool_destroy(MPIDU_genq_shared_cell_pool_t pool);
int MPIDU_genq_shared_cell_pool_alloc(MPIDU_genq_shared_cell_pool_t pool, void **cell);
int MPIDU_genq_shared_cell_pool_free(MPIDU_genq_shared_cell_pool_t * pool, void *cell);

#endif /* ifndef MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED */
