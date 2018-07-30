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
#ifndef CH4_INIT_H_INCLUDED
#define CH4_INIT_H_INCLUDED

#include "ch4_impl.h"
#include "ch4r_proc.h"
#include "ch4i_comm.h"
#include "ch4_comm.h"
#include "strings.h"
#include "datatype.h"
#include "ch4r_recvq.h"

#ifdef USE_PMI2_API
/* PMI does not specify a max size for jobid_size in PMI2_Job_GetId.
   CH3 uses jobid_size=MAX_JOBID_LEN=1024 when calling
   PMI2_Job_GetId. */
#define MPIDI_MAX_JOBID_LEN PMI2_MAX_VALLEN
#endif

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

categories:
    - name        : CH4
      description : cvars that control behavior of the CH4 device

cvars:
    - name        : MPIR_CVAR_CH4_NETMOD
      category    : CH4
      type        : string
      default     : ""
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        If non-empty, this cvar specifies which network module to use

    - name        : MPIR_CVAR_CH4_SHM
      category    : CH4
      type        : string
      default     : ""
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        If non-empty, this cvar specifies which shm module to use

    - name        : MPIR_CVAR_CH4_ROOTS_ONLY_PMI
      category    : CH4
      type        : boolean
      default     : false
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_LOCAL
      description : >-
        Enables an optimized business card exchange over PMI for node root processes only.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

#if (MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY__POBJ)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY__GLOBAL)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY__SINGLE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#elif  (MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY__LOCKFREE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#else
#error "Thread Granularity:  Invalid"
#endif

int MPIDI_choose_netmod(void);

int MPID_Init(int *argc, char ***argv, int requested, int *provided, int *has_args, int *has_env);
int MPID_InitCompleted(void);
int MPID_Finalize(void);
int MPID_Get_universe_size(int *universe_size);
int MPID_Get_processor_name(char *name, int namelen, int *resultlen);
void *MPID_Alloc_mem(size_t size, MPIR_Info * info_ptr);
int MPID_Free_mem(void *ptr);

#undef FUNCNAME
#define FUNCNAME MPID_Comm_get_lpid
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPID_Comm_get_lpid(MPIR_Comm * comm_ptr,
                                                int idx, int *lpid_ptr, MPL_bool is_remote)
{
    int mpi_errno = MPI_SUCCESS;
    int avtid = 0, lpid = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_COMM_GET_LPID);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_COMM_GET_LPID);

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM)
        MPIDIU_comm_rank_to_pid(comm_ptr, idx, &lpid, &avtid);
    else if (is_remote)
        MPIDIU_comm_rank_to_pid(comm_ptr, idx, &lpid, &avtid);
    else {
        MPIDIU_comm_rank_to_pid_local(comm_ptr, idx, &lpid, &avtid);
    }

    *lpid_ptr = MPIDIU_LUPID_CREATE(avtid, lpid);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_COMM_GET_LPID);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Get_node_id
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPID_Get_node_id(MPIR_Comm * comm, int rank, int *id_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_GET_NODE_ID);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_GET_NODE_ID);

    MPIDI_CH4U_get_node_id(comm, rank, id_p);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_GET_NODE_ID);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPID_Get_max_node_id
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPID_Get_max_node_id(MPIR_Comm * comm, int *max_id_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_GET_MAX_NODE_ID);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_GET_MAX_NODE_ID);

    MPIDI_CH4U_get_max_node_id(comm, max_id_p);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_GET_MAX_NODE_ID);
    return mpi_errno;
}

int MPID_Create_intercomm_from_lpids(MPIR_Comm * newcomm_ptr, int size, const int lpids[]);


#undef FUNCNAME
#define FUNCNAME MPID_Aint_add
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX MPI_Aint MPID_Aint_add(MPI_Aint base, MPI_Aint disp)
{
    MPI_Aint result;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_AINT_ADD);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_AINT_ADD);
    result = MPIR_VOID_PTR_CAST_TO_MPI_AINT((char *) MPIR_AINT_CAST_TO_VOID_PTR(base) + disp);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_AINT_ADD);
    return result;
}

#undef FUNCNAME
#define FUNCNAME MPID_Aint_diff
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX MPI_Aint MPID_Aint_diff(MPI_Aint addr1, MPI_Aint addr2)
{
    MPI_Aint result;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPID_AINT_DIFF);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPID_AINT_DIFF);

    result = MPIR_PTR_DISP_CAST_TO_MPI_AINT((char *) MPIR_AINT_CAST_TO_VOID_PTR(addr1)
                                            - (char *) MPIR_AINT_CAST_TO_VOID_PTR(addr2));
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPID_AINT_DIFF);
    return result;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Type_commit_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_Type_commit_hook(MPIR_Datatype * type)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_TYPE_CREATE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_TYPE_CREATE_HOOK);

    mpi_errno = MPIDI_NM_mpi_type_commit_hook(type);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#ifndef MPIDI_CH4_DIRECT_NETMOD
    mpi_errno = MPIDI_SHM_mpi_type_commit_hook(type);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_TYPE_CREATE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Type_free_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_Type_free_hook(MPIR_Datatype * type)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_TYPE_FREE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_TYPE_FREE_HOOK);

    mpi_errno = MPIDI_NM_mpi_type_free_hook(type);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#ifndef MPIDI_CH4_DIRECT_NETMOD
    mpi_errno = MPIDI_SHM_mpi_type_free_hook(type);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_TYPE_FREE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Op_commit_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_Op_commit_hook(MPIR_Op * op)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OP_CREATE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OP_CREATE_HOOK);

    mpi_errno = MPIDI_NM_mpi_op_commit_hook(op);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#ifndef MPIDI_CH4_DIRECT_NETMOD
    mpi_errno = MPIDI_SHM_mpi_op_commit_hook(op);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OP_CREATE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Op_free_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_Op_free_hook(MPIR_Op * op)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_OP_FREE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_OP_FREE_HOOK);

    mpi_errno = MPIDI_NM_mpi_op_free_hook(op);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#ifndef MPIDI_CH4_DIRECT_NETMOD
    mpi_errno = MPIDI_SHM_mpi_op_free_hook(op);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_OP_FREE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* CH4_INIT_H_INCLUDED */
