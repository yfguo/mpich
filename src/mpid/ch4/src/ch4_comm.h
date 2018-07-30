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
#ifndef CH4_COMM_H_INCLUDED
#define CH4_COMM_H_INCLUDED

#include "ch4_impl.h"
#include "ch4r_comm.h"
#include "ch4i_comm.h"

int MPID_Comm_AS_enabled(MPIR_Comm * comm);
int MPID_Comm_reenable_anysource(MPIR_Comm * comm, MPIR_Group ** failed_group_ptr);
int MPID_Comm_remote_group_failed(MPIR_Comm * comm, MPIR_Group ** failed_group_ptr);
int MPID_Comm_group_failed(MPIR_Comm * comm_ptr, MPIR_Group ** failed_group_ptr);
int MPID_Comm_failure_ack(MPIR_Comm * comm_ptr);
int MPID_Comm_failure_get_acked(MPIR_Comm * comm_ptr, MPIR_Group ** failed_group_ptr);
int MPID_Comm_revoke(MPIR_Comm * comm_ptr, int is_remote);
int MPID_Comm_get_all_failed_procs(MPIR_Comm * comm_ptr, MPIR_Group ** failed_group, int tag);
int MPIDI_Comm_split_type(MPIR_Comm * user_comm_ptr, int split_type, int key,
                          MPIR_Info * info_ptr, MPIR_Comm ** newcomm_ptr);
int MPIDI_Comm_create_hook(MPIR_Comm * comm);

int MPIDI_Comm_free_hook(MPIR_Comm * comm);

int MPID_Intercomm_exchange_map(MPIR_Comm * local_comm, int local_leader,
                                MPIR_Comm * peer_comm, int remote_leader,
                                int *remote_size, int **remote_lupids, int *is_low_group);

#endif /* CH4_COMM_H_INCLUDED */
