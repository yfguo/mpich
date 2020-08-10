/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDIG_AM_RMA_H_INCLUDED
#define MPIDIG_AM_RMA_H_INCLUDED

#include "ch4_impl.h"

extern MPIR_T_pvar_timer_t PVAR_TIMER_rma_amhdr_set ATTRIBUTE((unused));

/* Create a completed RMA request. Used when a request-based operation (e.g. RPUT)
 * completes immediately (=without actually issuing active messages) */
#define MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq_)                        \
    do {                                                                \
        /* create a completed request for user if issuing is completed immediately. */ \
        (sreq_) = MPIR_Request_create_complete(MPIR_REQUEST_KIND__RMA); \
        MPIR_ERR_CHKANDSTMT((sreq_) == NULL, mpi_errno, MPIX_ERR_NOREQ, \
                            goto fn_fail, "**nomemreq");                \
    } while (0)

static inline int MPIDIG_do_put_new(const void *origin_addr, int origin_count,
                                    MPI_Datatype origin_datatype, int target_rank,
                                    MPI_Aint target_disp, int target_count,
                                    MPI_Datatype target_datatype, MPIDI_av_entry_t * av,
                                    MPIR_Win * win, MPIR_Request ** sreq_ptr,
                                    const void *flattened_dt, size_t flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIR_Request *sreq = NULL;
    MPIDIG_put_msg_t am_hdr;
    uint64_t offset;
    size_t data_sz;
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_DO_PUT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_DO_PUT);

    av = MPIDIU_comm_rank_to_av(win->comm_ptr, target_rank);

#ifndef MPIDI_CH4_DIRECT_NETMOD
    is_local = MPIDI_av_is_local(av);
#endif

    MPIDIG_RMA_OP_CHECK_SYNC(target_rank, win);

    MPIDI_Datatype_check_size(origin_datatype, origin_count, data_sz);
    if (data_sz == 0)
        goto immed_cmpl;

    if (target_rank == win->comm_ptr->rank) {
        offset = win->disp_unit * target_disp;
        mpi_errno = MPIR_Localcopy(origin_addr,
                                   origin_count,
                                   origin_datatype,
                                   (char *) win->base + offset, target_count, target_datatype);
        MPIR_ERR_CHECK(mpi_errno);
        goto immed_cmpl;
    }

    /* Only create request when issuing is not completed.
     * We initialize two ref_count for progress engine and request-based OP,
     * then put needs to free the second ref_count.*/
    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RMA, 2);
    MPIR_ERR_CHKANDSTMT(sreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    MPIDIG_REQUEST(sreq, req->preq.win_ptr) = win;
    MPIDIG_REQUEST(sreq, req->preq.target_datatype) = target_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(target_datatype);

    MPIR_T_PVAR_TIMER_START(RMA, rma_amhdr_set);
    am_hdr.src_rank = win->comm_ptr->rank;
    am_hdr.target_disp = target_disp;

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;   /* ignored on target when flattened_dt is sent */

    am_hdr.preq_ptr = sreq;
    am_hdr.win_id = MPIDIG_WIN(win, win_id);

    /* Increase local and remote completion counters and set the local completion
     * counter in request, thus it can be decreased at request completion. */
    MPIDIG_win_cmpl_cnts_incr(win, target_rank, &sreq->completion_notification);
    MPIDIG_REQUEST(sreq, rank) = target_rank;

    am_hdr.flattened_sz = flattened_sz;
    MPIR_Datatype_get_true_lb(target_datatype, &am_hdr.target_true_lb);
    MPIR_T_PVAR_TIMER_END(RMA, rma_amhdr_set);

    if (flattened_sz) {
        void *tmp_am_hdr = MPL_malloc(sizeof(am_hdr) + flattened_sz, MPL_MEM_OTHER);
        memcpy(tmp_am_hdr, &am_hdr, sizeof(am_hdr));
        memcpy(tmp_am_hdr + sizeof(am_hdr), flattened_dt, flattened_sz);
        mpi_errno = MPIDIG_isend_impl_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                          MPIDIG_PUT_REQ, tmp_am_hdr, sizeof(am_hdr) + flattened_sz,
                                          protocol);
        MPL_free(tmp_am_hdr);
    } else {
        mpi_errno = MPIDIG_isend_impl_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                          MPIDIG_PUT_REQ, &am_hdr, sizeof(am_hdr), protocol);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (sreq_ptr)
        *sreq_ptr = sreq;
    else if (sreq != NULL)
        MPIR_Request_free_unsafe(sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_DO_PUT);
    return mpi_errno;

  immed_cmpl:
    if (sreq_ptr)
        MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq);
    goto fn_exit;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_put_dt(const void *origin_addr, int origin_count,
                                   MPI_Datatype origin_datatype, int target_rank,
                                   MPI_Aint target_disp, int target_count,
                                   MPI_Datatype target_datatype, MPIDI_av_entry_t * av,
                                   MPIR_Win * win, MPIR_Request ** sreq_ptr,
                                   const void *flattened_dt, size_t flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIR_Request *sreq = NULL;
    MPIDIG_put_msg_t am_hdr;
    uint64_t offset;
    size_t data_sz;
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_DO_PUT_DT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_DO_PUT_DT);

    av = MPIDIU_comm_rank_to_av(win->comm_ptr, target_rank);

