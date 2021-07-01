/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef OFI_SEND_H_INCLUDED
#define OFI_SEND_H_INCLUDED

typedef struct MPIDI_OFI_send_lightweight_data {
    struct fid_ep *ep;
    const void *buf;
    size_t data_sz;
    uint64_t cq_data;
    fi_addr_t dest_addr;
    uint64_t tag;
    int vni_local;
    int hint;
    void *pack_buffer;
} MPIDI_OFI_send_lightweight_data_t;

typedef struct MPIDI_OFI_send_iov_data {
    struct fid_ep *ep;
    struct fi_msg_tagged msg;
    uint64_t flags;
    MPIR_Request *req;
    int vni_local;
} MPIDI_OFI_send_iov_data_t;

typedef union MPIDI_OFI_host_cb_data {
    MPIDI_OFI_send_lightweight_data_t send_lw;
    MPIDI_OFI_send_iov_data_t send_iov;
} MPIDI_OFI_host_cb_data_t;

enum {
    MPIDI_OFI_SEND_LIGHTWEIGHT = 0,
    MPIDI_OFI_SEND_IOV,
    MPIDI_OFI_NUM_HOST_CBS
};

typedef int (*MPIDI_OFI_host_cb_t) (MPIDI_OFI_host_cb_data_t * data);

void MPIDI_OFI_init_host_cbs();
int MPIDI_OFI_send_lightweight_cb(MPIDI_OFI_host_cb_data_t * data);
int MPIDI_OFI_send_iov_cb(MPIDI_OFI_host_cb_data_t * data);

#endif /* ifndef OFI_SEND_H_INCLUDED */
