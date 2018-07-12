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
#ifndef CH4R_INIT_H_INCLUDED
#define CH4R_INIT_H_INCLUDED

// #include "ch4_impl.h"
// #include "ch4i_util.h"
// #include "ch4r_buf.h"
// #include "ch4r_callbacks.h"
// #include "ch4r_rma_target_callbacks.h"
// #include "ch4r_rma_origin_callbacks.h"
// #include "mpidig.h"
// #include "uthash.h"

int MPIDI_CH4U_init_comm(MPIR_Comm * comm);
int MPIDI_CH4U_destroy_comm(MPIR_Comm * comm);
void *MPIDI_CH4U_mpi_alloc_mem(size_t size, MPIR_Info * info_ptr);
int MPIDI_CH4U_mpi_free_mem(void *ptr);
int MPIDIU_update_node_map(int avtid, int size, int node_map[]);

#endif /* CH4R_INIT_H_INCLUDED */