#ifndef MPIDI_CH4_DIRECT_NETMOD
    is_local = MPIDI_av_is_local(av);
#endif

    MPIDIG_RMA_OP_CHECK_SYNC(target_rank, win);

    MPIDI_Datatype_check_size(origin_datatype, origin_count, data_sz);
    if (data_sz == 0)
        goto immed_cmpl;

    if (target_rank == win->comm_ptr->rank) {
        offset = win->disp_unit * target_disp;
        mpi_errno = MPIR_Localcopy(origin_addr,
                                   origin_count,
                                   origin_datatype,
                                   (char *) win->base + offset, target_count, target_datatype);
        MPIR_ERR_CHECK(mpi_errno);
        goto immed_cmpl;
    }

    /* Only create request when issuing is not completed.
     * We initialize two ref_count for progress engine and request-based OP,
     * then put needs to free the second ref_count.*/
    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RMA, 2);
    MPIR_ERR_CHKANDSTMT(sreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    MPIDIG_REQUEST(sreq, req->preq.win_ptr) = win;
    MPIDIG_REQUEST(sreq, req->preq.target_datatype) = target_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(target_datatype);

    MPIR_T_PVAR_TIMER_START(RMA, rma_amhdr_set);
    am_hdr.src_rank = win->comm_ptr->rank;
    am_hdr.target_disp = target_disp;

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;   /* ignored on target when flattened_dt is sent */

    am_hdr.preq_ptr = sreq;
    am_hdr.win_id = MPIDIG_WIN(win, win_id);

    /* Increase local and remote completion counters and set the local completion
     * counter in request, thus it can be decreased at request completion. */
    MPIDIG_win_cmpl_cnts_incr(win, target_rank, &sreq->completion_notification);
    MPIDIG_REQUEST(sreq, rank) = target_rank;

    am_hdr.flattened_sz = flattened_sz;
    MPIR_T_PVAR_TIMER_END(RMA, rma_amhdr_set);

    MPIDIG_REQUEST(sreq, req->preq.origin_addr) = (void *) origin_addr;
    MPIDIG_REQUEST(sreq, req->preq.origin_count) = origin_count;
    MPIDIG_REQUEST(sreq, req->preq.origin_datatype) = origin_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(origin_datatype);

    mpi_errno = MPIDIG_isend_impl_new(flattened_dt, flattened_sz, MPI_BYTE, target_rank,
                                      0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                      MPIDIG_PUT_DT_REQ, &am_hdr, sizeof(am_hdr), protocol);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (sreq_ptr)
        *sreq_ptr = sreq;
    else if (sreq != NULL)
        MPIR_Request_free_unsafe(sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_DO_PUT_DT);
    return mpi_errno;

  immed_cmpl:
    if (sreq_ptr)
        MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq);
    goto fn_exit;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_get_new(void *origin_addr, int origin_count,
                                    MPI_Datatype origin_datatype, int target_rank,
                                    MPI_Aint target_disp, int target_count,
                                    MPI_Datatype target_datatype, MPIDI_av_entry_t * av,
                                    MPIR_Win * win, MPIR_Request ** sreq_ptr, void *flattened_dt,
                                    size_t flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    size_t offset;
    MPIR_Request *sreq = NULL;
    MPIDIG_get_msg_t am_hdr;
    size_t data_sz;
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_DO_GET);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_DO_GET);

    av = MPIDIU_comm_rank_to_av(win->comm_ptr, target_rank);

