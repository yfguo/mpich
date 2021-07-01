/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "ofi_impl.h"
#include "ofi_host_cb.h"

void MPIDI_OFI_init_host_cbs()
{
    MPIDI_OFI_global.host_cbs[MPIDI_OFI_SEND_LIGHTWEIGHT] = MPIDI_OFI_send_lightweight_cb;
    MPIDI_OFI_global.host_cbs[MPIDI_OFI_SEND_IOV] = MPIDI_OFI_send_iov_cb;
}

int MPIDI_OFI_send_lightweight_cb(MPIDI_OFI_host_cb_data_t * data)
{
    MPIDI_OFI_send_lightweight_data_t *d = data->send_lw;
    MPIDI_OFI_CALL_RETRY(fi_tinjectdata(d->ep,
                                        d->buf,
                                        d->data_sz,
                                        d->cq_data,
                                        d->dest_addr,
                                        d->tag),
                         d->vni_local, tinjectdata, d->comm->hints[MPIR_COMM_HINT_EAGAIN]);
    if (d->pack_buffer) {
        MPIR_gpu_free_host(d->pack_buffer);
    }
    MPL_free(data);
}

int MPIDI_OFI_send_iov_cb(MPIDI_OFI_host_cb_data_t * data)
{
    MPIDI_OFI_send_iov_data_t *d = data->send_iov;
    MPIDI_OFI_CALL_RETRY(fi_tinjectdata(d->ep, &d->msg, d->flags), d->vni_local, tsendv, FALSE);
    MPL_free(data);
}
