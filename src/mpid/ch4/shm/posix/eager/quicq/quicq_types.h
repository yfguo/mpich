/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_QUICQ_TYPES_H_INCLUDED
#define POSIX_EAGER_QUICQ_TYPES_H_INCLUDED

#include <mpidimpl.h>
#include "mpidu_init_shm.h"
#include "mpidu_genq.h"

#define MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_EXTBUF (0x1)
#define MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_HDR (0x2)
#define MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_DATA (0x4)

/* The number of cells per queue is always rounded up to the next power of two number during
 * init. In this way we can convert counter value to index by just masking the higher bits.
 * The counter has 64 bits. We do not check counter overflow as it would be extremely
 * unlikely to happen. Assuming each rank can send and recv at 100M messages per second, it
 * would take more than 5000 years to overflow.
 */
#define MPIDI_POSIX_EAGER_QUICQ_CNTR_MASK ((uint64_t) MPIR_CVAR_CH4_SHM_POSIX_QUICQ_NUM_CELLS - 1)
#define MPIDI_POSIX_EAGER_QUICQ_CNTR_TO_IDX(cntr) ((cntr) & MPIDI_POSIX_EAGER_QUICQ_CNTR_MASK)

typedef struct MPIDI_POSIX_eager_quicq_cell MPIDI_POSIX_eager_quicq_cell_t;

/* Each cell contains some data being communicated from one process to another. */
struct MPIDI_POSIX_eager_quicq_cell {
    uint16_t type;              /* Type of cell (head/tail/etc.) */
    uint16_t from;              /* Who is the message in the cell from */
    uint32_t payload_size;      /* Size of the message in the cell */
    MPIDI_POSIX_am_header_t am_header;  /* If this cell is the beginning of a message, it will have
                                         * an active message header and this will point to it. */
};

typedef struct MPIDI_POSIX_eager_quicq_extbuf_hdr {
    uint64_t handle;
    void *buf;
} MPIDI_POSIX_eager_quicq_extbuf_hdr;

typedef struct MPIDI_POSIX_eager_quicq_cntr {
    union {
        MPL_atomic_uint64_t a;
        char pad[MPL_CACHELINE_SIZE];
    } seq;
    union {
        MPL_atomic_uint64_t a;
        char pad[MPL_CACHELINE_SIZE];
    } ack;
} MPIDI_POSIX_eager_quicq_cntr_t;

typedef struct MPIDI_POSIX_eager_quicq_terminal {
    void *cell_base;
    MPIDI_POSIX_eager_quicq_cntr_t *cntr;
    uint64_t last_seq;
    uint64_t last_ack;
} MPIDI_POSIX_eager_quicq_terminal_t;

typedef struct MPIDI_POSIX_eager_quicq_transport {
    int size_of_cell;
    int cell_alloc_size;
    int num_cells_per_queue;
    int num_queues;
    void *shm_base;
    void **cell_bases;
    MPIDI_POSIX_eager_quicq_terminal_t *send_terminals;
    MPIDI_POSIX_eager_quicq_terminal_t *recv_terminals;
    MPIDU_genq_shmem_pool_t extbuf_pool;
} MPIDI_POSIX_eager_quicq_transport_t;

typedef struct MPIDI_POSIX_eager_quicq_global {
    int max_vcis;
    /* 2d array indexed with [src_vci][dst_vci] */
    MPIDI_POSIX_eager_quicq_transport_t transports[MPIDI_CH4_MAX_VCIS][MPIDI_CH4_MAX_VCIS];
} MPIDI_POSIX_eager_quicq_global_t;

extern MPIDI_POSIX_eager_quicq_global_t MPIDI_POSIX_eager_quicq_global;

MPL_STATIC_INLINE_PREFIX MPIDI_POSIX_eager_quicq_transport_t
    * MPIDI_POSIX_eager_quicq_get_transport(int vci_src, int vci_dst)
{
    return &MPIDI_POSIX_eager_quicq_global.transports[vci_src][vci_dst];
}

#define MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell) \
    ((char*)(cell) + sizeof(MPIDI_POSIX_eager_quicq_cell_t))

#define MPIDI_POSIX_EAGER_QUICQ_CELL_CAPACITY(transport) \
    ((transport)->cell_alloc_size - sizeof(MPIDI_POSIX_eager_quicq_cell_t))

#endif /* POSIX_EAGER_QUICQ_TYPES_H_INCLUDED */
