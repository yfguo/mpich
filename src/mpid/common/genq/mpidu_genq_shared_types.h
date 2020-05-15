/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDU_GENQ_SHARED_TYPES_H_INCLUDED
#define MPIDU_GENQ_SHARED_TYPES_H_INCLUDED

#ifdef MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY

#include "mpl.h"
#include "mpidu_init_shm.h"

typedef struct shared_cell_header shared_cell_header_t;

struct shared_cell_header {
    uintptr_t cell_offset;
    MPL_atomic_int_t in_use;
    uintptr_t next;
    uintptr_t prev;
};

typedef struct cell_block cell_block_t;
struct cell_block {
    shared_cell_header_t **cell_headers;
    cell_block_t *next;
};

typedef struct shared_cell_pool {
    uintptr_t cell_size;
    uintptr_t cell_alloc_size;
    uintptr_t num_cells_in_block;
    uintptr_t max_num_cells;

    void *all_cells;
    shared_cell_header_t *all_cell_headers;
    MPL_atomic_int_t *global_block_index;

    uintptr_t num_blocks;
    uintptr_t max_num_blocks;
    cell_block_t *cell_blocks;
} shared_cell_pool_t;

#define HEADER_TO_INDEX(pool, header) \
    (((uintptr_t) (header) - (uintptr_t) (((shared_cell_pool_t *) pool)->all_cell_headers)) \
     / sizeof(shared_cell_header_t))

#define CELL_TO_INDEX(pool, cell) \
    (((uintptr_t) (cell) - (uintptr_t) ((shared_cell_pool_t *) pool)->all_cells) \
     / ((shared_cell_pool_t *) pool)->cell_size)

#define INDEX_TO_HEADER(pool, index) \
    (&(((shared_cell_pool_t *) pool)->all_cell_headers[index]))

#define CELL_TO_HEADER(pool, cell) INDEX_TO_HEADER(pool, CELL_TO_INDEX(pool, cell))

#define HEADER_TO_HANDLE(pool, header) \
    ((uintptr_t) (header) - (uintptr_t) (((shared_cell_pool_t *) pool)->all_cell_headers) + 4)

#define HANDLE_TO_HEADER(pool, handle) \
    ((shared_cell_header_t *) ((uintptr_t) ((shared_cell_pool_t *) pool)->all_cell_headers \
                               + ((uintptr_t) handle) - 4))

#define HEADER_TO_CELL(pool, header) \
    (((char *) ((shared_cell_pool_t *) pool)->all_cells) + ((shared_cell_header_t *) header)->cell_offset)

#else
#error "This file is for inclusion in the shared MPIDU genq implementation only"
#endif /* MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY */

#endif /* ifndef MPIDU_GENQ_SHARED_TYPES_H_INCLUDED */
