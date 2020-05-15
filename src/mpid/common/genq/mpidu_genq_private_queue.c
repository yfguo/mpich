/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidu_genq_private_queue.h"
#include "mpl.h"
#include "mpidimpl.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct private_cell_header private_cell_header_t;
struct private_cell_header {
    void *cell;
    union {
        int n;
        MPL_atomic_int_t a;
    } in_use;
    private_cell_header_t *next;
    private_cell_header_t *prev;
};

typedef struct cell_block cell_block_t;
struct cell_block {
    private_cell_header_t *cell_headers;
    void **cells;
    void *slab;
    void *last_cell;
    cell_block_t *next;
};

typedef struct private_queue {
    uintptr_t cell_size;
    uintptr_t cell_alloc_size;
    uintptr_t num_cells_in_block;
    uintptr_t max_num_cells;
    int is_thread_safe;

    MPIDU_genq_malloc_fn malloc_fn;
    MPIDU_genq_free_fn free_fn;

    uintptr_t num_blocks;
    uintptr_t max_num_blocks;
    cell_block_t *cell_blocks;

    union {
        private_cell_header_t *n;
        MPL_atomic_ptr_t a;
    } head;
    union {
        private_cell_header_t *n;
        MPL_atomic_ptr_t a;
    } tail;
} private_queue_t;

static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

static private_cell_header_t *cell_to_header(private_queue_t * queue, void *cell);
static int cell_block_alloc(private_queue_t * queue, cell_block_t ** block);

static private_cell_header_t *cell_to_header(private_queue_t * queue, void *cell)
{
    for (cell_block_t * curr = queue->cell_blocks; curr; curr = curr->next) {
        if (cell <= curr->last_cell && cell >= curr->slab) {
            return &curr->cell_headers[(int) ((char *) cell - (char *) curr->slab)
                                       / queue->cell_size];
        }
    }
    return NULL;
}

int MPIDU_genq_private_queue_create(uintptr_t cell_size, uintptr_t num_cells_in_block,
                                    uintptr_t max_num_cells, MPIDU_genq_malloc_fn malloc_fn,
                                    MPIDU_genq_free_fn free_fn, unsigned flags,
                                    MPIDU_genq_private_queue_t ** queue)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_CREATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_CREATE);

    pthread_mutex_lock(&global_mutex);

    queue_obj = MPL_malloc(sizeof(private_queue_t), MPL_MEM_OTHER);

    queue_obj->cell_size = cell_size;
    queue_obj->num_cells_in_block = num_cells_in_block;
    queue_obj->max_num_cells = max_num_cells;

    queue_obj->is_thread_safe = flags & MPIDU_GENQ_PRIVATE_QUEUE_CONCURRENT;

    queue_obj->malloc_fn = malloc_fn;
    queue_obj->free_fn = free_fn;

    queue_obj->num_blocks = 0;
    queue_obj->max_num_blocks = max_num_cells / num_cells_in_block;
    queue_obj->cell_blocks = NULL;

    if (queue_obj->is_thread_safe) {
        MPL_atomic_store_ptr(&queue_obj->head.a, NULL);
        MPL_atomic_store_ptr(&queue_obj->tail.a, NULL);
    } else {
        queue_obj->head.n = NULL;
        queue_obj->tail.n = NULL;
    }

    *queue = (void *) queue_obj;

    pthread_mutex_unlock(&global_mutex);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_CREATE);
    return rc;
}

int MPIDU_genq_private_queue_destroy(MPIDU_genq_private_queue_t * queue)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj = (private_queue_t *) queue;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DESTROY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DESTROY);

    pthread_mutex_lock(&global_mutex);

    for (cell_block_t * block = queue_obj->cell_blocks; block;) {
        cell_block_t *next = block->next;

        queue_obj->free_fn(block->slab);
        MPL_free(block->cells);
        MPL_free(block->cell_headers);
        MPL_free(block);
        block = next;
    }

    /* free self */
    MPL_free(queue_obj);

    pthread_mutex_unlock(&global_mutex);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DESTROY);
    return rc;
}

