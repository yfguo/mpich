/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include <mpiimpl.h>
#include <stdlib.h>
#include "typerep_internal.h"

#if (MPICH_DATATYPE_ENGINE == MPICH_DATATYPE_ENGINE_YAKSA)
#include "yaksa.h"
#else
#include <dataloop.h>
#endif

struct flatten_hdr {
    MPI_Aint size;
    MPI_Aint extent, ub, lb, true_ub, true_lb;
    int is_contig;
    int basic_type;
    MPI_Aint num_contig_blocks;
};

/*
 * MPIR_Typerep_flatten_size
 *
 * Parameters:
 * datatype_ptr        - (IN)  datatype to flatten
 * flattened_type_size - (OUT) buffer size needed for the flattened representation
 */
int MPIR_Typerep_flatten_size(MPIR_Datatype * datatype_ptr, int *flattened_type_size)
{
    int flattened_loop_size;
    int mpi_errno = MPI_SUCCESS;

#if (MPICH_DATATYPE_ENGINE == MPICH_DATATYPE_ENGINE_YAKSA)
    uintptr_t size;
    int rc = yaksa_flatten_size((yaksa_type_t) (intptr_t) datatype_ptr->typerep.handle, &size);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");
    assert(size <= INT_MAX);
    flattened_loop_size = (int) size;
#else
    MPIR_Dataloop_flatten_size(datatype_ptr, &flattened_loop_size);
#endif

    *flattened_type_size = flattened_loop_size + sizeof(struct flatten_hdr);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

/*
 * MPIR_Typerep_flatten
 *
 * Parameters:
 * datatype_ptr   - (IN)  datatype to flatten
 * flattened_type - (OUT) buffer that will contain the flattened representation
 */
int MPIR_Typerep_flatten(MPIR_Datatype * datatype_ptr, void *flattened_type)
{
    struct flatten_hdr *flatten_hdr = (struct flatten_hdr *) flattened_type;
    void *flattened_typerep = (void *) ((char *) flattened_type + sizeof(struct flatten_hdr));
    int mpi_errno = MPI_SUCCESS;

    flatten_hdr->size = datatype_ptr->size;
    flatten_hdr->extent = datatype_ptr->extent;
    flatten_hdr->ub = datatype_ptr->ub;
    flatten_hdr->lb = datatype_ptr->lb;
    flatten_hdr->true_ub = datatype_ptr->true_ub;
    flatten_hdr->true_lb = datatype_ptr->true_lb;
    flatten_hdr->is_contig = datatype_ptr->is_contig;
    flatten_hdr->basic_type = datatype_ptr->basic_type;
    flatten_hdr->num_contig_blocks = datatype_ptr->typerep.num_contig_blocks;

#if (MPICH_DATATYPE_ENGINE == MPICH_DATATYPE_ENGINE_YAKSA)
    int rc = yaksa_flatten((yaksa_type_t) (intptr_t) datatype_ptr->typerep.handle,
                           flattened_typerep);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");
#else
    mpi_errno = MPIR_Dataloop_flatten(datatype_ptr, flattened_typerep);
    MPIR_ERR_CHECK(mpi_errno);
#endif

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

/*
 * MPIR_Typerep_unflatten
 *
 * Parameters:
 * datatype_ptr   - (OUT) datatype into which the buffer will be unflattened
 * flattened_type - (IN)  buffer that contains the flattened representation
 */
int MPIR_Typerep_unflatten(MPIR_Datatype ** datatype_ptr, void *flattened_type)
{
    struct flatten_hdr *flatten_hdr = (struct flatten_hdr *) flattened_type;
    void *flattened_typerep = (void *) ((char *) flattened_type + sizeof(struct flatten_hdr));
    int mpi_errno = MPI_SUCCESS;
    MPIR_Datatype *new_dtp = NULL;

    new_dtp = (MPIR_Datatype *) MPIR_Handle_obj_alloc(&MPIR_Datatype_mem);
    if (!new_dtp) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s",
                             "MPIR_Datatype_mem");
    }
    MPIR_Object_set_ref(new_dtp, 1);

    new_dtp->is_committed = 1;
    new_dtp->attributes = 0;
    new_dtp->name[0] = 0;
    new_dtp->is_contig = flatten_hdr->is_contig;
    new_dtp->typerep.num_contig_blocks = flatten_hdr->num_contig_blocks;
    new_dtp->size = flatten_hdr->size;
    new_dtp->extent = flatten_hdr->extent;
    new_dtp->basic_type = flatten_hdr->basic_type;
    new_dtp->ub = flatten_hdr->ub;
    new_dtp->lb = flatten_hdr->lb;
    new_dtp->true_ub = flatten_hdr->true_ub;
    new_dtp->true_lb = flatten_hdr->true_lb;
    new_dtp->contents = NULL;
    new_dtp->flattened = NULL;

#if (MPICH_DATATYPE_ENGINE == MPICH_DATATYPE_ENGINE_YAKSA)
    int rc = yaksa_unflatten((yaksa_type_t *) & new_dtp->typerep.handle, flattened_typerep);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");
#else
    mpi_errno = MPIR_Dataloop_unflatten(new_dtp, flattened_typerep);
    MPIR_ERR_CHECK(mpi_errno);
#endif

    *datatype_ptr = new_dtp;

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}
