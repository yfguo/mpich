/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef OFI_SEND_COMMON_H_INCLUDED
#define OFI_SEND_COMMON_H_INCLUDED

#include "ofi_impl.h"

#define MPIDI_OFI_SEND_NEEDS_PACK (-1)  /* Could not use iov; needs packing */

/* Common macro used by all MPIDI_NM_mpi_send routines to facilitate tuning */
#define MPIDI_OFI_SEND_VNIS(vni_src_, vni_dst_) \
    do { \
        if (*request != NULL) { \
            /* workq path */ \
            vni_src_ = 0; \
            vni_dst_ = 0; \
        } else { \
            vni_src_ = MPIDI_OFI_get_vni(SRC_VCI_FROM_SENDER, comm, comm->rank, rank, tag); \
            vni_dst_ = MPIDI_OFI_get_vni(DST_VCI_FROM_SENDER, comm, comm->rank, rank, tag); \
        } \
    } while (0)

#endif /* ifndef OFI_SEND_COMMON_H_INCLUDED */
