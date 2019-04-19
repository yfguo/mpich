/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2019 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2019 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include "mpiimpl.h"
#include "tsp_gentran.h"
#include "gentran_utils.h"
#include "../iallgatherv/iallgatherv_tsp_ring_algos_prototypes.h"
#include "../iallgatherv/iallgatherv_tsp_recexch_algos_prototypes.h"
#include "../iallgatherv/iallgatherv_tsp_brucks_algos_prototypes.h"

static int MPIR_Allgatherv_sched_intra_auto(const void *sendbuf, int sendcount,
                                            MPI_Datatype sendtype, void *recvbuf,
                                            const int *recvcounts, const int *displs,
                                            MPI_Datatype recvtype, MPIR_Comm * comm_ptr,
                                            MPIR_TSP_sched_t * sched)
{
    int mpi_errno = MPI_SUCCESS;
    mpi_errno =
        MPIR_TSP_Iallgatherv_sched_intra_ring(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
                                              displs, recvtype, comm_ptr, sched);

    return mpi_errno;
}

/* -- Begin Profiling Symbol Block for routine MPIX_Allgatherv_init */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPIX_Allgatherv_init = PMPIX_Allgatherv_init
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPIX_Allgatherv_init  MPIX_Allgatherv_init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPIX_Allgatherv_init as PMPIX_Allgatherv_init
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPIX_Allgatherv_init(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
                         const int *recvcounts, const int *displs, MPI_Datatype recvtype,
                         MPI_Comm comm, MPI_Info info, MPI_Request * request)
    __attribute__ ((weak, alias("PMPIX_Allgatherv_init")));
#endif
/* -- End Profiling Symbol Block */

/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPIX_Allgatherv_init
#define MPIX_Allgatherv_init PMPIX_Allgatherv_init

