/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_QUICQ_SEND_H_INCLUDED
#define POSIX_EAGER_QUICQ_SEND_H_INCLUDED

#include "quicq_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_eager_payload_limit(void)
{
    return MPIDI_POSIX_EAGER_QUICQ_CELL_SIZE - sizeof(MPIDI_POSIX_eager_quicq_cell_t);
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_eager_buf_limit(void)
{
    return MPIDI_POSIX_EAGER_QUICQ_CELL_SIZE;
}

/* This function attempts to send the next chunk of a message via the queue. If no cells are
 * available, this function will return and the caller is expected to queue the message for later
 * and retry.
 *
 * grank   - The global rank (the rank in MPI_COMM_WORLD) of the receiving process.
 * msg_hdr - The header of the message to be sent. This can be NULL if there is no header to be sent
 *           (such as if the header was sent in a previous chunk, am_hdr will be NULL too in this
 *           case.
 * am_hdr, am_hdr_sz    - am header this could be NULL if not sending the first chunk
 * buf, count, datatype - Data buffer and signature for the send buffer. They could be NULL in the
 *                        case of a header-only message
 * offset               - current offset.
 * bytes_sent           - output variable for how much data actually been sent, pass NULL if no data
 *                        need to be send
 */
MPL_STATIC_INLINE_PREFIX int
MPIDI_POSIX_eager_send(int grank, MPIDI_POSIX_am_header_t * msg_hdr, const void *am_hdr,
                       MPI_Aint am_hdr_sz, const void *buf, MPI_Aint count, MPI_Datatype datatype,
                       MPI_Aint offset, int src_vci, int dst_vci, MPI_Aint * bytes_sent)
{
    MPIDI_POSIX_eager_quicq_transport_t *transport;
    MPIDI_POSIX_eager_quicq_cell_t *cell;
    MPIDI_POSIX_eager_quicq_terminal_t *terminal;
    size_t capacity, available;
    char *payload;
    int ret = MPIDI_POSIX_OK;
    MPI_Aint packed_size = 0;

    MPIR_FUNC_ENTER;

    /* Get the transport object that holds all of the global variables. */
    transport = MPIDI_POSIX_eager_quicq_get_transport(src_vci, dst_vci);

    /* Try to get a new cell to hold the message */
    /* Select the appropriate free queue depending on whether we are using intra-NUMA or inter-NUMAfree
     * free queue. */
    int dst_local_rank = MPIDI_POSIX_global.local_ranks[grank];
    bool is_topo_local =
        (MPIDI_POSIX_global.local_rank_dist[dst_local_rank] == MPIDI_POSIX_DIST__LOCAL);

    /* Find the correct queue for this rank pair. */
    terminal = &transport->send_terminals[dst_local_rank];

    /* If a cell wasn't available, let the caller know that we weren't able to send the message
     * immediately. */
    if (unlikely(terminal->last_seq - terminal->last_ack >= transport->num_cells_per_queue)) {
        ret = MPIDI_POSIX_NOK;
        goto fn_exit;
    }

    int cell_idx = terminal->last_seq & MPIDI_POSIX_EAGER_QUICQ_CNTR_MASK;
    cell = terminal->cell_base + cell_idx * transport->cell_alloc_size;
    terminal->last_seq++;

    /* Get the memory allocated to be used for the message transportation. */
    payload = MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell);

    /* Figure out the capacity of each cell */
    capacity = MPIDI_POSIX_EAGER_QUICQ_CELL_CAPACITY(transport);

    available = capacity;

    cell->from = MPIR_Process.local_rank;

    /* If this is the beginning of the message, mark it as the head. Otherwise it will be the
     * tail. */
    cell->payload_size = 0;
    if (am_hdr) {
        MPI_Aint resized_am_hdr_sz = MPL_ROUND_UP_ALIGN(am_hdr_sz, MAX_ALIGNMENT);
        cell->am_header = *msg_hdr;
        cell->type = MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_HDR;
        /* send am_hdr if this is the first segment */
        if (is_topo_local) {
            MPIR_Typerep_copy(payload, am_hdr, am_hdr_sz, MPIR_TYPEREP_FLAG_NONE);
        } else {
            MPIR_Typerep_copy(payload, am_hdr, am_hdr_sz, MPIR_TYPEREP_FLAG_STREAM);
        }
        /* make sure the data region starts at the boundary of MAX_ALIGNMENT */
        payload = payload + resized_am_hdr_sz;
        cell->payload_size += resized_am_hdr_sz;
        cell->am_header.am_hdr_sz = resized_am_hdr_sz;
        available -= cell->am_header.am_hdr_sz;
    } else {
        cell->type = MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_DATA;
    }

    /* We want to skip packing of send buffer if there is no data to be sent . buf == NULL is
     * not a correct check here because derived datatype can use absolute address for displacement
     * which requires buffer address passed as MPI_BOTTOM which is usually NULL. count == 0 is also
     * not reliable because the derived datatype could have zero block size which contains no
     * data. */
    if (bytes_sent) {
        if (is_topo_local) {
            MPIR_Typerep_pack(buf, count, datatype, offset, payload, available, &packed_size,
                              MPIR_TYPEREP_FLAG_NONE);
        } else {
            MPIR_Typerep_pack(buf, count, datatype, offset, payload, available, &packed_size,
                              MPIR_TYPEREP_FLAG_STREAM);
        }
        cell->payload_size += packed_size;
        *bytes_sent = packed_size;
    }

    MPL_atomic_relaxed_store_uint32(&terminal->cntr->seq.a, terminal->last_seq);

  fn_exit:
    MPIR_FUNC_EXIT;
    return ret;
}

#endif /* POSIX_EAGER_QUICQ_SEND_H_INCLUDED */