#ifndef MPIDI_CH4_DIRECT_NETMOD
    is_local = MPIDI_rank_is_local(target_rank, win->comm_ptr);
#endif

    MPIDIG_RMA_OP_CHECK_SYNC(target_rank, win);

    MPIDI_Datatype_check_size(origin_datatype, origin_count, data_sz);
    if (data_sz == 0)
        goto immed_cmpl;

    if (target_rank == win->comm_ptr->rank) {
        offset = win->disp_unit * target_disp;
        mpi_errno = MPIR_Localcopy((char *) win->base + offset,
                                   target_count,
                                   target_datatype, origin_addr, origin_count, origin_datatype);
        MPIR_ERR_CHECK(mpi_errno);
        goto immed_cmpl;
    }

    /* Only create request when issuing is not completed.
     * We initialize two ref_count for progress engine and request-based OP,
     * then get needs to free the second ref_count.*/
    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RMA, 2);
    MPIR_ERR_CHKANDSTMT(sreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");

    MPIDIG_REQUEST(sreq, req->greq.win_ptr) = win;
    MPIDIG_REQUEST(sreq, req->greq.addr) = origin_addr;
    MPIDIG_REQUEST(sreq, req->greq.count) = origin_count;
    MPIDIG_REQUEST(sreq, req->greq.datatype) = origin_datatype;
    MPIDIG_REQUEST(sreq, req->greq.target_datatype) = target_datatype;
    MPIDIG_REQUEST(sreq, rank) = target_rank;
    MPIR_Datatype_add_ref_if_not_builtin(origin_datatype);
    MPIR_Datatype_add_ref_if_not_builtin(target_datatype);

    MPIR_T_PVAR_TIMER_START(RMA, rma_amhdr_set);
    am_hdr.target_disp = target_disp;

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;

    am_hdr.greq_ptr = sreq;
    am_hdr.win_id = MPIDIG_WIN(win, win_id);
    am_hdr.src_rank = win->comm_ptr->rank;

    /* Increase local and remote completion counters and set the local completion
     * counter in request, thus it can be decreased at request completion. */
    MPIDIG_win_cmpl_cnts_incr(win, target_rank, &sreq->completion_notification);

    am_hdr.flattened_sz = flattened_sz;
    MPIR_Datatype_get_true_lb(target_datatype, &am_hdr.target_true_lb);
    MPIR_T_PVAR_TIMER_END(RMA, rma_amhdr_set);

    if (flattened_sz) {
        mpi_errno = MPIDIG_isend_impl_new(flattened_dt, flattened_sz, MPI_BYTE, target_rank,
                                          0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                          MPIDIG_GET_REQ, &am_hdr, sizeof(am_hdr), protocol);
    } else {
        mpi_errno = MPIDIG_isend_impl_new(NULL, 0, MPI_DATATYPE_NULL, target_rank, 0, win->comm_ptr,
                                          0, av, &sreq, MPIR_ERR_NONE, MPIDIG_GET_REQ, &am_hdr,
                                          sizeof(am_hdr), protocol);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (sreq_ptr)
        *sreq_ptr = sreq;
    else if (sreq != NULL)
        MPIR_Request_free_unsafe(sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_DO_GET);
    return mpi_errno;

  immed_cmpl:
    if (sreq_ptr)
        MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq);
    goto fn_exit;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_accumulate_new(const void *origin_addr, int origin_count,
                                           MPI_Datatype origin_datatype, int target_rank,
                                           MPI_Aint target_disp, int target_count,
                                           MPI_Datatype target_datatype, MPI_Op op,
                                           MPIDI_av_entry_t * av, MPIR_Win * win,
                                           MPIR_Request ** sreq_ptr, const void *flattened_dt,
                                           size_t flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIR_Request *sreq = NULL;
    size_t basic_type_size;
    MPIDIG_acc_req_msg_t am_hdr;
    uint64_t data_sz, target_data_sz;
    MPIR_Datatype *dt_ptr;
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_DO_ACCUMULATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_DO_ACCUMULATE);

    av = MPIDIU_comm_rank_to_av(win->comm_ptr, target_rank);

#ifndef MPIDI_CH4_DIRECT_NETMOD
    is_local = MPIDI_av_is_local(av);
#endif

    MPIDIG_RMA_OP_CHECK_SYNC(target_rank, win);

    MPIDI_Datatype_get_size_dt_ptr(origin_count, origin_datatype, data_sz, dt_ptr);
    MPIDI_Datatype_check_size(target_datatype, target_count, target_data_sz);
    if (data_sz == 0 || target_data_sz == 0) {
        goto immed_cmpl;
    }

    /* Only create request when issuing is not completed.
     * We initialize two ref_count for progress engine and request-based OP,
     * then acc needs to free the second ref_count.*/
    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RMA, 2);
    MPIR_ERR_CHKANDSTMT(sreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    MPIDIG_REQUEST(sreq, req->areq.win_ptr) = win;
    MPIDIG_REQUEST(sreq, req->areq.target_datatype) = target_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(target_datatype);

    MPIR_T_PVAR_TIMER_START(RMA, rma_amhdr_set);
    am_hdr.req_ptr = sreq;
    am_hdr.origin_count = origin_count;

    if (HANDLE_IS_BUILTIN(origin_datatype)) {
        am_hdr.origin_datatype = origin_datatype;
    } else {
        am_hdr.origin_datatype = (dt_ptr) ? dt_ptr->basic_type : MPI_DATATYPE_NULL;
        MPIR_Datatype_get_size_macro(am_hdr.origin_datatype, basic_type_size);
        am_hdr.origin_count = (basic_type_size > 0) ? data_sz / basic_type_size : 0;
    }

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;
    am_hdr.target_disp = target_disp;
    am_hdr.op = op;
    am_hdr.win_id = MPIDIG_WIN(win, win_id);
    am_hdr.src_rank = win->comm_ptr->rank;

    /* Increase local and remote completion counters and set the local completion
     * counter in request, thus it can be decreased at request completion. */
    MPIDIG_win_cmpl_cnts_incr(win, target_rank, &sreq->completion_notification);
    /* Increase remote completion counter for acc. */
    MPIDIG_win_remote_acc_cmpl_cnt_incr(win, target_rank);

    MPIDIG_REQUEST(sreq, rank) = target_rank;
    MPIDIG_REQUEST(sreq, req->areq.data_sz) = data_sz;

    am_hdr.flattened_sz = flattened_sz;
    MPIR_T_PVAR_TIMER_END(RMA, rma_amhdr_set);

    if (flattened_sz) {
        void *tmp_am_hdr = MPL_malloc(sizeof(am_hdr) + flattened_sz, MPL_MEM_OTHER);
        memcpy(tmp_am_hdr, &am_hdr, sizeof(am_hdr));
        memcpy(tmp_am_hdr + sizeof(am_hdr), flattened_dt, flattened_sz);
        mpi_errno = MPIDIG_isend_impl_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                          MPIDIG_ACC_REQ, tmp_am_hdr, sizeof(am_hdr) + flattened_sz,
                                          protocol);
        MPL_free(tmp_am_hdr);
    } else {
        mpi_errno = MPIDIG_isend_impl_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                          MPIDIG_ACC_REQ, &am_hdr, sizeof(am_hdr), protocol);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (sreq_ptr)
        *sreq_ptr = sreq;
    else if (sreq != NULL)
        MPIR_Request_free_unsafe(sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_DO_ACCUMULATE);
    return mpi_errno;

  immed_cmpl:
    if (sreq_ptr)
        MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq);
    goto fn_exit;

  fn_fail:
    goto fn_exit;
}

