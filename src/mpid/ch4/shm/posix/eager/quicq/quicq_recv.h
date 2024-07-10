/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_QUICQ_RECV_H_INCLUDED
#define POSIX_EAGER_QUICQ_RECV_H_INCLUDED

#include "quicq_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX int
MPIDI_POSIX_eager_recv_begin(int vci, MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    MPIDI_POSIX_eager_quicq_transport_t *transport;
    MPIDI_POSIX_eager_quicq_terminal_t *terminal;
    MPIDI_POSIX_eager_quicq_cell_t *cell = NULL;
    int ret = MPIDI_POSIX_NOK;

    MPIR_FUNC_ENTER;

    /* TODO: measure the latency overhead due to multiple vci */
    int max_vcis = MPIDI_POSIX_eager_quicq_global.max_vcis;
    for (int vci_src = 0; vci_src < max_vcis; vci_src++) {
        transport = MPIDI_POSIX_eager_quicq_get_transport(vci_src, vci);

        for (int src_local_rank = 0; src_local_rank < MPIR_Process.local_size;
             src_local_rank++) {
            if (src_local_rank == MPIR_Process.local_rank) {
                continue;
            }
            terminal = &transport->recv_terminals[src_local_rank];
            if (terminal->last_ack == terminal->last_seq) {
                continue;
            }

            int cell_idx = terminal->last_ack & MPIDI_POSIX_EAGER_QUICQ_CNTR_MASK;
            cell = terminal->cell_base + cell_idx * transport->cell_alloc_size;
            terminal->last_ack++;

            transaction->src_local_rank = cell->from;
            transaction->src_vci = vci_src;
            transaction->dst_vci = vci;
            transaction->payload = MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell);
            transaction->payload_sz = cell->payload_size;

            if (likely(cell->type == MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_HDR)) {
                transaction->msg_hdr = &cell->am_header;
            } else {
                MPIR_Assert(cell->type == MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_DATA);
                transaction->msg_hdr = NULL;
            }

            transaction->transport.quicq.pointer_to_cell = cell;

            ret = MPIDI_POSIX_OK;
            break;
        }
    }

    MPIR_FUNC_EXIT;
    return ret;
}

MPL_STATIC_INLINE_PREFIX void
MPIDI_POSIX_eager_recv_memcpy(MPIDI_POSIX_eager_recv_transaction_t * transaction,
                              void *dst, const void *src, size_t size)
{
    MPIR_Typerep_copy(dst, src, size, MPIR_TYPEREP_FLAG_NONE);
}

MPL_STATIC_INLINE_PREFIX void
MPIDI_POSIX_eager_recv_commit(MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    MPIDI_POSIX_eager_quicq_cell_t *cell;
    MPIDI_POSIX_eager_quicq_transport_t *transport;
    MPIDI_POSIX_eager_quicq_terminal_t *terminal;

    MPIR_FUNC_ENTER;

    transport = MPIDI_POSIX_eager_quicq_get_transport(transaction->src_vci, transaction->dst_vci);
    terminal = &transport->recv_terminals[transaction->src_local_rank];

    MPL_atomic_relaxed_store_uint32(&terminal->cntr->ack.a, terminal->last_ack);

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_posted_hook(int grank)
{
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_completed_hook(int grank)
{
}

#endif /* POSIX_EAGER_QUICQ_RECV_H_INCLUDED */
