/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2017 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include "mpiimpl.h"
#include "tsp_gentran.h"
#include "gentran_utils.h"
#include "../iallreduce/iallreduce_tsp_recexch_algos_prototypes.h"
#include "../iallreduce/iallreduce_tsp_recexch_reduce_scatter_recexch_allgatherv_algos_prototypes.h"
#include "../iallreduce/iallreduce_tsp_tree_algos_prototypes.h"
#include "../iallreduce/iallreduce_tsp_ring_algos_prototypes.h"

/* -- Begin Profiling Symbol Block for routine MPIX_Allreduce_init */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPIX_Allreduce_init = PMPIX_Allreduce_init
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPIX_Allreduce_init  MPIX_Allreduce_init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPIX_Allreduce_init as PMPIX_Allreduce_init
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPIX_Allreduce_init(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                        MPI_Op op, MPI_Comm comm, MPI_Info info, MPI_Request * request)
    __attribute__ ((weak, alias("PMPIX_Allreduce_init")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPIX_Allreduce_init
#define MPIX_Allreduce_init PMPIX_Allreduce_init

static int MPIR_Allreduce_sched_intra_auto(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm_ptr,
                                           MPIR_TSP_sched_t * sched)
{
    int mpi_errno = MPI_SUCCESS;
    mpi_errno = MPIR_TSP_Iallreduce_sched_intra_recexch(sendbuf, recvbuf, count, datatype,
                                                        op, comm_ptr,
                                                        MPIR_IALLREDUCE_RECEXCH_TYPE_MULTIPLE_BUFFER,
                                                        MPIR_CVAR_IALLREDUCE_RECEXCH_KVAL, sched);

    return mpi_errno;
}

int MPIR_Allreduce_init(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                        MPI_Op op, MPIR_Comm * comm_ptr, MPIR_Info * info_ptr,
                        MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_TSP_sched_t *sched;
    int is_commutative = MPIR_Op_is_commutative(op);
    int nranks = comm_ptr->local_size;

    /* create a new request */
    MPIR_Request *req = MPIR_Request_create(MPIR_REQUEST_KIND__PREQUEST_COLL);
    if (!req)
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**nomem");

    req->u.persist.real_request = NULL;

    /* generate the schedule */
    sched = MPL_malloc(sizeof(MPIR_TSP_sched_t), MPL_MEM_COLL);
    MPIR_ERR_CHKANDJUMP(!sched, mpi_errno, MPI_ERR_OTHER, "**nomem");
    MPIR_TSP_sched_create(sched, true);

    switch (MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM) {
        case MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM_gentran_recexch_single_buffer:
            mpi_errno =
                MPIR_TSP_Iallreduce_sched_intra_recexch(sendbuf, recvbuf, count, datatype, op,
                                                        comm_ptr,
                                                        MPIR_IALLREDUCE_RECEXCH_TYPE_SINGLE_BUFFER,
                                                        MPIR_CVAR_IALLREDUCE_RECEXCH_KVAL, sched);
            break;
        case MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM_gentran_recexch_multiple_buffer:
            mpi_errno =
                MPIR_TSP_Iallreduce_sched_intra_recexch(sendbuf, recvbuf, count, datatype, op,
                                                        comm_ptr,
                                                        MPIR_IALLREDUCE_RECEXCH_TYPE_MULTIPLE_BUFFER,
                                                        MPIR_CVAR_IALLREDUCE_RECEXCH_KVAL, sched);
            break;
        case MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM_gentran_tree:
            /*Only knomial_1 tree supports non-commutative operations */
            MPII_COLLECTIVE_FALLBACK_CHECK(comm_ptr->rank, is_commutative ||
                                           MPIR_Iallreduce_tree_type == MPIR_TREE_TYPE_KNOMIAL_1,
                                           mpi_errno,
                                           "Pallreduce gentran_tree cannot be applied.\n");
            mpi_errno =
                MPIR_TSP_Iallreduce_sched_intra_tree(sendbuf, recvbuf, count, datatype, op,
                                                     comm_ptr, MPIR_Iallreduce_tree_type,
                                                     MPIR_CVAR_IALLREDUCE_TREE_KVAL,
                                                     MPIR_CVAR_IALLREDUCE_TREE_PIPELINE_CHUNK_SIZE,
                                                     MPIR_CVAR_IALLREDUCE_TREE_BUFFER_PER_CHILD,
                                                     sched);
            break;
        case MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM_gentran_ring:
            MPII_COLLECTIVE_FALLBACK_CHECK(comm_ptr->rank, is_commutative, mpi_errno,
                                           "Pallreduce gentran_ring cannot be applied.\n");
            mpi_errno =
                MPIR_TSP_Iallreduce_sched_intra_ring(sendbuf, recvbuf, count, datatype,
                                                     op, comm_ptr, sched);
            break;
        case MPIR_CVAR_IALLREDUCE_INTRA_ALGORITHM_gentran_recexch_reduce_scatter_recexch_allgatherv:
            MPII_COLLECTIVE_FALLBACK_CHECK(comm_ptr->rank, is_commutative &&
                                           count >= nranks, mpi_errno,
                                           "Pallreduce gentran_recexch_reduce_scatter_recexch_allgatherv cannot be applied.\n");
            /*  This algorithm will work for commutative operations and if the count is
             * bigger than total number of ranks. If it not commutative or if the count < nranks,
             * MPIR_Iallreduce_sched algorithm will be run */
            mpi_errno =
                MPIR_TSP_Iallreduce_sched_intra_recexch_reduce_scatter_recexch_allgatherv
                (sendbuf, recvbuf, count, datatype, op, comm_ptr,
                 MPIR_CVAR_IALLREDUCE_RECEXCH_KVAL, sched);
            break;
        default:
            mpi_errno = MPIR_Allreduce_sched_intra_auto(sendbuf, recvbuf, count, datatype,
                                                        op, comm_ptr, sched);
            break;
    }

    MPIR_ERR_CHECK(mpi_errno);
  fallback:
    mpi_errno =
        MPIR_Allreduce_sched_intra_auto(sendbuf, recvbuf, count, datatype, op, comm_ptr, sched);


    req->u.persist.sched = sched;

    *request = req;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif

/*@
MPIX_Allreduce_init - Creates a nonblocking, persistent collective communication request for the allreduce operation.

Input Parameters:
+ sendbuf - starting address of send buffer (choice)
. count - number of elements in send buffer (integer)
. datatype - data type of elements of send buffer (handle)
. op - operation (handle)
. info - info argument (handle)
. comm - communicator (handle)


Output Parameters:
. recvbuf - starting address of receive buffer (choice)
. request - communication request (handle)

.N ThreadSafe

.N Fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_BUFFER
.N MPI_ERR_ROOT

.seealso: MPI_Start, MPI_Startall, MPI_Request_free
@*/
int MPIX_Allreduce_init(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                        MPI_Op op, MPI_Comm comm, MPI_Info info, MPI_Request * request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Info *info_ptr = NULL;
    MPIR_Request *request_ptr = NULL;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIX_ALLREDUCE_INIT);

    MPIR_ERRTEST_INITIALIZED_ORDIE();

    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPIR_FUNC_TERSE_PT2PT_ENTER(MPID_STATE_MPIX_ALLREDUCE_INIT);

    /* Validate handle parameters needing to be converted */
#ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            MPIR_ERRTEST_COMM(comm, mpi_errno);
        }
        MPID_END_ERROR_CHECKS;
    }
#endif /* HAVE_ERROR_CHECKING */

    /* Convert MPI object handles to object pointers */
    MPIR_Comm_get_ptr(comm, comm_ptr);
    MPIR_Info_get_ptr(info, info_ptr);

    /* Validate parameters and objects (post conversion) */
#ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            MPIR_Datatype *datatype_ptr = NULL;
            MPIR_Op *op_ptr = NULL;

            MPIR_Comm_valid_ptr(comm_ptr, mpi_errno, FALSE);
            if (mpi_errno != MPI_SUCCESS)
                goto fn_fail;
            MPIR_ERRTEST_COUNT(count, mpi_errno);
            MPIR_ERRTEST_DATATYPE(datatype, "datatype", mpi_errno);
            MPIR_ERRTEST_OP(op, mpi_errno);

            if (HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
                MPIR_Datatype_get_ptr(datatype, datatype_ptr);
                MPIR_Datatype_valid_ptr(datatype_ptr, mpi_errno);
                if (mpi_errno != MPI_SUCCESS)
                    goto fn_fail;
                MPIR_Datatype_committed_ptr(datatype_ptr, mpi_errno);
                if (mpi_errno != MPI_SUCCESS)
                    goto fn_fail;
            }

            if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTERCOMM) {
                MPIR_ERRTEST_SENDBUF_INPLACE(sendbuf, count, mpi_errno);
            } else {
                if (count != 0 && sendbuf != MPI_IN_PLACE)
                    MPIR_ERRTEST_ALIAS_COLL(sendbuf, recvbuf, mpi_errno);
            }

            if (sendbuf != MPI_IN_PLACE)
                MPIR_ERRTEST_USERBUFFER(sendbuf, count, datatype, mpi_errno);

            MPIR_ERRTEST_RECVBUF_INPLACE(recvbuf, count, mpi_errno);
            MPIR_ERRTEST_USERBUFFER(recvbuf, count, datatype, mpi_errno);

            if (HANDLE_GET_KIND(op) != HANDLE_KIND_BUILTIN) {
                MPIR_Op_get_ptr(op, op_ptr);
                MPIR_Op_valid_ptr(op_ptr, mpi_errno);
            }
            if (HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN) {
                mpi_errno = (*MPIR_OP_HDL_TO_DTYPE_FN(op)) (datatype);
            }
            if (mpi_errno != MPI_SUCCESS)
                goto fn_fail;
        }
        MPID_END_ERROR_CHECKS;
    }
#endif /* HAVE_ERROR_CHECKING */

    /* ... body of routine ...  */

    mpi_errno =
        MPID_Allreduce_init(sendbuf, recvbuf, count, datatype, op, comm_ptr, info_ptr,
                            &request_ptr);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    /* return the handle of the request to the user */
    MPIR_OBJ_PUBLISH_HANDLE(*request, request_ptr->handle);

    /* ... end of body of routine ... */

  fn_exit:
    MPIR_FUNC_TERSE_PT2PT_EXIT(MPID_STATE_MPIX_ALLREDUCE_INIT);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    return mpi_errno;

  fn_fail:
    /* --BEGIN ERROR HANDLING-- */
#ifdef HAVE_ERROR_CHECKING
    {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__, MPI_ERR_OTHER,
                                 "**mpix_allreduce_init",
                                 "**mpix_allreduce_init %p %p %d %D %O %C %I %p", sendbuf, recvbuf,
                                 count, datatype, op, comm, info, request);
    }
#endif
    mpi_errno = MPIR_Err_return_comm(comm_ptr, __func__, mpi_errno);
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}
