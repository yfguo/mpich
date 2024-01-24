/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef OFL_H_INCLUDED
#define OFL_H_INCLUDED

#include <mpidimpl.h>

int MPIDI_OFL_init_local(int *tag_bits);
int MPIDI_OFL_init_world(void);
int MPIDI_OFL_mpi_finalize_hook(void);
int MPIDI_OFL_post_init(void);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_progress(int vci,
                                                int *made_progress) MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_comm_set_hints(MPIR_Comm * comm_ptr, MPIR_Info * info);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_am_send_hdr(int rank, MPIR_Comm * comm, int handler_id,
                                                   const void *am_hdr, MPI_Aint am_hdr_sz,
                                                   int src_vci,
                                                   int dst_vci) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_am_isend(int rank, MPIR_Comm * comm, int handler_id,
                                                const void *am_hdr, MPI_Aint am_hdr_sz,
                                                const void *data, MPI_Aint count,
                                                MPI_Datatype datatype, int src_vci, int dst_vci,
                                                MPIR_Request * sreq) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_am_send_hdr_reply(MPIR_Comm * comm, int src_rank,
                                                         int handler_id, const void *am_hdr,
                                                         MPI_Aint am_hdr_sz, int src_vci,
                                                         int dst_vci) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_am_isend_reply(MPIR_Comm * comm, int src_rank,
                                                      int handler_id, const void *am_hdr,
                                                      MPI_Aint am_hdr_sz, const void *data,
                                                      MPI_Aint count, MPI_Datatype datatype,
                                                      int src_vci, int dst_vci,
                                                      MPIR_Request * sreq) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX MPIDIG_recv_data_copy_cb MPIDI_OFL_am_get_data_copy_cb(uint32_t attr)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX MPI_Aint MPIDI_OFL_am_hdr_max_sz(void) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX MPI_Aint MPIDI_OFL_am_eager_limit(void) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX MPI_Aint MPIDI_OFL_am_eager_buf_limit(void) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX bool MPIDI_OFL_am_check_eager(MPI_Aint am_hdr_sz, MPI_Aint data_sz,
                                                       const void *data, MPI_Aint count,
                                                       MPI_Datatype datatype,
                                                       MPIR_Request * sreq)
    MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_comm_commit_pre_hook(MPIR_Comm * comm);
int MPIDI_OFL_mpi_comm_commit_post_hook(MPIR_Comm * comm);
int MPIDI_OFL_mpi_comm_free_hook(MPIR_Comm * comm);
int MPIDI_OFL_mpi_win_create_hook(MPIR_Win * win);
int MPIDI_OFL_mpi_win_allocate_hook(MPIR_Win * win);
int MPIDI_OFL_mpi_win_allocate_shared_hook(MPIR_Win * win);
int MPIDI_OFL_mpi_win_create_dynamic_hook(MPIR_Win * win);
int MPIDI_OFL_mpi_win_attach_hook(MPIR_Win * win, void *base, MPI_Aint size);
int MPIDI_OFL_mpi_win_detach_hook(MPIR_Win * win, const void *base);
int MPIDI_OFL_mpi_win_free_hook(MPIR_Win * win);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_win_cmpl_hook(MPIR_Win * win) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_win_local_cmpl_hook(MPIR_Win * win)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_target_cmpl_hook(int rank,
                                                            MPIR_Win * win)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_target_local_cmpl_hook(int rank,
                                                                  MPIR_Win * win)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX void MPIDI_OFL_am_request_init(MPIR_Request * req)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX void MPIDI_OFL_am_request_finalize(MPIR_Request * req)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_isend(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank, int tag,
                                                 MPIR_Comm * comm, int attr,
                                                 MPIDI_av_entry_t * addr,
                                                 MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_cancel_send(MPIR_Request * sreq)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_irecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                 int rank, int tag, MPIR_Comm * comm, int attr,
                                                 MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                  MPIR_Request * message) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_cancel_recv(MPIR_Request * rreq)
    MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_psend_init(const void *buf, int partitions, MPI_Aint count, MPI_Datatype datatype,
                             int rank, int tag, MPIR_Comm * comm, MPIR_Info * info,
                             MPIDI_av_entry_t * av, MPIR_Request ** req_p);
