/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_QUICQ_PROGRESS_H_INCLUDED
#define POSIX_EAGER_QUICQ_PROGRESS_H_INCLUDED

#include "quicq_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_POSIX_eager_progress(int vci, int *made_progress)
{
    int ret = MPI_SUCCESS;
    MPIDI_POSIX_eager_quicq_transport_t *transport;
    MPIDI_POSIX_eager_quicq_terminal_t *terminal;

    *made_progress = 0;
    int max_vcis = MPIDI_POSIX_eager_quicq_global.max_vcis;
    for (int vci_src = 0; vci_src < max_vcis; vci_src++) {
        transport = MPIDI_POSIX_eager_quicq_get_transport(vci_src, vci);

        for (int src_local_rank = 0; src_local_rank < MPIR_Process.local_size;
             src_local_rank++) {
            if (src_local_rank == MPIR_Process.local_rank) {
                continue;
            }
            terminal = &transport->recv_terminals[src_local_rank];
            if (terminal->last_seq == terminal->last_ack) {
                uint64_t new_seq = MPL_atomic_acquire_load_uint64(&terminal->cntr->seq.a);
                if (new_seq != terminal->last_ack) {
                    terminal->last_seq = new_seq;
                    *made_progress = 1;
                }
            }
        }

        for (int dst_local_rank = 0; dst_local_rank < MPIR_Process.local_size;
             dst_local_rank++) {
            if (dst_local_rank == MPIR_Process.local_rank) {
                continue;
            }
            terminal = &transport->send_terminals[dst_local_rank];
            if (terminal->last_ack < terminal->last_seq) {
                /* TODO: what if recv completes out of order */
                uint64_t new_ack = MPL_atomic_acquire_load_uint64(&terminal->cntr->ack.a);
                if (new_ack != terminal->last_ack) {
                    MPIDI_POSIX_eager_quicq_cell_t *cell = NULL;
                    for (int i = terminal->last_ack; i < new_ack; i++) {
                        int cell_idx = MPIDI_POSIX_EAGER_QUICQ_CNTR_TO_IDX(i);
                        cell = terminal->cell_base + cell_idx * transport->cell_alloc_size;
                        if (cell->type & MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_EXTBUF) {
                            char *payload = MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell);
                            void *extbuf = ((MPIDI_POSIX_eager_quicq_extbuf_hdr *) payload)->buf;
                            MPIDU_genq_shmem_pool_cell_free(transport->extbuf_pool, extbuf);
                        }
                    }
                    terminal->last_ack = new_ack;
                    *made_progress = 1;
                }
            }
        }
    }

    return ret;
}

#endif /* POSIX_EAGER_QUICQ_PROGRESS_H_INCLUDED */
