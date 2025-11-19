/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_IQUEUE_TYPES_H_INCLUDED
#define POSIX_EAGER_IQUEUE_TYPES_H_INCLUDED

#include <mpidimpl.h>
#include "mpidu_init_shm.h"
#include "mpidu_genq.h"
#include "iqueue_iov_buf.h"

#define MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_HDR 0x1
#define MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_DATA 0x2
#define MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_IOV_BUF 0x4

/* Each cell contains some data being communicated from one process to another.
 * The struct will be packed by default, occuping 16 bytes. Used in regular
 * queue and fast box */
typedef struct MPIDI_POSIX_eager_iqueue_cell {
    uint16_t type;              /* Type of cell (head/tail/etc.) */
    uint16_t from;              /* Who is the message in the cell from */
    uint32_t payload_size;      /* Size of the message in the cell */
    MPIDI_POSIX_am_header_t am_header;  /* If this cell is the beginning of a message, it will have
                                         * an active message header and this will point to it. */
} MPIDI_POSIX_eager_iqueue_cell_t;

typedef struct MPIDI_POSIX_eager_iqueue_cell_ext {
    MPIDI_POSIX_eager_iqueue_cell_t base;
    uint64_t iov_buf_handle;
} MPIDI_POSIX_eager_iqueue_cell_ext_t;

typedef struct {
    /* *INDENT-OFF* */
    _Alignas(MPL_CACHELINE_SIZE) MPL_atomic_uint64_t seq;
    _Alignas(MPL_CACHELINE_SIZE) MPL_atomic_uint64_t ack;
    /* *INDENT-ON* */
} MPIDI_POSIX_eager_iqueue_fbox_header_t;

typedef struct {
    MPIDI_POSIX_eager_iqueue_fbox_header_t *header;
    /* local cached version of seq and ack, starting from a new cache line */
    uint64_t last_seq;
    uint64_t last_ack;
    int size;
} MPIDI_POSIX_eager_iqueue_fbox_t;

typedef struct MPIDI_POSIX_eager_iqueue_transport {
    int num_cells;              /* The number of cells allocated to each terminal in this transport */
    int size_of_cell;           /* The size of each of the cells in this transport */
    int fb_num_cells;
    int fb_cell_size;
    MPIDU_genq_shmem_queue_u *terminals;        /* The list of all the terminals that
                                                 * describe each of the cells */
    MPIDU_genq_shmem_queue_t my_terminal;
    MPIDU_genq_shmem_pool_t cell_pool;
    MPIDI_POSIX_eager_iqueue_fbox_t *send_q;
    MPIDI_POSIX_eager_iqueue_fbox_t *recv_q;
    MPIDI_POSIX_eager_iqueue_iov_buf_pool_t pool;
} MPIDI_POSIX_eager_iqueue_transport_t;

typedef struct MPIDI_POSIX_eager_iqueue_global {
    int max_vcis;
    /* sizes for shmem slabs */
    int slab_size;
    int terminal_offset;
    int fbox_offset;
    int iov_buf_offset;
    /* shmem slabs */
    void *root_slab;
    void *all_vci_slab;
    /* 2d array indexed with [src_vci][dst_vci] */
    MPIDI_POSIX_eager_iqueue_transport_t transports[MPIDI_CH4_MAX_VCIS][MPIDI_CH4_MAX_VCIS];
} MPIDI_POSIX_eager_iqueue_global_t;

extern MPIDI_POSIX_eager_iqueue_global_t MPIDI_POSIX_eager_iqueue_global;

MPL_STATIC_INLINE_PREFIX MPIDI_POSIX_eager_iqueue_transport_t
    * MPIDI_POSIX_eager_iqueue_get_transport(int vci_src, int vci_dst)
{
    return &MPIDI_POSIX_eager_iqueue_global.transports[vci_src][vci_dst];
}

#define MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell) \
    ((char*)(cell) + sizeof(MPIDI_POSIX_eager_iqueue_cell_t))

#define MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport) \
    ((transport)->size_of_cell - sizeof(MPIDI_POSIX_eager_iqueue_cell_t))

#define MPIDI_POSIX_EAGER_IQUEUE_FBOX_CELL_CAPACITY(transport) \
    ((transport)->fb_cell_size - sizeof(MPIDI_POSIX_eager_iqueue_cell_t))

