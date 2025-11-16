/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_IQUEUE_RECV_H_INCLUDED
#define POSIX_EAGER_IQUEUE_RECV_H_INCLUDED

#include "iqueue_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX int
MPIDI_POSIX_eager_recv_begin(int vci, MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    MPIDI_POSIX_eager_iqueue_transport_t *transport;
    MPIDI_POSIX_eager_iqueue_cell_t *cell = NULL;
    int ret = MPIDI_POSIX_NOK;

    MPIR_FUNC_ENTER;

    /* TODO: measure the latency overhead due to multiple vci */
    int max_vcis = MPIDI_POSIX_eager_iqueue_global.max_vcis;
    for (int vci_src = 0; vci_src < max_vcis; vci_src++) {
        transport = MPIDI_POSIX_eager_iqueue_get_transport(vci_src, vci);

        if (MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_FBOX_ENABLE) {
            MPIDI_POSIX_eager_iqueue_fbox_t *q = NULL;
            for (int i = 0; i < MPIR_Process.local_size; i++) {
                q = &transport->recv_q[i];
                /* read atomic if local cache is empty */
                if (q->last_ack == q->last_seq) {
                    uint64_t new_seq = MPL_atomic_acquire_load_uint64(&q->header->seq);
                    /* if no update */
                    if (new_seq == q->last_seq) {
                        continue;
                    }
                    q->last_seq = new_seq;
                }
                cell = (MPIDI_POSIX_eager_iqueue_cell_t *)
                    MPIDI_POSIX_EAGER_IQUEUE_FBOX_CELL_BY_CNTR(q, q->last_ack);
                break;
            }
        } else {
            MPIDU_genq_shmem_queue_dequeue(transport->cell_pool, transport->my_terminal,
                                           (void **) &cell);
        }

        if (cell) {
            transaction->src_local_rank = cell->from;
            transaction->src_vci = vci_src;
            transaction->dst_vci = vci;
            if (cell->type & MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_IOV_BUF) {
                uint64_t handle = ((MPIDI_POSIX_eager_iqueue_cell_ext_t *) cell)->iov_buf_handle;
                transaction->payload = MPIDI_POSIX_eager_iqueue_iov_buf_map_handle(&transport->pool,
                                                                                   handle);
                /* printf("recv use iov buf, handle %0"PRIx64"\n", handle); */
            } else {
                transaction->payload = MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell);
            }
            transaction->payload_sz = cell->payload_size;

            if (likely(cell->type & MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_HDR)) {
                transaction->msg_hdr = &cell->am_header;
            } else {
                MPIR_Assert(cell->type & MPIDI_POSIX_EAGER_IQUEUE_CELL_TYPE_DATA);
                transaction->msg_hdr = NULL;
            }
            /* printf("recv type %0x, payload_size %d, from %d, am_type %d, am_hdr_sz %d, handler_id %d\n", */
            /*        cell->type, cell->payload_size, cell->from, */
            /*        cell->am_header.am_type, cell->am_header.am_hdr_sz, cell->am_header.handler_id); */

            transaction->transport.iqueue.pointer_to_cell = cell;

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
    MPIDI_POSIX_eager_iqueue_cell_t *cell;
    MPIDI_POSIX_eager_iqueue_transport_t *transport;

    MPIR_FUNC_ENTER;

    transport = MPIDI_POSIX_eager_iqueue_get_transport(transaction->src_vci, transaction->dst_vci);

    if (MPIR_CVAR_CH4_SHM_POSIX_IQUEUE_FBOX_ENABLE) {
        MPIDI_POSIX_eager_iqueue_fbox_t *q = &transport->recv_q[transaction->src_local_rank];
        q->last_ack++;
        MPL_atomic_release_store_uint64(&q->header->ack, q->last_ack);
    } else {
        cell = (MPIDI_POSIX_eager_iqueue_cell_t *) transaction->transport.iqueue.pointer_to_cell;
        MPIDU_genq_shmem_pool_cell_free(transport->cell_pool, cell);
    }

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_posted_hook(int grank)
{
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_completed_hook(int grank)
{
}

#endif /* POSIX_EAGER_IQUEUE_RECV_H_INCLUDED */
