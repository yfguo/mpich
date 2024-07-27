/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_FBOX_RECV_H_INCLUDED
#define POSIX_EAGER_FBOX_RECV_H_INCLUDED

#include "fbox_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX int
MPIDI_POSIX_eager_recv_begin(int vci, MPIDI_POSIX_eager_recv_transaction_t * transaction)
{
    int j, local_rank, grank;
    MPIDI_POSIX_fastbox_t *fbox_in;
    int mpi_errno = MPIDI_POSIX_NOK;

    MPIR_FUNC_ENTER;

    for (j = 0; j < MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE +
         MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_BATCH_SIZE; j++) {

        /* Before polling *all* of the fastboxes, poll the ones that are most likely to have messages
         * (where receives have been preposted). */
        if (j < MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE) {
            /* Get the next fastbox to poll. */
            local_rank = MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[j];
        }
        /* If we have finished the cached fastboxes, continue polling the rest of the fastboxes
         * where we left off last time. */
        else {
            int16_t last_cache =
                MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks
                [MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE];
            /* Figure out the next fastbox to poll by incrementing the counter. */
            last_cache = (last_cache + 1) % (int16_t) MPIR_Process.local_size;
            local_rank = last_cache;
            MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks
                [MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE] = last_cache;
        }

        if (local_rank == -1) {
            continue;
        }

        /* Find the correct fastbox and update the pointer for the next time around the loop. */
        fbox_in = MPIDI_POSIX_eager_fbox_control_global.mailboxes.in[local_rank];

        /* If the data ready flag is set, there is a message waiting. */
        if (MPL_atomic_load_int(&fbox_in->data_ready)) {
            /* Initialize public transaction part */
            grank = MPIDI_POSIX_global.local_procs[local_rank];

            if (likely(fbox_in->is_header)) {
                /* Only received the header for the message */
                transaction->msg_hdr = fbox_in->payload;
                transaction->payload = fbox_in->payload + sizeof(MPIDI_POSIX_am_header_t);
                transaction->payload_sz = fbox_in->payload_sz - sizeof(MPIDI_POSIX_am_header_t);
            } else {
                /* Received a fragment of the message payload */
                transaction->msg_hdr = NULL;
                transaction->payload = fbox_in->payload;
                transaction->payload_sz = fbox_in->payload_sz;
            }

            transaction->src_local_rank = local_rank;
            transaction->src_vci = 0;
            transaction->dst_vci = 0;

            /* Initialize private transaction part */
            transaction->transport.fbox.fbox_ptr = fbox_in;

            /* We found a message so return success and stop. */
            mpi_errno = MPIDI_POSIX_OK;
            goto fn_exit;
        }
    }

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
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
    MPIDI_POSIX_fastbox_t *fbox_in = NULL;

    MPIR_FUNC_ENTER;

    fbox_in = (MPIDI_POSIX_fastbox_t *) transaction->transport.fbox.fbox_ptr;

    MPL_atomic_store_int(&fbox_in->data_ready, 0);

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_posted_hook(int grank)
{
    int local_rank, i;

    MPIR_FUNC_ENTER;

    if (grank >= 0) {
        local_rank = MPIDI_POSIX_global.local_ranks[grank];

        /* Put the posted receive in the list of fastboxes to be polled first. If the list is full,
         * it will get polled after the boxes in the list are polled, which will be slower, but will
         * still match the message. */
        for (i = 0; i < MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE; i++) {
            if (MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] == -1) {
                MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] = local_rank;
                break;
            } else if (MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] ==
                       local_rank) {
                break;
            } else {
                continue;
            }
        }
    }

    MPIR_FUNC_EXIT;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_POSIX_eager_recv_completed_hook(int grank)
{
    int i, local_rank;

    MPIR_FUNC_ENTER;

    if (grank >= 0) {
        local_rank = MPIDI_POSIX_global.local_ranks[grank];

        /* Remove the posted receive from the list of fastboxes to be polled first now that the
         * request is done. */
        for (i = 0; i < MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE; i++) {
            if (MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] == local_rank) {
                MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] = -1;
                break;
            } else {
                continue;
            }
        }
    }

    MPIR_FUNC_EXIT;
}

#endif /* POSIX_EAGER_FBOX_RECV_H_INCLUDED */
