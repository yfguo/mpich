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
#ifndef OFI_SPAWN_H_INCLUDED
#define OFI_SPAWN_H_INCLUDED

#include "ofi_impl.h"

int MPIDI_OFI_mpi_comm_connect(const char *port_name, MPIR_Info * info, int root, int timeout,
                               MPIR_Comm * comm_ptr, MPIR_Comm ** newcomm);
int MPIDI_OFI_mpi_comm_disconnect(MPIR_Comm * comm_ptr);
int MPIDI_OFI_mpi_open_port(MPIR_Info * info_ptr, char *port_name);
int MPIDI_OFI_mpi_close_port(const char *port_name);
int MPIDI_OFI_mpi_comm_accept(const char *port_name, MPIR_Info * info, int root,
                              MPIR_Comm * comm_ptr, MPIR_Comm ** newcomm);
#ifdef NETMOD_INLINE
#define MPIDI_NM_mpi_comm_connect MPIDI_NM_mpi_comm_connect
#define MPIDI_NM_mpi_comm_disconnect MPIDI_NM_mpi_comm_disconnect
#define MPIDI_NM_mpi_open_port MPIDI_NM_mpi_open_port
#define MPIDI_NM_mpi_close_port MPIDI_NM_mpi_close_port
#define MPIDI_NM_mpi_comm_accept MPIDI_NM_mpi_comm_accept
#endif

#endif /* OFI_SPAWN_H_INCLUDED */
