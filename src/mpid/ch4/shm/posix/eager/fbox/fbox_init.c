/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpidimpl.h"
#include "fbox_noinline.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE
      category    : CH4
      type        : int
      default     : 3
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The size of the array to store expected receives to speed up fastbox polling.

    - name        : MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_BATCH_SIZE
      category    : CH4
      type        : int
      default     : 4
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        The number of fastboxes to poll during one iteration of the progress loop.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

#define FBOX_INDEX(sender, receiver)  ((MPIR_Process.local_size) * (sender) + (receiver))
#define FBOX_OFFSET(sender, receiver) (MPIDI_POSIX_FBOX_SIZE * FBOX_INDEX(sender, receiver))

MPIDI_POSIX_eager_fbox_control_t MPIDI_POSIX_eager_fbox_control_global = { 0 };

int MPIDI_POSIX_fbox_init(int rank, int size)
{
    int mpi_errno = MPI_SUCCESS;
    int i = 0;

    MPIR_FUNC_ENTER;

    MPIR_CHKPMEM_DECL(3);

    MPIR_CHKPMEM_MALLOC(MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks,
                        int16_t *,
                        sizeof(*MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks) *
                        (MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE + 1), mpi_errno,
                        "first_poll_local_ranks", MPL_MEM_SHM);

    /* -1 means we aren't looking for anything in particular. */
    for (i = 0; i < MPIR_CVAR_CH4_SHM_POSIX_EAGER_FBOX_POLL_CACHE_SIZE; i++) {
        MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] = -1;
    }

    /* The final entry in the cache array is for tracking the fallback place to start looking for
     * messages if all other entries in the cache are empty. */
    MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks[i] = 0;

    /* Create region with one fastbox for every pair of local processes. */
    size_t len =
        MPIR_Process.local_size * MPIR_Process.local_size * MPIDI_POSIX_FBOX_SIZE;

    /* Actually allocate the segment and assign regions to the pointers */
    mpi_errno = MPIDU_Init_shm_alloc(len, &MPIDI_POSIX_eager_fbox_control_global.shm_ptr);
    MPIR_ERR_CHECK(mpi_errno);

    /* Allocate table of pointers to fastboxes */
    MPIR_CHKPMEM_MALLOC(MPIDI_POSIX_eager_fbox_control_global.mailboxes.in,
                        MPIDI_POSIX_fastbox_t **,
                        MPIR_Process.local_size * sizeof(MPIDI_POSIX_fastbox_t *), mpi_errno,
                        "fastboxes", MPL_MEM_SHM);
    MPIR_CHKPMEM_MALLOC(MPIDI_POSIX_eager_fbox_control_global.mailboxes.out,
                        MPIDI_POSIX_fastbox_t **,
                        MPIR_Process.local_size * sizeof(MPIDI_POSIX_fastbox_t *), mpi_errno,
                        "fastboxes", MPL_MEM_SHM);

    /* Fill in fbox tables */
    char *fastboxes_p = (char *) MPIDI_POSIX_eager_fbox_control_global.shm_ptr;
    for (i = 0; i < MPIR_Process.local_size; i++) {
        MPIDI_POSIX_eager_fbox_control_global.mailboxes.in[i] =
            (void *) (fastboxes_p + FBOX_OFFSET(i, MPIR_Process.local_rank));
        MPIDI_POSIX_eager_fbox_control_global.mailboxes.out[i] =
            (void *) (fastboxes_p + FBOX_OFFSET(MPIR_Process.local_rank, i));

        memset(MPIDI_POSIX_eager_fbox_control_global.mailboxes.in[i], 0, MPIDI_POSIX_FBOX_SIZE);
    }

    mpi_errno = MPIDU_Init_shm_barrier();
    MPIR_ERR_CHECK(mpi_errno);

    MPIR_CHKPMEM_COMMIT();

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    MPIR_CHKPMEM_REAP();
    goto fn_exit;
}

int MPIDI_POSIX_fbox_post_init(void)
{
    int mpi_errno = MPI_SUCCESS;

    return mpi_errno;
}

int MPIDI_POSIX_fbox_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_ENTER;

    MPL_free(MPIDI_POSIX_eager_fbox_control_global.mailboxes.in);
    MPL_free(MPIDI_POSIX_eager_fbox_control_global.mailboxes.out);
    MPL_free(MPIDI_POSIX_eager_fbox_control_global.first_poll_local_ranks);

    mpi_errno = MPIDU_Init_shm_free(MPIDI_POSIX_eager_fbox_control_global.shm_ptr);

  fn_exit:
    MPIR_FUNC_EXIT;
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