int MPIDI_OFL_mpi_precv_init(void *buf, int partitions, MPI_Aint count, MPI_Datatype datatype,
                             int rank, int tag, MPIR_Comm * comm, MPIR_Info * info,
                             MPIDI_av_entry_t * av, MPIR_Request ** req_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_part_start(MPIR_Request * req) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_pready_range(int partition_low, int partition_high,
                                                        MPIR_Request * sreq)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_pready_list(int length, const int array_of_partitions[],
                                                       MPIR_Request * sreq)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_parrived(MPIR_Request * rreq, int partition,
                                                    int *flag) MPL_STATIC_INLINE_SUFFIX;
void *MPIDI_OFL_mpi_alloc_mem(MPI_Aint size, MPIR_Info * info);
int MPIDI_OFL_mpi_free_mem(void *ptr);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_improbe(int source, int tag, MPIR_Comm * comm, int attr,
                                                   int *flag, MPIR_Request ** message_p,
                                                   MPI_Status * status) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iprobe(int source, int tag, MPIR_Comm * comm, int attr,
                                                  int *flag,
                                                  MPI_Status * status) MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_set_info(MPIR_Win * win, MPIR_Info * info);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_put(const void *origin_addr, MPI_Aint origin_count,
                                               MPI_Datatype origin_datatype, int target_rank,
                                               MPI_Aint target_disp, MPI_Aint target_count,
                                               MPI_Datatype target_datatype, MPIR_Win * win,
                                               MPIDI_winattr_t winattr) MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_get_info(MPIR_Win * win, MPIR_Info ** info_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_get(void *origin_addr, MPI_Aint origin_count,
                                               MPI_Datatype origin_datatype, int target_rank,
                                               MPI_Aint target_disp, MPI_Aint target_count,
                                               MPI_Datatype target_datatype, MPIR_Win * win,
                                               MPIDI_winattr_t winattr) MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_free(MPIR_Win ** win_p);
int MPIDI_OFL_mpi_win_create(void *base, MPI_Aint length, int disp_unit, MPIR_Info * info,
                             MPIR_Comm * comm_ptr, MPIR_Win ** win_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_accumulate(const void *origin_addr,
                                                      MPI_Aint origin_count,
                                                      MPI_Datatype origin_datatype, int target_rank,
                                                      MPI_Aint target_disp, MPI_Aint target_count,
                                                      MPI_Datatype target_datatype, MPI_Op op,
                                                      MPIR_Win * win,
                                                      MPIDI_winattr_t winattr)
    MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_attach(MPIR_Win * win, void *base, MPI_Aint size);
int MPIDI_OFL_mpi_win_allocate_shared(MPI_Aint size, int disp_unit, MPIR_Info * info,
                                      MPIR_Comm * comm_ptr, void **baseptr_p, MPIR_Win ** win_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_rput(const void *origin_addr, MPI_Aint origin_count,
                                                MPI_Datatype origin_datatype, int target_rank,
                                                MPI_Aint target_disp, MPI_Aint target_count,
                                                MPI_Datatype target_datatype, MPIR_Win * win,
                                                MPIDI_winattr_t winattr,
                                                MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_detach(MPIR_Win * win, const void *base);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_compare_and_swap(const void *origin_addr,
                                                            const void *compare_addr,
                                                            void *result_addr,
                                                            MPI_Datatype datatype, int target_rank,
                                                            MPI_Aint target_disp, MPIR_Win * win,
                                                            MPIDI_winattr_t winattr)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_raccumulate(const void *origin_addr,
                                                       MPI_Aint origin_count,
                                                       MPI_Datatype origin_datatype,
                                                       int target_rank, MPI_Aint target_disp,
                                                       MPI_Aint target_count,
                                                       MPI_Datatype target_datatype, MPI_Op op,
                                                       MPIR_Win * win, MPIDI_winattr_t winattr,
                                                       MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_rget_accumulate(const void *origin_addr,
                                                           MPI_Aint origin_count,
                                                           MPI_Datatype origin_datatype,
                                                           void *result_addr, MPI_Aint result_count,
                                                           MPI_Datatype result_datatype,
                                                           int target_rank, MPI_Aint target_disp,
                                                           MPI_Aint target_count,
                                                           MPI_Datatype target_datatype, MPI_Op op,
                                                           MPIR_Win * win, MPIDI_winattr_t winattr,
                                                           MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_fetch_and_op(const void *origin_addr, void *result_addr,
                                                        MPI_Datatype datatype, int target_rank,
                                                        MPI_Aint target_disp, MPI_Op op,
                                                        MPIR_Win * win,
                                                        MPIDI_winattr_t winattr)
    MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_win_allocate(MPI_Aint size, int disp_unit, MPIR_Info * info, MPIR_Comm * comm,
                               void *baseptr, MPIR_Win ** win_p);
int MPIDI_OFL_mpi_win_create_dynamic(MPIR_Info * info, MPIR_Comm * comm, MPIR_Win ** win_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_rget(void *origin_addr, MPI_Aint origin_count,
                                                MPI_Datatype origin_datatype, int target_rank,
                                                MPI_Aint target_disp, MPI_Aint target_count,
                                                MPI_Datatype target_datatype, MPIR_Win * win,
                                                MPIDI_winattr_t winattr,
                                                MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_get_accumulate(const void *origin_addr,
                                                          MPI_Aint origin_count,
                                                          MPI_Datatype origin_datatype,
                                                          void *result_addr, MPI_Aint result_count,
                                                          MPI_Datatype result_datatype,
                                                          int target_rank, MPI_Aint target_disp,
                                                          MPI_Aint target_count,
                                                          MPI_Datatype target_datatype, MPI_Op op,
                                                          MPIR_Win * win,
                                                          MPIDI_winattr_t winattr)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_barrier(MPIR_Comm * comm,
                                                   MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_bcast(void *buffer, MPI_Aint count,
                                                 MPI_Datatype datatype, int root, MPIR_Comm * comm,
                                                 MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_allreduce(const void *sendbuf, void *recvbuf,
                                                     MPI_Aint count, MPI_Datatype datatype,
                                                     MPI_Op op, MPIR_Comm * comm,
                                                     MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_allgather(const void *sendbuf, MPI_Aint sendcount,
                                                     MPI_Datatype sendtype, void *recvbuf,
                                                     MPI_Aint recvcount, MPI_Datatype recvtype,
                                                     MPIR_Comm * comm,
                                                     MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_allgatherv(const void *sendbuf, MPI_Aint sendcount,
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      const MPI_Aint * recvcounts,
                                                      const MPI_Aint * displs,
                                                      MPI_Datatype recvtype, MPIR_Comm * comm,
                                                      MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_scatter(const void *sendbuf, MPI_Aint sendcount,
                                                   MPI_Datatype sendtype, void *recvbuf,
                                                   MPI_Aint recvcount, MPI_Datatype recvtype,
                                                   int root, MPIR_Comm * comm,
                                                   MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_scatterv(const void *sendbuf,
                                                    const MPI_Aint * sendcounts,
                                                    const MPI_Aint * displs, MPI_Datatype sendtype,
                                                    void *recvbuf, MPI_Aint recvcount,
                                                    MPI_Datatype recvtype, int root,
                                                    MPIR_Comm * comm_ptr,
                                                    MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_gather(const void *sendbuf, MPI_Aint sendcount,
                                                  MPI_Datatype sendtype, void *recvbuf,
                                                  MPI_Aint recvcount, MPI_Datatype recvtype,
                                                  int root, MPIR_Comm * comm,
                                                  MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_gatherv(const void *sendbuf, MPI_Aint sendcount,
                                                   MPI_Datatype sendtype, void *recvbuf,
                                                   const MPI_Aint * recvcounts,
                                                   const MPI_Aint * displs, MPI_Datatype recvtype,
                                                   int root, MPIR_Comm * comm,
                                                   MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_alltoall(const void *sendbuf, MPI_Aint sendcount,
                                                    MPI_Datatype sendtype, void *recvbuf,
                                                    MPI_Aint recvcount, MPI_Datatype recvtype,
                                                    MPIR_Comm * comm,
                                                    MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_alltoallv(const void *sendbuf,
                                                     const MPI_Aint * sendcounts,
                                                     const MPI_Aint * sdispls,
                                                     MPI_Datatype sendtype, void *recvbuf,
                                                     const MPI_Aint * recvcounts,
                                                     const MPI_Aint * rdispls,
                                                     MPI_Datatype recvtype, MPIR_Comm * comm,
                                                     MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_alltoallw(const void *sendbuf,
                                                     const MPI_Aint * sendcounts,
                                                     const MPI_Aint * sdispls,
                                                     const MPI_Datatype sendtypes[], void *recvbuf,
                                                     const MPI_Aint * recvcounts,
                                                     const MPI_Aint * rdispls,
                                                     const MPI_Datatype recvtypes[],
                                                     MPIR_Comm * comm,
                                                     MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_reduce(const void *sendbuf, void *recvbuf,
                                                  MPI_Aint count, MPI_Datatype datatype, MPI_Op op,
                                                  int root, MPIR_Comm * comm_ptr,
                                                  MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_reduce_scatter(const void *sendbuf, void *recvbuf,
                                                          const MPI_Aint * recvcounts,
                                                          MPI_Datatype datatype, MPI_Op op,
                                                          MPIR_Comm * comm_ptr,
                                                          MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_reduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                                MPI_Aint recvcount,
                                                                MPI_Datatype datatype, MPI_Op op,
                                                                MPIR_Comm * comm_ptr,
                                                                MPIR_Errflag_t errflag)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_scan(const void *sendbuf, void *recvbuf, MPI_Aint count,
                                                MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                                MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_exscan(const void *sendbuf, void *recvbuf,
                                                  MPI_Aint count, MPI_Datatype datatype, MPI_Op op,
                                                  MPIR_Comm * comm,
                                                  MPIR_Errflag_t errflag) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_neighbor_allgather(const void *sendbuf,
                                                              MPI_Aint sendcount,
                                                              MPI_Datatype sendtype, void *recvbuf,
                                                              MPI_Aint recvcount,
                                                              MPI_Datatype recvtype,
                                                              MPIR_Comm * comm)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_neighbor_allgatherv(const void *sendbuf,
                                                               MPI_Aint sendcount,
                                                               MPI_Datatype sendtype, void *recvbuf,
                                                               const MPI_Aint * recvcounts,
                                                               const MPI_Aint * displs,
                                                               MPI_Datatype recvtype,
                                                               MPIR_Comm * comm)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_neighbor_alltoallv(const void *sendbuf,
                                                              const MPI_Aint * sendcounts,
                                                              const MPI_Aint * sdispls,
                                                              MPI_Datatype sendtype, void *recvbuf,
                                                              const MPI_Aint * recvcounts,
                                                              const MPI_Aint * rdispls,
                                                              MPI_Datatype recvtype,
                                                              MPIR_Comm * comm)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_neighbor_alltoallw(const void *sendbuf,
                                                              const MPI_Aint * sendcounts,
                                                              const MPI_Aint * sdispls,
                                                              const MPI_Datatype sendtypes[],
                                                              void *recvbuf,
                                                              const MPI_Aint * recvcounts,
                                                              const MPI_Aint * rdispls,
                                                              const MPI_Datatype recvtypes[],
                                                              MPIR_Comm * comm)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_neighbor_alltoall(const void *sendbuf,
                                                             MPI_Aint sendcount,
                                                             MPI_Datatype sendtype, void *recvbuf,
                                                             MPI_Aint recvcount,
                                                             MPI_Datatype recvtype,
                                                             MPIR_Comm * comm)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ineighbor_allgather(const void *sendbuf,
                                                               MPI_Aint sendcount,
                                                               MPI_Datatype sendtype, void *recvbuf,
                                                               MPI_Aint recvcount,
                                                               MPI_Datatype recvtype,
                                                               MPIR_Comm * comm,
                                                               MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ineighbor_allgatherv(const void *sendbuf,
                                                                MPI_Aint sendcount,
                                                                MPI_Datatype sendtype,
                                                                void *recvbuf,
                                                                const MPI_Aint * recvcounts,
                                                                const MPI_Aint * displs,
                                                                MPI_Datatype recvtype,
                                                                MPIR_Comm * comm,
                                                                MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ineighbor_alltoall(const void *sendbuf,
                                                              MPI_Aint sendcount,
                                                              MPI_Datatype sendtype, void *recvbuf,
                                                              MPI_Aint recvcount,
                                                              MPI_Datatype recvtype,
                                                              MPIR_Comm * comm,
                                                              MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ineighbor_alltoallv(const void *sendbuf,
                                                               const MPI_Aint * sendcounts,
                                                               const MPI_Aint * sdispls,
                                                               MPI_Datatype sendtype, void *recvbuf,
                                                               const MPI_Aint * recvcounts,
                                                               const MPI_Aint * rdispls,
                                                               MPI_Datatype recvtype,
                                                               MPIR_Comm * comm,
                                                               MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ineighbor_alltoallw(const void *sendbuf,
                                                               const MPI_Aint * sendcounts,
                                                               const MPI_Aint * sdispls,
                                                               const MPI_Datatype sendtypes[],
                                                               void *recvbuf,
                                                               const MPI_Aint * recvcounts,
                                                               const MPI_Aint * rdispls,
                                                               const MPI_Datatype recvtypes[],
                                                               MPIR_Comm * comm,
                                                               MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ibarrier(MPIR_Comm * comm,
                                                    MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ibcast(void *buffer, MPI_Aint count,
                                                  MPI_Datatype datatype, int root, MPIR_Comm * comm,
                                                  MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iallgather(const void *sendbuf, MPI_Aint sendcount,
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      MPI_Aint recvcount, MPI_Datatype recvtype,
                                                      MPIR_Comm * comm,
                                                      MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iallgatherv(const void *sendbuf, MPI_Aint sendcount,
                                                       MPI_Datatype sendtype, void *recvbuf,
                                                       const MPI_Aint * recvcounts,
                                                       const MPI_Aint * displs,
                                                       MPI_Datatype recvtype, MPIR_Comm * comm,
                                                       MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iallreduce(const void *sendbuf, void *recvbuf,
                                                      MPI_Aint count, MPI_Datatype datatype,
                                                      MPI_Op op, MPIR_Comm * comm,
                                                      MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ialltoall(const void *sendbuf, MPI_Aint sendcount,
                                                     MPI_Datatype sendtype, void *recvbuf,
                                                     MPI_Aint recvcount, MPI_Datatype recvtype,
                                                     MPIR_Comm * comm,
                                                     MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ialltoallv(const void *sendbuf,
                                                      const MPI_Aint * sendcounts,
                                                      const MPI_Aint * sdispls,
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      const MPI_Aint * recvcounts,
                                                      const MPI_Aint * rdispls,
                                                      MPI_Datatype recvtype, MPIR_Comm * comm,
                                                      MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ialltoallw(const void *sendbuf,
                                                      const MPI_Aint * sendcounts,
                                                      const MPI_Aint * sdispls,
                                                      const MPI_Datatype sendtypes[], void *recvbuf,
                                                      const MPI_Aint * recvcounts,
                                                      const MPI_Aint * rdispls,
                                                      const MPI_Datatype recvtypes[],
                                                      MPIR_Comm * comm,
                                                      MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iexscan(const void *sendbuf, void *recvbuf,
                                                   MPI_Aint count, MPI_Datatype datatype, MPI_Op op,
                                                   MPIR_Comm * comm,
                                                   MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_igather(const void *sendbuf, MPI_Aint sendcount,
                                                   MPI_Datatype sendtype, void *recvbuf,
                                                   MPI_Aint recvcount, MPI_Datatype recvtype,
                                                   int root, MPIR_Comm * comm,
                                                   MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_igatherv(const void *sendbuf, MPI_Aint sendcount,
                                                    MPI_Datatype sendtype, void *recvbuf,
                                                    const MPI_Aint * recvcounts,
                                                    const MPI_Aint * displs, MPI_Datatype recvtype,
                                                    int root, MPIR_Comm * comm,
                                                    MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ireduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                                 MPI_Aint recvcount,
                                                                 MPI_Datatype datatype, MPI_Op op,
                                                                 MPIR_Comm * comm,
                                                                 MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ireduce_scatter(const void *sendbuf, void *recvbuf,
                                                           const MPI_Aint * recvcounts,
                                                           MPI_Datatype datatype, MPI_Op op,
                                                           MPIR_Comm * comm,
                                                           MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_ireduce(const void *sendbuf, void *recvbuf,
                                                   MPI_Aint count, MPI_Datatype datatype, MPI_Op op,
                                                   int root, MPIR_Comm * comm_ptr,
                                                   MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iscan(const void *sendbuf, void *recvbuf, MPI_Aint count,
                                                 MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                                 MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iscatter(const void *sendbuf, MPI_Aint sendcount,
                                                    MPI_Datatype sendtype, void *recvbuf,
                                                    MPI_Aint recvcount, MPI_Datatype recvtype,
                                                    int root, MPIR_Comm * comm,
                                                    MPIR_Request ** req_p) MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_mpi_iscatterv(const void *sendbuf,
                                                     const MPI_Aint * sendcounts,
                                                     const MPI_Aint * displs, MPI_Datatype sendtype,
                                                     void *recvbuf, MPI_Aint recvcount,
                                                     MPI_Datatype recvtype, int root,
                                                     MPIR_Comm * comm_ptr,
                                                     MPIR_Request ** req_p)
    MPL_STATIC_INLINE_SUFFIX;
int MPIDI_OFL_mpi_type_commit_hook(MPIR_Datatype * type);
int MPIDI_OFL_mpi_type_free_hook(MPIR_Datatype * type);
int MPIDI_OFL_mpi_op_commit_hook(MPIR_Op * op_p);
int MPIDI_OFL_mpi_op_free_hook(MPIR_Op * op_p);
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_op_cs_enter_hook(MPIR_Win * win)
    MPL_STATIC_INLINE_SUFFIX;
MPL_STATIC_INLINE_PREFIX int MPIDI_OFL_rma_op_cs_exit_hook(MPIR_Win * win) MPL_STATIC_INLINE_SUFFIX;

#endif /* OFL_H_INCLUDED */