static inline int MPIDIG_do_accumulate_dt(const void *origin_addr, int origin_count,
                                          MPI_Datatype origin_datatype, int target_rank,
                                          MPI_Aint target_disp, int target_count,
                                          MPI_Datatype target_datatype, MPI_Op op,
                                          MPIDI_av_entry_t * av, MPIR_Win * win,
                                          MPIR_Request ** sreq_ptr, const void *flattened_dt,
                                          size_t flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIR_Request *sreq = NULL;
    size_t basic_type_size;
    MPIDIG_acc_req_msg_t am_hdr;
    uint64_t data_sz, target_data_sz;
    MPIR_Datatype *dt_ptr;
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_DO_ACCUMULATE_DT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_DO_ACCUMULATE_DT);

    av = MPIDIU_comm_rank_to_av(win->comm_ptr, target_rank);

#ifndef MPIDI_CH4_DIRECT_NETMOD
    is_local = MPIDI_av_is_local(av);
#endif

    MPIDIG_RMA_OP_CHECK_SYNC(target_rank, win);

    MPIDI_Datatype_get_size_dt_ptr(origin_count, origin_datatype, data_sz, dt_ptr);
    MPIDI_Datatype_check_size(target_datatype, target_count, target_data_sz);
    if (data_sz == 0 || target_data_sz == 0) {
        goto immed_cmpl;
    }

    /* Only create request when issuing is not completed.
     * We initialize two ref_count for progress engine and request-based OP,
     * then acc needs to free the second ref_count.*/
    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RMA, 2);
    MPIR_ERR_CHKANDSTMT(sreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    MPIDIG_REQUEST(sreq, req->areq.win_ptr) = win;
    MPIDIG_REQUEST(sreq, req->areq.target_datatype) = target_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(target_datatype);

    MPIR_T_PVAR_TIMER_START(RMA, rma_amhdr_set);
    am_hdr.req_ptr = sreq;
    am_hdr.origin_count = origin_count;

    if (HANDLE_IS_BUILTIN(origin_datatype)) {
        am_hdr.origin_datatype = origin_datatype;
    } else {
        am_hdr.origin_datatype = (dt_ptr) ? dt_ptr->basic_type : MPI_DATATYPE_NULL;
        MPIR_Datatype_get_size_macro(am_hdr.origin_datatype, basic_type_size);
        am_hdr.origin_count = (basic_type_size > 0) ? data_sz / basic_type_size : 0;
    }

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;
    am_hdr.target_disp = target_disp;
    am_hdr.op = op;
    am_hdr.win_id = MPIDIG_WIN(win, win_id);
    am_hdr.src_rank = win->comm_ptr->rank;

    /* Increase local and remote completion counters and set the local completion
     * counter in request, thus it can be decreased at request completion. */
    MPIDIG_win_cmpl_cnts_incr(win, target_rank, &sreq->completion_notification);
    /* Increase remote completion counter for acc. */
    MPIDIG_win_remote_acc_cmpl_cnt_incr(win, target_rank);

    MPIDIG_REQUEST(sreq, rank) = target_rank;
    MPIDIG_REQUEST(sreq, req->areq.data_sz) = data_sz;

    am_hdr.flattened_sz = flattened_sz;
    MPIR_T_PVAR_TIMER_END(RMA, rma_amhdr_set);

    MPIDIG_REQUEST(sreq, req->areq.origin_addr) = (void *) origin_addr;
    MPIDIG_REQUEST(sreq, req->areq.origin_count) = origin_count;
    MPIDIG_REQUEST(sreq, req->areq.origin_datatype) = origin_datatype;
    MPIR_Datatype_add_ref_if_not_builtin(origin_datatype);

    mpi_errno = MPIDIG_isend_impl_new(flattened_dt, flattened_sz, MPI_BYTE, target_rank,
                                      0, win->comm_ptr, 0, av, &sreq, MPIR_ERR_NONE,
                                      MPIDIG_ACC_DT_REQ, &am_hdr, sizeof(am_hdr), protocol);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    if (sreq_ptr)
        *sreq_ptr = sreq;
    else if (sreq != NULL)
        MPIR_Request_free_unsafe(sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_DO_ACCUMULATE_DT);
    return mpi_errno;

  immed_cmpl:
    if (sreq_ptr)
        MPIDI_RMA_REQUEST_CREATE_COMPLETE(sreq);
    goto fn_exit;

  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_put_new(const void *origin_addr, int origin_count,
                                                MPI_Datatype origin_datatype, int target_rank,
                                                MPI_Aint target_disp, int target_count,
                                                MPI_Datatype target_datatype, MPIDI_av_entry_t * av,
                                                MPIR_Win * win, int flattened_sz, int is_long_dt,
                                                int protocol)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_PUT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_PUT);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_put_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, NULL,
                                      NULL, 0, protocol);
    } else {
        void *flattened_dt = NULL;
        int size = 0;
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);

        if (!is_long_dt) {
            mpi_errno = MPIDIG_do_put_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          target_disp, target_count, target_datatype, av, win, NULL,
                                          flattened_dt, flattened_sz, protocol);
        } else {
            mpi_errno = MPIDIG_do_put_dt(origin_addr, origin_count, origin_datatype, target_rank,
                                         target_disp, target_count, target_datatype, av, win, NULL,
                                         flattened_dt, flattened_sz, protocol);
        }
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_rput_new(const void *origin_addr, int origin_count,
                                                 MPI_Datatype origin_datatype, int target_rank,
                                                 MPI_Aint target_disp, int target_count,
                                                 MPI_Datatype target_datatype,
                                                 MPIDI_av_entry_t * av, MPIR_Win * win,
                                                 MPIR_Request ** request, int flattened_sz,
                                                 int is_long_dt, int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_RPUT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_RPUT);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_put_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, &sreq,
                                      NULL, 0, protocol);
    } else {
        void *flattened_dt = NULL;
        int size = 0;
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);

        if (!is_long_dt) {
            mpi_errno = MPIDIG_do_put_new(origin_addr, origin_count, origin_datatype, target_rank,
                                          target_disp, target_count, target_datatype, av, win,
                                          &sreq, flattened_dt, flattened_sz, protocol);
        } else {
            mpi_errno = MPIDIG_do_put_dt(origin_addr, origin_count, origin_datatype, target_rank,
                                         target_disp, target_count, target_datatype, av, win, &sreq,
                                         flattened_dt, flattened_sz, protocol);
        }
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    *request = sreq;
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_RPUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_get_new(void *origin_addr, int origin_count,
                                                MPI_Datatype origin_datatype, int target_rank,
                                                MPI_Aint target_disp, int target_count,
                                                MPI_Datatype target_datatype, MPIDI_av_entry_t * av,
                                                MPIR_Win * win, int flattened_sz, int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    void *flattened_dt = NULL;
    int size = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_GET);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_GET);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_get_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, NULL,
                                      NULL, 0, protocol);
    } else {
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);
        mpi_errno = MPIDIG_do_get_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, NULL,
                                      flattened_dt, flattened_sz, protocol);
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_GET);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_rget_new(void *origin_addr, int origin_count,
                                                 MPI_Datatype origin_datatype, int target_rank,
                                                 MPI_Aint target_disp, int target_count,
                                                 MPI_Datatype target_datatype,
                                                 MPIDI_av_entry_t * av, MPIR_Win * win,
                                                 MPIR_Request ** request, int flattened_sz,
                                                 int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    void *flattened_dt = NULL;
    int size = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_RGET);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_RGET);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_get_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, &sreq,
                                      NULL, 0, protocol);
    } else {
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);
        mpi_errno = MPIDIG_do_get_new(origin_addr, origin_count, origin_datatype, target_rank,
                                      target_disp, target_count, target_datatype, av, win, &sreq,
                                      flattened_dt, flattened_sz, protocol);

    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    *request = sreq;
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_RGET);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_raccumulate_new(const void *origin_addr, int origin_count,
                                                        MPI_Datatype origin_datatype,
                                                        int target_rank, MPI_Aint target_disp,
                                                        int target_count,
                                                        MPI_Datatype target_datatype, MPI_Op op,
                                                        MPIDI_av_entry_t * av, MPIR_Win * win,
                                                        MPIR_Request ** request, int flattened_sz,
                                                        int is_long_dt, int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_RACCUMULATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_RACCUMULATE);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_accumulate_new(origin_addr, origin_count, origin_datatype,
                                             target_rank, target_disp, target_count,
                                             target_datatype, op, av, win, &sreq, NULL, 0,
                                             protocol);
    } else {
        void *flattened_dt = NULL;
        int size = 0;
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);

        if (!is_long_dt) {
            mpi_errno = MPIDIG_do_accumulate_new(origin_addr, origin_count, origin_datatype,
                                                 target_rank, target_disp, target_count,
                                                 target_datatype, op, av, win, &sreq, flattened_dt,
                                                 flattened_sz, protocol);
        } else {
            mpi_errno = MPIDIG_do_accumulate_dt(origin_addr, origin_count, origin_datatype,
                                                target_rank, target_disp, target_count,
                                                target_datatype, op, av, win, &sreq, flattened_dt,
                                                flattened_sz, protocol);
        }
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    *request = sreq;
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_RACCUMULATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDIG_mpi_accumulate_new(const void *origin_addr, int origin_count,
                                                       MPI_Datatype origin_datatype,
                                                       int target_rank, MPI_Aint target_disp,
                                                       int target_count,
                                                       MPI_Datatype target_datatype, MPI_Op op,
                                                       MPIDI_av_entry_t * av, MPIR_Win * win,
                                                       int flattened_sz, int is_long_dt,
                                                       int protocol)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDIG_MPI_ACCUMULATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDIG_MPI_ACCUMULATE);

    if (!flattened_sz) {
        mpi_errno = MPIDIG_do_accumulate_new(origin_addr, origin_count, origin_datatype,
                                             target_rank, target_disp, target_count,
                                             target_datatype, op, av, win, NULL, NULL, 0, protocol);
    } else {
        void *flattened_dt = NULL;
        int size = 0;
        MPIR_Datatype_get_flattened(target_datatype, &flattened_dt, &size);

        if (!is_long_dt) {
            mpi_errno = MPIDIG_do_accumulate_new(origin_addr, origin_count, origin_datatype,
                                                 target_rank, target_disp, target_count,
                                                 target_datatype, op, av, win, NULL, flattened_dt,
                                                 flattened_sz, protocol);
        } else {
            mpi_errno = MPIDIG_do_accumulate_dt(origin_addr, origin_count, origin_datatype,
                                                target_rank, target_disp, target_count,
                                                target_datatype, op, av, win, NULL, flattened_dt,
                                                flattened_sz, protocol);
        }
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDIG_MPI_ACCUMULATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDIG_AM_RMA_H_INCLUDED */
