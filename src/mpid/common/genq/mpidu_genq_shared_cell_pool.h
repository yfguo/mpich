/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED
#define MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED

#include "mpidu_genq_common.h"
#define MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY 1
#include "mpidu_genq_shared_types.h"
#undef MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY
#include "mpidu_init_shm.h"
#include "mpl.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef void *MPIDU_genq_shared_cell_pool_t;

static inline int MPIDU_genq_shared_cell_pool_create(uintptr_t cell_size, uintptr_t cells_in_block,
                                       uintptr_t max_num_cells,
                                       MPIDU_genq_shared_cell_pool_t * pool);
static inline int MPIDU_genq_shared_cell_pool_destroy(MPIDU_genq_shared_cell_pool_t pool);
static inline int MPIDU_genq_shared_cell_pool_alloc(MPIDU_genq_shared_cell_pool_t pool, void **cell);
static inline int MPIDU_genq_shared_cell_pool_free(MPIDU_genq_shared_cell_pool_t pool, void *cell);

static inline int MPIDU_genqi_shared_cell_block_alloc(shared_cell_pool_s * pool, cell_block_s ** block);
static inline int MPIDU_genqi_shared_cell_block_alloc(shared_cell_pool_s * pool, cell_block_s ** block)
{
    int rc = MPI_SUCCESS;
    int block_idx = MPL_atomic_fetch_add_int((pool->global_block_index), 1);
    MPIR_ERR_CHKANDJUMP(block_idx >= pool->max_num_blocks, rc, MPI_ERR_OTHER, "**nomem");

    cell_block_s *new_block = (cell_block_s *) MPL_malloc(sizeof(cell_block_s), MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!new_block, rc, MPI_ERR_OTHER, "**nomem");

    new_block->cell_headers =
        (shared_cell_header_s **) MPL_malloc(pool->num_cells_in_block
                                             * sizeof(shared_cell_header_s), MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!new_block->cell_headers, rc, MPI_ERR_OTHER, "**nomem");

    new_block->next = NULL;

    /* init cell headers */
    int idx = block_idx * pool->num_cells_in_block;
    for (int i = 0; i < pool->num_cells_in_block; i++) {
        new_block->cell_headers[i] = &pool->all_cell_headers[idx + i];
        new_block->cell_headers[i]->next = 0;
        new_block->cell_headers[i]->prev = 0;
        new_block->cell_headers[i]->cell_offset = (uintptr_t) (idx + i) * pool->cell_size;
        MPL_atomic_store_int(&new_block->cell_headers[i]->in_use, 0);
    }

    *block = new_block;

  fn_exit:
    return rc;
  fn_fail:
    MPL_atomic_fetch_sub_int((pool->global_block_index), 1);
    MPL_free(new_block->cell_headers);
    goto fn_exit;
}

int MPIDU_genq_shared_cell_pool_create(uintptr_t cell_size, uintptr_t num_cells_in_block,
                                       uintptr_t max_num_cells,
                                       MPIDU_genq_shared_cell_pool_t * pool)
{
    int rc = MPI_SUCCESS;
    shared_cell_pool_s *pool_obj;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_CREATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_CREATE);

    pool_obj = MPL_malloc(sizeof(shared_cell_pool_s), MPL_MEM_OTHER);

    pool_obj->cell_size = cell_size;
    pool_obj->num_cells_in_block = num_cells_in_block;
    pool_obj->max_num_cells = max_num_cells;

    rc = MPIDU_Init_shm_alloc(sizeof(MPL_atomic_int_t), (void *) &pool_obj->global_block_index);
    MPIR_ERR_CHECK(rc);

    MPL_atomic_store_int(pool_obj->global_block_index, 0);

    rc = MPIDU_Init_shm_alloc(max_num_cells * sizeof(shared_cell_header_s),
                              (void *) &pool_obj->all_cell_headers);
    MPIR_ERR_CHECK(rc);

    rc = MPIDU_Init_shm_alloc(max_num_cells * cell_size, (void *) &pool_obj->all_cells);
    MPIR_ERR_CHECK(rc);

    pool_obj->num_blocks = 0;
    pool_obj->max_num_blocks = max_num_cells / num_cells_in_block;
    pool_obj->cell_blocks = NULL;

    *pool = (void *) pool_obj;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_CREATE);
    return rc;
  fn_fail:
    MPIDU_Init_shm_free(pool_obj->all_cells);
    MPIDU_Init_shm_free(pool_obj->all_cell_headers);
    MPIDU_Init_shm_free(pool_obj->global_block_index);
    MPL_free(pool_obj);
    goto fn_exit;
}

int MPIDU_genq_shared_cell_pool_destroy(MPIDU_genq_shared_cell_pool_t pool)
{
    int rc = MPI_SUCCESS;
    shared_cell_pool_s *pool_obj = (shared_cell_pool_s *) pool;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_DESTROY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_DESTROY);

    for (cell_block_s * block = pool_obj->cell_blocks; block;) {
        cell_block_s *next = block->next;
        MPL_free(block->cell_headers);
        MPL_free(block);
        block = next;
    }

    MPIDU_Init_shm_free(pool_obj->all_cells);
    MPIDU_Init_shm_free(pool_obj->all_cell_headers);
    MPIDU_Init_shm_free(pool_obj->global_block_index);

    /* free self */
    MPL_free(pool_obj);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_DESTROY);
    return rc;
}

int MPIDU_genq_shared_cell_pool_alloc(MPIDU_genq_shared_cell_pool_t pool, void **cell)
{
    int rc = MPI_SUCCESS;
    shared_cell_pool_s *pool_obj = (shared_cell_pool_s *) pool;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_ALLOC);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_ALLOC);

    /* iteratively find a unused cell */
    for (cell_block_s * block = pool_obj->cell_blocks; block; block = block->next) {
        for (int i = 0; i < pool_obj->num_cells_in_block; i++) {
            if (MPL_atomic_cas_int(&block->cell_headers[i]->in_use, 0, 1) == 0) {
                *cell = HEADER_TO_CELL(pool_obj, block->cell_headers[i]);
                goto fn_exit;
            }
        }
    }

    cell_block_s *new_block;
    rc = MPIDU_genqi_shared_cell_block_alloc(pool_obj, &new_block);
    MPIR_ERR_CHECK(rc);

    pool_obj->num_blocks++;

    if (pool_obj->cell_blocks == NULL) {
        pool_obj->cell_blocks = new_block;
    } else {
        cell_block_s *block;
        for (block = pool_obj->cell_blocks; block->next; block = block->next) {
        }
        block->next = new_block;
    }

    for (int i = 0; i < pool_obj->num_cells_in_block; i++) {
        if (MPL_atomic_cas_int(&new_block->cell_headers[i]->in_use, 0, 1) == 0) {
            *cell = HEADER_TO_CELL(pool_obj, new_block->cell_headers[i]);
            goto fn_exit;
        }
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_ALLOC);
    return rc;
  fn_fail:
    goto fn_exit;
}

int MPIDU_genq_shared_cell_pool_free(MPIDU_genq_shared_cell_pool_t pool, void *cell)
{
    int rc = MPI_SUCCESS;
    shared_cell_pool_s *pool_obj = (shared_cell_pool_s *) pool;
    shared_cell_header_s *cell_header = CELL_TO_HEADER(pool_obj, cell);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_FREE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_FREE);

    cell_header->next = 0;
    cell_header->prev = 0;
    MPL_atomic_store_int(&cell_header->in_use, 0);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_CELL_POOL_FREE);
    return rc;
}
#endif /* ifndef MPIDU_GENQ_SHARED_CELL_POOL_H_INCLUDED */
