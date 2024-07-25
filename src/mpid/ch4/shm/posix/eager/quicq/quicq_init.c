/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "quicq_noinline.h"
#include "mpidu_genq.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_CH4_SHM_POSIX_QUICQ_NUM_CELLS
      category    : CH4
      type        : int
      default     : 8
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The number of cells used for the depth of the quicq.

    - name        : MPIR_CVAR_CH4_SHM_POSIX_QUICQ_CELL_SIZE
      category    : CH4
      type        : int
      default     : 256
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        Size of each cell.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

#define QUEUE_CELL_BASE(q_base) \
    ((char *) (q_base) + sizeof(MPIDI_POSIX_eager_quicq_cntr_t))

MPIDI_POSIX_eager_quicq_global_t MPIDI_POSIX_eager_quicq_global;

static int init_transport(int vci_src, int vci_dst)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_POSIX_eager_quicq_transport_t *transport;
    transport = MPIDI_POSIX_eager_quicq_get_transport(vci_src, vci_dst);

    transport->num_cells_per_queue = MPL_pof2(MPIR_CVAR_CH4_SHM_POSIX_QUICQ_NUM_CELLS);
    transport->size_of_cell = MPIR_CVAR_CH4_SHM_POSIX_QUICQ_CELL_SIZE;

    transport->cell_alloc_size = transport->size_of_cell
        + MPL_ROUND_UP_ALIGN(sizeof(MPIDI_POSIX_eager_quicq_cell_t), MPL_CACHELINE_SIZE);
    transport->cell_alloc_size = MPL_ROUND_UP_ALIGN(transport->cell_alloc_size, MPL_CACHELINE_SIZE);

    /* to and from each other processes */
    transport->num_queues = MPIR_Process.local_size - 1;
    int total_num_queues = MPIR_Process.local_size * transport->num_queues;

    int queue_obj_size = transport->cell_alloc_size * transport->num_cells_per_queue
        + sizeof(MPIDI_POSIX_eager_quicq_cntr_t);
    int shm_alloc_size = total_num_queues * queue_obj_size;

    mpi_errno = MPIDU_Init_shm_alloc(shm_alloc_size, &transport->shm_base);
    MPIR_ERR_CHECK(mpi_errno);

    transport->send_terminals = (MPIDI_POSIX_eager_quicq_terminal_t *)
        MPL_malloc(MPIR_Process.local_size * sizeof(MPIDI_POSIX_eager_quicq_terminal_t),
                   MPL_MEM_SHM);
    transport->recv_terminals = (MPIDI_POSIX_eager_quicq_terminal_t *)
        MPL_malloc(MPIR_Process.local_size * sizeof(MPIDI_POSIX_eager_quicq_terminal_t),
                   MPL_MEM_SHM);

    int my_rank = MPIR_Process.local_rank;
    transport->send_terminals[my_rank].cell_base = NULL;
    transport->send_terminals[my_rank].cntr = NULL;
    transport->send_terminals[my_rank].last_seq = 0;
    transport->send_terminals[my_rank].last_ack = 0;
    transport->recv_terminals[my_rank].cell_base = NULL;
    transport->recv_terminals[my_rank].cntr = NULL;
    transport->recv_terminals[my_rank].last_seq = 0;
    transport->recv_terminals[my_rank].last_ack = 0;
    for (int remote_rank = 0; remote_rank < MPIR_Process.local_size; remote_rank++) {
        int q_idx = 0;
        if (my_rank < remote_rank) {
            q_idx = my_rank * transport->num_queues + remote_rank - 1;
        } else if (my_rank == remote_rank) {
            continue;
        } else {
            q_idx = my_rank * transport->num_queues + remote_rank;
        }
        void *q_base = transport->shm_base + queue_obj_size * q_idx;
        memset(q_base, 0, queue_obj_size);
        transport->send_terminals[remote_rank].cell_base = QUEUE_CELL_BASE(q_base);
        transport->send_terminals[remote_rank].cntr = q_base;
        MPL_atomic_store_uint32(&transport->send_terminals[remote_rank].cntr->seq.a, 0);
        MPL_atomic_store_uint32(&transport->send_terminals[remote_rank].cntr->ack.a, 0);
        transport->send_terminals[remote_rank].last_seq = 0;
        transport->send_terminals[remote_rank].last_ack = 0;
    }
    for (int remote_rank = 0; remote_rank < MPIR_Process.local_size; remote_rank++) {
        int q_idx = 0;
        if (my_rank < remote_rank) {
            q_idx = remote_rank * transport->num_queues + my_rank;
        } else if (my_rank == remote_rank) {
            continue;
        } else {
            q_idx = remote_rank * transport->num_queues + my_rank - 1;
        }
        void *q_base = transport->shm_base + queue_obj_size * q_idx;
        /* recv terminal is initialized by the sender as send terminal */
        transport->recv_terminals[remote_rank].cell_base = QUEUE_CELL_BASE(q_base);
        transport->recv_terminals[remote_rank].cntr = q_base;
        transport->recv_terminals[remote_rank].last_seq = 0;
        transport->recv_terminals[remote_rank].last_ack = 0;
    }

    if (MPIR_CVAR_DEBUG_SUMMARY && MPIR_Process.rank == 0) {
        fprintf(stdout, "==== QUICQ sizes and limits ====\n");
        fprintf(stdout, "MPIR_CVAR_CH4_POSIX_QUICQ_NUM_CELLS %d\n",
                MPIR_CVAR_CH4_SHM_POSIX_QUICQ_NUM_CELLS);
        fprintf(stdout, "MPIR_CVAR_CH4_POSIX_QUICQ_CELL_SIZE %d\n",
                MPIR_CVAR_CH4_SHM_POSIX_QUICQ_CELL_SIZE);
        fprintf(stdout, "sizeof(MPIDI_POSIX_eager_quicq_cell_t): %lu\n",
                sizeof(MPIDI_POSIX_eager_quicq_cell_t));
        fprintf(stdout, "cell_alloc_size: %d\n", transport->cell_alloc_size);
        fprintf(stdout, "num_cells_per_queue: %d\n", transport->num_cells_per_queue);
        fprintf(stdout, "queue_obj_size: %d\n", queue_obj_size);
    }

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_POSIX_quicq_init(int rank, int size)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    /* ensure max alignment for payload */
    MPIR_Assert((MPIR_CVAR_CH4_SHM_POSIX_QUICQ_CELL_SIZE & (MAX_ALIGNMENT - 1)) == 0);
    MPIR_Assert((sizeof(MPIDI_POSIX_eager_quicq_cell_t) & (MAX_ALIGNMENT - 1)) == 0);

    /* Init vci 0. Communication on vci 0 is enabled afterwards. */
    MPIDI_POSIX_eager_quicq_global.max_vcis = 1;

    mpi_errno = init_transport(0, 0);
    MPIR_ERR_CHECK(mpi_errno);

    mpi_errno = MPIDU_Init_shm_barrier();
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_POSIX_quicq_post_init(void)
{
    int mpi_errno = MPI_SUCCESS;

    /* gather max_vcis */
    int max_vcis = 0;
    max_vcis = 0;
    MPIDU_Init_shm_put(&MPIDI_POSIX_global.num_vcis, sizeof(int));
    MPIDU_Init_shm_barrier();
    for (int i = 0; i < MPIR_Process.local_size; i++) {
        int num;
        MPIDU_Init_shm_get(i, sizeof(int), &num);
        if (max_vcis < num) {
            max_vcis = num;
        }
    }
    MPIDU_Init_shm_barrier();

    MPIDI_POSIX_eager_quicq_global.max_vcis = max_vcis;

    for (int vci_src = 0; vci_src < max_vcis; vci_src++) {
        for (int vci_dst = 0; vci_dst < max_vcis; vci_dst++) {
            if (vci_src == 0 && vci_dst == 0) {
                continue;
            }
            mpi_errno = init_transport(vci_src, vci_dst);
            MPIR_ERR_CHECK(mpi_errno);

        }
    }

    mpi_errno = MPIDU_Init_shm_barrier();
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_POSIX_quicq_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    int max_vcis = MPIDI_POSIX_eager_quicq_global.max_vcis;
    for (int vci_src = 0; vci_src < max_vcis; vci_src++) {
        for (int vci_dst = 0; vci_dst < max_vcis; vci_dst++) {
            MPIDI_POSIX_eager_quicq_transport_t *transport;
            transport = MPIDI_POSIX_eager_quicq_get_transport(vci_src, vci_dst);

            mpi_errno = MPIDU_Init_shm_free(transport->shm_base);
            MPIR_ERR_CHECK(mpi_errno);

            MPL_free(transport->send_terminals);
            MPL_free(transport->recv_terminals);
        }
    }

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