static int cell_block_alloc(private_queue_t * queue, cell_block_t ** block)
{
    int rc = MPI_SUCCESS;
    cell_block_t *new_block = NULL;

    new_block = (cell_block_t *) MPL_malloc(sizeof(cell_block_t), MPL_MEM_OTHER);

    MPIR_ERR_CHKANDJUMP(!new_block, rc, MPI_ERR_OTHER, "**nomem");

    new_block->slab = queue->malloc_fn(queue->num_cells_in_block * queue->cell_alloc_size);
    MPIR_ERR_CHKANDJUMP(!new_block->slab, rc, MPI_ERR_OTHER, "**nomem");

    new_block->cell_headers =
        (private_cell_header_t *) MPL_malloc(queue->num_cells_in_block
                                             * sizeof(private_cell_header_t), MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!new_block->cell_headers, rc, MPI_ERR_OTHER, "**nomem");

    new_block->cells =
        (void **) MPL_malloc(queue->num_cells_in_block * sizeof(void *), MPL_MEM_OTHER);
    MPIR_ERR_CHKANDJUMP(!new_block->cells, rc, MPI_ERR_OTHER, "**nomem");

    /* init cell headers */
    for (int i = 0; i < queue->num_cells_in_block; i++) {
        if (queue->is_thread_safe) {
            MPL_atomic_store_int(&new_block->cell_headers[i].in_use.a, 0);
        } else {
            new_block->cell_headers[i].in_use.n = 0;
        }
        new_block->cell_headers[i].next = NULL;
        new_block->cell_headers[i].prev = NULL;
        new_block->cell_headers[i].cell = (char *) new_block->slab + i * queue->cell_size;
        new_block->cells[i] = new_block->cell_headers[i].cell;
    }

    new_block->last_cell = new_block->cells[queue->num_cells_in_block - 1];

    new_block->next = NULL;

    *block = new_block;

  fn_exit:
    return rc;
  fn_fail:
    goto fn_exit;
}

int MPIDU_genq_private_queue_alloc(MPIDU_genq_private_queue_t * queue, void **cell)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj = (private_queue_t *) queue;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ALLOC);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ALLOC);

    /* iteratively find a unused cell */
    for (cell_block_t * block = queue_obj->cell_blocks; block; block = block->next) {
        for (int i = 0; i < queue_obj->num_cells_in_block; i++) {
            if (queue_obj->is_thread_safe) {
                if (MPL_atomic_cas_int(&block->cell_headers[i].in_use.a, 0, 1) == 0) {
                    *cell = block->cell_headers[i].cell;
                    goto fn_exit;
                }
            } else {
                if (!block->cell_headers[i].in_use.n) {
                    *cell = block->cell_headers[i].cell;
                    goto fn_exit;
                }
            }
        }
    }

    /* try allocate more blocks if no free cell found */
    MPIR_Assert(queue_obj->num_blocks <= queue_obj->max_num_blocks);
    if (queue_obj->num_blocks == queue_obj->max_num_blocks) {
        *cell = NULL;
        goto fn_exit;
    }

    cell_block_t *new_block;
    rc = cell_block_alloc(queue_obj, &new_block);
    MPIR_ERR_CHECK(rc);

    queue_obj->num_blocks++;

    if (queue_obj->cell_blocks == NULL) {
        queue_obj->cell_blocks = new_block;
    } else {
        cell_block_t *block;
        for (block = queue_obj->cell_blocks; block->next; block = block->next) {
        }
        block->next = new_block;
    }

    for (int i = 0; i < queue_obj->num_cells_in_block; i++) {
        if (queue_obj->is_thread_safe) {
            if (MPL_atomic_cas_int(&new_block->cell_headers[i].in_use.a, 0, 1) == 0) {
                *cell = new_block->cell_headers[i].cell;
                goto fn_exit;
            }
        } else {
            if (!new_block->cell_headers[i].in_use.n) {
                *cell = new_block->cell_headers[i].cell;
                goto fn_exit;
            }
        }
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ALLOC);
    return rc;
  fn_fail:
    goto fn_exit;
}

