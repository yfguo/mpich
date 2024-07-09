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

        for (int src_local_rank = 0; src_local_rank < MPIR_Process.local_size; src_local_rank++) {
            if (src_local_rank == MPIR_Process.local_rank) {
                continue;
            }
            terminal = &transport->recv_terminals[src_local_rank];
            if (terminal->last_ack == terminal->last_seq) {
                uint64_t new_seq = MPL_atomic_acquire_load_uint64(&terminal->cntr->seq.a);
                if (new_seq != terminal->last_ack) {
                    terminal->last_seq = new_seq;
                } else {
                    continue;
                }
            }

            int cell_idx = MPIDI_POSIX_EAGER_QUICQ_CNTR_TO_IDX(terminal->last_ack);
            cell = terminal->cell_base + cell_idx * transport->cell_alloc_size;

            transaction->src_local_rank = cell->from;
            transaction->src_vci = vci_src;
            transaction->dst_vci = vci;
            transaction->payload_sz = cell->payload_size;

            if (likely(cell->type & MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_HDR)) {
                transaction->msg_hdr = &cell->am_header;
            } else {
                MPIR_Assert(cell->type & MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_DATA);
                transaction->msg_hdr = NULL;
            }

            if (cell->type & MPIDI_POSIX_EAGER_QUICQ_CELL_TYPE_EXTBUF) {
                /* switch the cell to the actual buffer */
                char *extbuf_hdr = MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell);
                uint64_t handle = ((MPIDI_POSIX_eager_quicq_extbuf_hdr *) extbuf_hdr)->handle;
                char *extbuf = MPIDU_genq_shmem_pool_handle_to_cell(transport->extbuf_pool, handle);
                MPIR_Assert(extbuf != NULL);
                transaction->payload = extbuf;
            } else {
                transaction->payload = MPIDI_POSIX_EAGER_QUICQ_CELL_PAYLOAD(cell);
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
    MPIDI_POSIX_eager_quicq_transport_t *transport;
    MPIDI_POSIX_eager_quicq_terminal_t *terminal;

    MPIR_FUNC_ENTER;

    transport = MPIDI_POSIX_eager_quicq_get_transport(transaction->src_vci, transaction->dst_vci);
    terminal = &transport->recv_terminals[transaction->src_local_rank];
    terminal->last_ack++;
    MPL_atomic_release_store_uint64(&terminal->cntr->ack.a, terminal->last_ack);

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_posted_hook(int grank)
{
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_completed_hook(int grank)
{
}

#endif /* POSIX_EAGER_QUICQ_RECV_H_INCLUDED */
