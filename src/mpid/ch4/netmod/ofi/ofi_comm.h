/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef OFI_COMM_H_INCLUDED
#define OFI_COMM_H_INCLUDED

int MPIDI_OFI_mpi_comm_create_hook(MPIR_Comm * comm);
int MPIDI_OFI_mpi_comm_free_hook(MPIR_Comm * comm);

#ifdef NETMOD_INLINE
#define MPIDI_NM_mpi_comm_create_hook MPIDI_OFI_mpi_comm_create_hook
#define MPIDI_NM_mpi_comm_free_hook MPIDI_OFI_mpi_comm_free_hook
#endif

#endif /* FI_COMM_H_INCLUDED */