int MPIR_Allgatherv_init(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,
                         const int *recvcounts, const int *displs, MPI_Datatype recvtype,
                         MPIR_Comm * comm_ptr, MPIR_Info * info_ptr, MPIR_Request ** request,
                         MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS, mpi_errno_ret = MPI_SUCCESS;
    MPIR_TSP_sched_t *sched;

    /* create a new request */
    MPIR_Request *req = MPIR_Request_create(MPIR_REQUEST_KIND__PREQUEST_COLL);
    if (!req)
        MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**nomem");

    req->u.persist.real_request = NULL;

    /* generate the schedule */
    sched = MPL_malloc(sizeof(MPIR_TSP_sched_t), MPL_MEM_COLL);
    MPIR_ERR_CHKANDJUMP(!sched, mpi_errno, MPI_ERR_OTHER, "**nomem");
    MPIR_TSP_sched_create(sched, true);

    switch (MPIR_CVAR_IALLGATHERV_INTRA_ALGORITHM) {
        case MPIR_CVAR_IALLGATHERV_INTRA_ALGORITHM_gentran_recexch_halving:
            mpi_errno =
                MPIR_TSP_Iallgatherv_sched_intra_recexch(sendbuf, sendcount, sendtype, recvbuf,
                                                         recvcounts, displs, recvtype, comm_ptr,
                                                         MPIR_IALLGATHERV_RECEXCH_TYPE_DISTANCE_HALVING,
                                                         MPIR_CVAR_IALLGATHERV_RECEXCH_KVAL, sched);
            break;
        case MPIR_CVAR_IALLGATHERV_INTRA_ALGORITHM_gentran_recexch_doubling:
            mpi_errno =
                MPIR_TSP_Iallgatherv_sched_intra_recexch(sendbuf, sendcount, sendtype, recvbuf,
                                                         recvcounts, displs, recvtype, comm_ptr,
                                                         MPIR_IALLGATHERV_RECEXCH_TYPE_DISTANCE_DOUBLING,
                                                         MPIR_CVAR_IALLGATHERV_RECEXCH_KVAL, sched);
            break;
        case MPIR_CVAR_IALLGATHERV_INTRA_ALGORITHM_gentran_ring:
            mpi_errno =
                MPIR_TSP_Iallgatherv_sched_intra_ring(sendbuf, sendcount, sendtype, recvbuf,
                                                      recvcounts, displs, recvtype, comm_ptr,
                                                      sched);
            break;
        case MPIR_CVAR_IALLGATHERV_INTRA_ALGORITHM_gentran_brucks:
            mpi_errno =
                MPIR_TSP_Iallgatherv_sched_intra_brucks(sendbuf, sendcount, sendtype, recvbuf,
                                                        recvcounts, displs, recvtype, comm_ptr,
                                                        sched, MPIR_CVAR_IALLGATHERV_BRUCKS_KVAL);
            break;
        default:
            mpi_errno =
                MPIR_Allgatherv_sched_intra_auto(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
                                                 displs, recvtype, comm_ptr, sched);
            break;
    }
    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        *errflag = MPIR_ERR_OTHER;
        MPIR_ERR_SET(mpi_errno, *errflag, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    req->u.persist.sched = sched;

    *request = req;

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif

/*@
 * MPIX_Allgatherv_init - Creates a nonblocking, persistent collective communication request for the Allgatherv operation.
 *
 * Input Parameters:
 * + sendbuf - starting address of send buffer (choice)
 * . sendcount - number of elements to send to each process (integer)
 * . sendtype - data type of send buffer elements (handle)
 * . recvcounts - number of elements received from any process (integer)
 * . displs - number of elements received from any process (integer)
 * . recvtype - data type of receive buffer elements (handle)
 * . info - info argument (handle)
 * . request - request argument (handle)
 * - comm - communicator (handle)
 *
 *   Output Parameters:
 *   . recvbuf - address of receive buffer (choice)
 *
 *   .N ThreadSafe
 *
 *   .N Fortran
 *
 *   .N Errors
 *   .N MPI_ERR_COMM
 *   .N MPI_ERR_COUNT
 *   .N MPI_ERR_TYPE
 *   .N MPI_ERR_BUFFER
 *
 *   .seealso: MPI_Start, MPI_Startall, MPI_Request_free
 *   @*/
int MPIX_Allgatherv_init(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                         void *recvbuf, const int *recvcounts, const int *displs,
                         MPI_Datatype recvtype, MPI_Comm comm, MPI_Info info, MPI_Request * request)
{
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS, mpi_errno_ret = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL;
    MPIR_Info *info_ptr = NULL;
    MPIR_Request *request_ptr = NULL;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIX_ALLGATHERV_INIT);

    MPIR_ERRTEST_INITIALIZED_ORDIE();

    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPIR_FUNC_TERSE_PT2PT_ENTER(MPID_STATE_MPIX_ALLGATHERV_INIT);

    /* Validate parameters, especially handles needing to be converted */
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
            MPIR_Datatype *sendtype_ptr = NULL, *recvtype_ptr = NULL;
            int i, comm_size;

            MPIR_Comm_valid_ptr(comm_ptr, mpi_errno, FALSE);
            if (mpi_errno) {
                /* for communication errors, just record the error but continue */
                errflag = MPIR_ERR_OTHER;
                MPIR_ERR_SET(mpi_errno, errflag, "**fail");
                MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
            }

            if (sendbuf != MPI_IN_PLACE) {
                MPIR_ERRTEST_COUNT(sendcount, mpi_errno);
                MPIR_ERRTEST_DATATYPE(sendtype, "sendtype", mpi_errno);
                if (HANDLE_GET_KIND(sendtype) != HANDLE_KIND_BUILTIN) {
                    MPIR_Datatype_get_ptr(sendtype, sendtype_ptr);
                    MPIR_Datatype_valid_ptr(sendtype_ptr, mpi_errno);
                    if (mpi_errno) {
                        /* for communication errors, just record the error but continue */
                        errflag = MPIR_ERR_OTHER;
                        MPIR_ERR_SET(mpi_errno, errflag, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                    MPIR_Datatype_committed_ptr(sendtype_ptr, mpi_errno);
                    if (mpi_errno) {
                        /* for communication errors, just record the error but continue */
                        errflag = MPIR_ERR_OTHER;
                        MPIR_ERR_SET(mpi_errno, errflag, "**fail");
                        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                    }
                }
                /* Checks that sendbuf isn't the same as recvbuf. */
                if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM &&
                    sendtype == recvtype && recvcounts[comm_ptr->rank] != 0 && sendcount != 0) {
                    int recvtype_size;
                    MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
                    MPIR_ERRTEST_ALIAS_COLL(sendbuf,
                                            (char *) recvbuf +
                                            displs[comm_ptr->rank] * recvtype_size, mpi_errno);
                }

            }

            if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM)
                comm_size = comm_ptr->local_size;
            else
                comm_size = comm_ptr->remote_size;

            for (i = 0; i < comm_size; i++) {
                MPIR_ERRTEST_COUNT(recvcounts[i], mpi_errno);
                MPIR_ERRTEST_DATATYPE(recvtype, "recvtype", mpi_errno);
            }

            if (HANDLE_GET_KIND(recvtype) != HANDLE_KIND_BUILTIN) {
                MPIR_Datatype_get_ptr(recvtype, recvtype_ptr);
                MPIR_Datatype_valid_ptr(recvtype_ptr, mpi_errno);
                if (mpi_errno) {
                    /* for communication errors, just record the error but continue */
                    errflag = MPIR_ERR_OTHER;
                    MPIR_ERR_SET(mpi_errno, errflag, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
                MPIR_Datatype_committed_ptr(recvtype_ptr, mpi_errno);
                if (mpi_errno) {
                    /* for communication errors, just record the error but continue */
                    errflag = MPIR_ERR_OTHER;
                    MPIR_ERR_SET(mpi_errno, errflag, "**fail");
                    MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
                }
            }

            if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTERCOMM) {
                MPIR_ERRTEST_SENDBUF_INPLACE(sendbuf, sendcount, mpi_errno);
            }
            MPIR_ERRTEST_USERBUFFER(sendbuf, sendcount, sendtype, mpi_errno);
            for (i = 0; i < comm_size; i++) {
                if (recvcounts[i] > 0) {
                    MPIR_ERRTEST_RECVBUF_INPLACE(recvbuf, recvcounts[i], mpi_errno);
                    MPIR_ERRTEST_USERBUFFER(recvbuf, recvcounts[i], recvtype, mpi_errno);
                    break;
                }
            }
        }
        MPID_END_ERROR_CHECKS;
    }
#endif /* HAVE_ERROR_CHECKING */

    /* ... body of routine ...  */

    mpi_errno =
        MPID_Allgatherv_init(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype,
                             comm_ptr, info_ptr, &request_ptr, &errflag);
    if (mpi_errno) {
        /* for communication errors, just record the error but continue */
        errflag = MPIR_ERR_OTHER;
        MPIR_ERR_SET(mpi_errno, errflag, "**fail");
        MPIR_ERR_ADD(mpi_errno_ret, mpi_errno);
    }

    /* return the handle of the request to the user */
    MPIR_OBJ_PUBLISH_HANDLE(*request, request_ptr->handle);

    /* ... end of body of routine ... */

  fn_exit:
    MPIR_FUNC_TERSE_PT2PT_EXIT(MPID_STATE_MPIX_ALLGATHERV_INIT);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    return mpi_errno;
  fn_fail:
    /* --BEGIN ERROR HANDLING-- */
#ifdef HAVE_ERROR_CHECKING
    {
        mpi_errno =
            MPIR_Err_create_code(mpi_errno, MPIR_ERR_RECOVERABLE, __func__, __LINE__, MPI_ERR_OTHER,
                                 "**mpix_allgatherv_init",
                                 "**mpix_allgatherv_init %p %d %D %p %p %p %D %C %I %p", sendbuf,
                                 sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm,
                                 info, request);
    }
#endif
    mpi_errno = MPIR_Err_return_comm(comm_ptr, __func__, mpi_errno);
    goto fn_exit;
}
