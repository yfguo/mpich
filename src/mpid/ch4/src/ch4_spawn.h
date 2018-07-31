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
#ifndef CH4_SPAWN_H_INCLUDED
#define CH4_SPAWN_H_INCLUDED

#include "ch4_impl.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_CH4_COMM_CONNECT_TIMEOUT
      category    : CH4
      type        : int
      default     : 180
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_GROUP_EQ
      description : >-
        The default time out period in seconds for a connection attempt to the
        server communicator where the named port exists but no pending accept.
        User can change the value for a specified connection through its info
        argument.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

int MPID_Comm_spawn_multiple(int count, char *commands[], char **argvs[], const int maxprocs[],
                             MPIR_Info * info_ptrs[], int root, MPIR_Comm * comm_ptr,
                             MPIR_Comm ** intercomm, int errcodes[]);
int MPID_Comm_connect(const char *port_name, MPIR_Info * info, int root, MPIR_Comm * comm,
                      MPIR_Comm ** newcomm_ptr);
int MPID_Comm_disconnect(MPIR_Comm * comm_ptr);
int MPID_Open_port(MPIR_Info * info_ptr, char *port_name);
int MPID_Close_port(const char *port_name);
int MPID_Comm_accept(const char *port_name, MPIR_Info * info, int root, MPIR_Comm * comm,
                     MPIR_Comm ** newcomm_ptr);

#endif /* CH4_SPAWN_H_INCLUDED */