#define MPIDI_POSIX_EAGER_IQUEUE_FBOX_CELL_BY_CNTR(q, cntr) \
    ((char *) (q)->header + sizeof(MPIDI_POSIX_eager_iqueue_fbox_header_t) \
     + ((cntr) & ((q)->size - 1)) * MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_FBOX_CELL_SIZE)

#define CNTR_TO_RB_IDX(q, cntr) ((int) ((cntr) & ((q)->size - 1)))

#define MPIDI_POSIX_EAGER_IQUEUE_FBOX_BY_IDX(base, idx) \
    ((char *) (base) + (idx) * MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_FBOX_CELL_SIZE \
     * MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_FBOX_NUM_CELLS)

MPL_STATIC_INLINE_PREFIX void print_cntr(MPIDI_POSIX_eager_iqueue_transport_t *t, int peer,
                                         bool is_do_send)
{
    MPIDI_POSIX_eager_iqueue_fbox_t *s = NULL, *r = NULL;
    s = &t->send_q[peer];
    r = &t->recv_q[peer];
    if (is_do_send) {
        printf("%d: s->s %"PRIu64"(%2d) s->a %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               s->last_seq, CNTR_TO_RB_IDX(s, s->last_seq),
               s->last_ack, CNTR_TO_RB_IDX(s, s->last_ack)
               );
    } else {
        printf("%d:     r->s %"PRIu64"(%2d) r->a %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               r->last_seq, CNTR_TO_RB_IDX(r, r->last_seq),
               r->last_ack, CNTR_TO_RB_IDX(r, r->last_ack)
               );
    }
}

MPL_STATIC_INLINE_PREFIX void print_new_ack(MPIDI_POSIX_eager_iqueue_transport_t *t, int peer,
                                            MPIDI_POSIX_eager_iqueue_fbox_t *q,
                                            uint64_t new_value)
{
    MPIDI_POSIX_eager_iqueue_fbox_t *s = NULL, *r = NULL;
    s = &t->send_q[peer];
    r = &t->recv_q[peer];
    if (s == q) {
        printf("%d: UPDATE s->s %"PRIu64"(%2d) s->a %"PRIu64"(%2d) => %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               s->last_seq, CNTR_TO_RB_IDX(s, s->last_seq),
               s->last_ack, CNTR_TO_RB_IDX(s, s->last_ack),
               new_value, CNTR_TO_RB_IDX(s, new_value)
               );
    }
    if (r == q) {
        printf("%d: UPDATE     r->s %"PRIu64"(%2d) r->a %"PRIu64"(%2d) => %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               r->last_seq, CNTR_TO_RB_IDX(r, r->last_seq),
               r->last_ack, CNTR_TO_RB_IDX(r, r->last_ack),
               new_value, CNTR_TO_RB_IDX(r, new_value)
               );
    }
}

MPL_STATIC_INLINE_PREFIX void print_new_seq(MPIDI_POSIX_eager_iqueue_transport_t *t, int peer,
                                            MPIDI_POSIX_eager_iqueue_fbox_t *q,
                                            uint64_t new_value)
{
    MPIDI_POSIX_eager_iqueue_fbox_t *s = NULL, *r = NULL;
    s = &t->send_q[peer];
    r = &t->recv_q[peer];
    if (s == q) {
        printf("%d: UPDATE s->s %"PRIu64"(%2d) => %"PRIu64"(%2d) s->a %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               s->last_seq, CNTR_TO_RB_IDX(s, s->last_seq),
               new_value, CNTR_TO_RB_IDX(s, new_value),
               s->last_ack, CNTR_TO_RB_IDX(s, s->last_ack)
               );
    }
    if (r == q) {
        printf("%d: UPDATE     r->s %"PRIu64"(%2d) => %"PRIu64"(%2d) r->a %"PRIu64"(%2d)\n",
               MPIR_Process.rank,
               r->last_seq, CNTR_TO_RB_IDX(r, r->last_seq),
               new_value, CNTR_TO_RB_IDX(r, new_value),
               r->last_ack, CNTR_TO_RB_IDX(r, r->last_ack)
               );
    }
}
#endif /* POSIX_EAGER_IQUEUE_TYPES_H_INCLUDED */
