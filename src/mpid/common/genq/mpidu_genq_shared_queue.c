/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidu_genq_shared_cell_pool.h"
#include "mpidu_genq_shared_queue.h"
#define MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY 1
#include "mpidu_genq_shared_types.h"
#undef MPIDU_GENQ_SHARED_IMPLEMENTATION_ONLY
#include "mpl.h"
#include "mpidimpl.h"

static inline shared_cell_header_s *get_head_cell_header(MPIDU_genq_shared_queue_t queue,
                                                         MPIDU_genq_shared_cell_pool_t pool);

static inline shared_cell_header_s *get_head_cell_header(MPIDU_genq_shared_queue_t queue,
                                                         MPIDU_genq_shared_cell_pool_t pool)
{
    void *tail = NULL;
    shared_cell_header_s *head_cell_h = NULL;

    /* prepares the cells for dequeuing from the head in the following steps.
     * 1. atomic detaching all cells frm the queue tail
     * 2. find the head of the queue and rebuild the "next" pointers for cells
     */
    tail = MPL_atomic_swap_ptr(&queue->tail.m, NULL);
    if (!tail) {
        return NULL;
    }
    head_cell_h = HANDLE_TO_HEADER(pool, tail);

    if (head_cell_h != NULL) {
        uintptr_t curr_handle = HEADER_TO_HANDLE(pool, head_cell_h);
        while (head_cell_h->prev) {
            shared_cell_header_s *prev_cell_h = HANDLE_TO_HEADER(pool, head_cell_h->prev);
            prev_cell_h->next = curr_handle;
            curr_handle = head_cell_h->prev;
            head_cell_h = prev_cell_h;
        }
        return head_cell_h;
    } else {
        return NULL;
    }
}

int MPIDU_genq_shared_queue_init(MPIDU_genq_shared_queue_t queue, unsigned flags)
{
    int rc = MPI_SUCCESS;

    queue->flags = flags;

    if (flags & MPIDU_GENQ_SHARED_QUEUE_SERIAL) {
        queue->head.s = 0;
        queue->tail.s = 0;
    } else {
        if (flags & MPIDU_GENQ_SHARED_QUEUE_MC) {
            MPL_atomic_store_ptr(&queue->head.m, NULL);
        } else {
            queue->head.s = 0;
        }
        /* sp and mp all use atomic tail */
        MPL_atomic_store_ptr(&queue->tail.m, NULL);
    }

    return rc;
}

int MPIDU_genq_shared_queue_dequeue(MPIDU_genq_shared_queue_t queue,
                                    MPIDU_genq_shared_cell_pool_t pool, void **cell)
{
    int rc = MPI_SUCCESS;
    shared_cell_header_s *cell_h = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_DEQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_DEQUEUE);

    if (queue->flags == MPIDU_GENQ_SHARED_QUEUE_TYPE__SERIAL) {
        cell_h = HANDLE_TO_HEADER(pool, queue->head.s);
        if (queue->head.s) {
            queue->head.s = cell_h->next;
            if (!cell_h->next) {
                queue->tail.s = 0;
            }
            *cell = HEADER_TO_CELL(pool, cell_h);
        } else {
            *cell = NULL;
        }
    } else {    /* MPIDU_GENQ_SHARED_QUEUE_TYPE__MPSC */
        if (!queue->head.s) {
            cell_h = get_head_cell_header(queue, pool);
            if (cell_h) {
                *cell = HEADER_TO_CELL(pool, cell_h);
                queue->head.s = cell_h->next;
            } else {
                *cell = NULL;
            }
        } else {
            cell_h = HANDLE_TO_HEADER(pool, queue->head.s);
            *cell = HEADER_TO_CELL(pool, cell_h);
            queue->head.s = cell_h->next;
        }
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_DEQUEUE);
    return rc;
}

int MPIDU_genq_shared_queue_enqueue(MPIDU_genq_shared_queue_t queue,
                                    MPIDU_genq_shared_cell_pool_t pool, void *cell)
{
    int rc = MPI_SUCCESS;
    shared_cell_header_s *cell_h = CELL_TO_HEADER(pool, cell);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_ENQUEUE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_ENQUEUE);

    if (queue->flags == MPIDU_GENQ_SHARED_QUEUE_TYPE__SERIAL) {
        uintptr_t handle = HEADER_TO_HANDLE(pool, cell_h);

        if (queue->tail.s) {
            HANDLE_TO_HEADER(pool, queue->tail.s)->next = handle;
        }
        cell_h->prev = queue->tail.s;
        queue->tail.s = handle;
        if (!queue->head.s) {
            queue->head.s = queue->tail.s;
        }
    } else {    /* MPIDU_GENQ_SHARED_QUEUE_TYPE__MPSC */
        void *prev_handle = NULL;
        void *handle = (void *) HEADER_TO_HANDLE(pool, cell_h);

        do {
            prev_handle = MPL_atomic_load_ptr(&queue->tail.m);
            cell_h->prev = (uintptr_t) prev_handle;
        } while (MPL_atomic_cas_ptr(&queue->tail.m, prev_handle, handle) != prev_handle);
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDU_GENQ_SHARED_QUEUE_ENQUEUE);
    return rc;
}

void *MPIDU_genq_shared_queue_head(MPIDU_genq_shared_queue_t queue,
                                   MPIDU_genq_shared_cell_pool_t pool)
{
    shared_cell_header_s *cell_h = NULL;

    if (queue->flags == MPIDU_GENQ_SHARED_QUEUE_SERIAL) {
        cell_h = HANDLE_TO_HEADER(pool, queue->head.s);
    } else {    /* MPIDU_GENQ_SHARED_QUEUE_TYPE__MPSC */
        if (!queue->head.s) {
            cell_h = get_head_cell_header(queue, pool);
        } else {
            cell_h = HANDLE_TO_HEADER(pool, queue->head.s);
        }

    }
    if (cell_h) {
        return HEADER_TO_CELL(pool, cell_h);
    } else {
        return NULL;
    }
}

void *MPIDU_genq_shared_queue_next(MPIDU_genq_shared_cell_pool_t pool, void *cell)
{
    return HEADER_TO_CELL(pool, CELL_TO_HEADER(pool, cell)->next);
}