int MPIDU_genq_private_queue_free(MPIDU_genq_private_queue_t * queue, void *cell)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj = (private_queue_t *) queue;
    private_cell_header_t *cell_header = cell_to_header(queue_obj, cell);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_FREE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_FREE);

    cell_header->next = NULL;
    cell_header->prev = NULL;
    if (queue_obj->is_thread_safe) {
        MPL_atomic_store_int(&cell_header->in_use.a, 0);
    } else {
        cell_header->in_use.n = 0;
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_FREE);
    return rc;
}

int MPIDU_genq_private_queue_enqueue(MPIDU_genq_private_queue_t * queue, void *cell)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj = (private_queue_t *) queue;
    private_cell_header_t *cell_h = cell_to_header(queue_obj, cell);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ENQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ENQUEUE);

    if (queue_obj->is_thread_safe) {
        private_cell_header_t *tail_h = MPL_atomic_load_ptr(&queue_obj->tail.a);
        while (MPL_atomic_cas_ptr(&queue_obj->tail.a, (void *) tail_h, (void *) cell_h)
               != (void *) tail_h) {
            tail_h = MPL_atomic_load_ptr(&queue_obj->tail.a);
        }
        if (tail_h) {
            tail_h->next = cell_h;
        } else {
            MPL_atomic_cas_ptr(&queue_obj->head.a, (void *) tail_h, cell_h);
        }
        cell_h->prev = tail_h;
    } else {
        if (queue_obj->tail.n) {
            queue_obj->tail.n->next = cell_h;
        }
        cell_h->prev = queue_obj->tail.n;
        queue_obj->tail.n = cell_h;
        if (!queue_obj->head.n) {
            queue_obj->head.n = queue_obj->tail.n;
        }
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_ENQUEUE);
    return rc;
  fn_fail:
    goto fn_exit;
}

int MPIDU_genq_private_queue_dequeue(MPIDU_genq_private_queue_t * queue, void **cell)
{
    int rc = MPI_SUCCESS;
    private_queue_t *queue_obj = (private_queue_t *) queue;
    private_cell_header_t *cell_h = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DEQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DEQUEUE);

    if (queue_obj->is_thread_safe) {
        cell_h = MPL_atomic_load_ptr(&queue_obj->head.a);
        if (cell_h) {
            while (MPL_atomic_cas_ptr(&queue_obj->head.a, (void *) cell_h, (void *) cell_h->next)
                   != (void *) cell_h) {
                cell_h = MPL_atomic_load_ptr(&queue_obj->head.a);
            }
            if (!cell_h->next) {
                MPL_atomic_cas_ptr(&queue_obj->tail.a, (void *) cell_h, NULL);
            }
            *cell = cell_h->cell;
        }
    } else {
        cell_h = queue_obj->head.n;
        if (cell_h) {
            queue_obj->head.n = queue_obj->head.n->next;
            if (!cell_h->next) {
                queue_obj->tail.n = NULL;
            }
            *cell = cell_h->cell;
        } else {
            *cell = NULL;
        }
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_PRIVATE_QUEUE_DEQUEUE);
    return rc;
  fn_fail:
    goto fn_exit;
}

void *MPIDU_genq_private_queue_head(MPIDU_genq_private_queue_t * queue)
{
    private_queue_t *queue_obj = (private_queue_t *) queue;
    private_cell_header_t *cell_h = NULL;

    if (queue_obj->is_thread_safe) {
        cell_h = MPL_atomic_load_ptr(&queue_obj->head.a);
    } else {
        cell_h = queue_obj->head.n;
    }
    if (cell_h) {
        return cell_h->cell;
    } else {
        return NULL;
    }
}

void *MPIDU_genq_private_queue_next(MPIDU_genq_private_queue_t * queue, void *cell)
{
    return cell_to_header((private_queue_t *) queue, cell)->next;
}
