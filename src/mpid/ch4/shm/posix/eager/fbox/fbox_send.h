/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_EAGER_FBOX_SEND_H_INCLUDED
#define POSIX_EAGER_FBOX_SEND_H_INCLUDED

#include "fbox_impl.h"
#include "mpidu_genq.h"

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_eager_payload_limit(void)
{
    return MPIDI_POSIX_FBOX_DATA_LEN;
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_POSIX_eager_buf_limit(void)
{
    return MPIDI_POSIX_FBOX_DATA_LEN;
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
    MPIDI_POSIX_fastbox_t *fbox_out;
    size_t capacity, available;
    char *payload;
    int ret = MPIDI_POSIX_OK;
    MPI_Aint packed_size = 0;
    uint8_t *fbox_payload_ptr;
    size_t fbox_payload_size = MPIDI_POSIX_FBOX_DATA_LEN;
    size_t fbox_payload_size_left = MPIDI_POSIX_FBOX_DATA_LEN;

    MPIR_FUNC_ENTER;

    int dst_local_rank = MPIDI_POSIX_global.local_ranks[grank];
    fbox_out = MPIDI_POSIX_eager_fbox_control_global.mailboxes.out[dst_local_rank];

    if (MPL_atomic_load_int(&fbox_out->data_ready)) {
        ret = MPIDI_POSIX_NOK;
        goto fn_exit;
    }

    fbox_payload_ptr = fbox_out->payload;

    fbox_out->is_header = 0;
    fbox_out->payload_sz = 0;

    /* Get the memory allocated to be used for the message transportation. */
    payload = fbox_out->payload;

    /* Figure out the capacity of each cell */
    capacity = MPIDI_POSIX_FBOX_DATA_LEN;

    available = capacity;

    if (am_hdr) {
        msg_hdr->am_hdr_sz = am_hdr_sz;

        MPIR_Memcpy(payload, msg_hdr, sizeof(MPIDI_POSIX_am_header_t));
        payload += sizeof(MPIDI_POSIX_am_header_t);
        available -= sizeof(MPIDI_POSIX_am_header_t);

        fbox_out->is_header = 1;

        MPIR_Memcpy(payload, am_hdr, am_hdr_sz);
        payload += am_hdr_sz;
        available -= am_hdr_sz;
        fbox_out->payload_sz = capacity - available;
    } else {
        fbox_out->is_header = 0;
    }

    if (bytes_sent) {
        MPIR_Typerep_pack(buf, count, datatype, offset, payload, available, &packed_size,
                          MPIR_TYPEREP_FLAG_NONE);
        fbox_out->payload_sz += packed_size;
        *bytes_sent = packed_size;
    }

    MPL_atomic_store_int(&fbox_out->data_ready, 1);

  fn_exit:
    MPIR_FUNC_EXIT;
    return ret;
}

#endif /* POSIX_EAGER_FBOX_SEND_H_INCLUDED */
