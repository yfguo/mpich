/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/* TODO: this file should be generated */

#include "csel_internal.h"

const char *Csel_coll_type_str[] = {
    /* MPIR_CSEL_COLL_TYPE__ALLGATHER */ "allgather",
    /* MPIR_CSEL_COLL_TYPE__ALLGATHERV */ "allgatherv",
    /* MPIR_CSEL_COLL_TYPE__ALLREDUCE */ "allreduce",
    /* MPIR_CSEL_COLL_TYPE__ALLTOALL */ "alltoall",
    /* MPIR_CSEL_COLL_TYPE__ALLTOALLV */ "alltoallv",
    /* MPIR_CSEL_COLL_TYPE__ALLTOALLW */ "alltoallw",
    /* MPIR_CSEL_COLL_TYPE__BARRIER */ "barrier",
    /* MPIR_CSEL_COLL_TYPE__BCAST */ "bcast",
    /* MPIR_CSEL_COLL_TYPE__EXSCAN */ "exscan",
    /* MPIR_CSEL_COLL_TYPE__GATHER */ "gather",
    /* MPIR_CSEL_COLL_TYPE__GATHERV */ "gatherv",
    /* MPIR_CSEL_COLL_TYPE__IALLGATHER */ "iallgather",
    /* MPIR_CSEL_COLL_TYPE__IALLGATHERV */ "iallgatherv",
    /* MPIR_CSEL_COLL_TYPE__IALLREDUCE */ "iallreduce",
    /* MPIR_CSEL_COLL_TYPE__IALLTOALL */ "ialltoall",
    /* MPIR_CSEL_COLL_TYPE__IALLTOALLV */ "ialltoallv",
    /* MPIR_CSEL_COLL_TYPE__IALLTOALLW */ "ialltoallw",
    /* MPIR_CSEL_COLL_TYPE__IBARRIER */ "ibarrier",
    /* MPIR_CSEL_COLL_TYPE__IBCAST */ "ibcast",
    /* MPIR_CSEL_COLL_TYPE__IEXSCAN */ "iexscan",
    /* MPIR_CSEL_COLL_TYPE__IGATHER */ "igather",
    /* MPIR_CSEL_COLL_TYPE__IGATHERV */ "igatherv",
    /* MPIR_CSEL_COLL_TYPE__INEIGHBOR_ALLGATHER */ "ineighbor_allgather",
    /* MPIR_CSEL_COLL_TYPE__INEIGHBOR_ALLGATHERV */ "ineighbor_allgatherv",
    /* MPIR_CSEL_COLL_TYPE__INEIGHBOR_ALLTOALL */ "ineighbor_alltoall",
    /* MPIR_CSEL_COLL_TYPE__INEIGHBOR_ALLTOALLV */ "ineighbor_alltoallv",
    /* MPIR_CSEL_COLL_TYPE__INEIGHBOR_ALLTOALLW */ "ineighbor_alltoallw",
    /* MPIR_CSEL_COLL_TYPE__IREDUCE */ "ireduce",
    /* MPIR_CSEL_COLL_TYPE__IREDUCE_SCATTER */ "ireduce_scatter",
    /* MPIR_CSEL_COLL_TYPE__IREDUCE_SCATTER_BLOCK */ "ireduce_scatter_block",
    /* MPIR_CSEL_COLL_TYPE__ISCAN */ "iscan",
    /* MPIR_CSEL_COLL_TYPE__ISCATTER */ "iscatter",
    /* MPIR_CSEL_COLL_TYPE__ISCATTERV */ "iscatterv",
    /* MPIR_CSEL_COLL_TYPE__NEIGHBOR_ALLGATHER */ "neighbor_allgather",
    /* MPIR_CSEL_COLL_TYPE__NEIGHBOR_ALLGATHERV */ "neighbor_allgatherv",
    /* MPIR_CSEL_COLL_TYPE__NEIGHBOR_ALLTOALL */ "neighbor_alltoall",
    /* MPIR_CSEL_COLL_TYPE__NEIGHBOR_ALLTOALLV */ "neighbor_alltoallv",
    /* MPIR_CSEL_COLL_TYPE__NEIGHBOR_ALLTOALLW */ "neighbor_alltoallw",
    /* MPIR_CSEL_COLL_TYPE__REDUCE */ "reduce",
    /* MPIR_CSEL_COLL_TYPE__REDUCE_SCATTER */ "reduce_scatter",
    /* MPIR_CSEL_COLL_TYPE__REDUCE_SCATTER_BLOCK */ "reduce_scatter_block",
    /* MPIR_CSEL_COLL_TYPE__SCAN */ "scan",
    /* MPIR_CSEL_COLL_TYPE__SCATTER */ "scatter",
    /* MPIR_CSEL_COLL_TYPE__SCATTERV */ "scatterv",
    /* MPIR_CSEL_COLL_TYPE__END */ "!END_OF_COLLECTIVE"
};

const char *Csel_comm_hierarchy_str[] = {
    /* MPIR_CSEL_COMM_HIERARCHY__FLAT */ "flat",
    /* MPIR_CSEL_COMM_HIERARCHY__NODE */ "node",
    /* MPIR_CSEL_COMM_HIERARCHY__NODE_ROOTS */ "node_roots",
    /* MPIR_CSEL_COMM_HIERARCHY__PARENT */ "parent",
    /* MPIR_CSEL_COMM_HIERARCHY__END */ "!END_OF_COMM_HIERARCHY"
};

const char *Csel_container_type_str[] = {
#include "csel_constants_gen.h"

    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_POSIX_mpi_bcast_release_gather */
    "MPIDI_POSIX_mpi_bcast_release_gather",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_POSIX_mpi_bcast_ipc_read */
    "MPIDI_POSIX_mpi_bcast_ipc_read",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Bcast_impl */ "MPIR_Bcast_impl",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_POSIX_mpi_barrier_release_gather */
    "MPIDI_POSIX_mpi_barrier_release_gather",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Barrier_impl */ "MPIR_Barrier_impl",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_POSIX_mpi_allreduce_release_gather */
    "MPIDI_POSIX_mpi_allreduce_release_gather",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Allreduce_impl */ "MPIR_Allreduce_impl",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_POSIX_mpi_reduce_release_gather */
    "MPIDI_POSIX_mpi_reduce_release_gather",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_impl */ "MPIR_Reduce_impl",

    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Barrier_intra_composition_alpha */
    "MPIDI_Barrier_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Barrier_intra_composition_beta */
    "MPIDI_Barrier_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Bcast_intra_composition_alpha */
    "MPIDI_Bcast_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Bcast_intra_composition_beta */
    "MPIDI_Bcast_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Bcast_intra_composition_gamma */
    "MPIDI_Bcast_intra_composition_gamma",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Bcast_intra_composition_delta */
    "MPIDI_Bcast_intra_composition_delta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Reduce_intra_composition_alpha */
    "MPIDI_Reduce_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Reduce_intra_composition_beta */
    "MPIDI_Reduce_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Reduce_intra_composition_gamma */
    "MPIDI_Reduce_intra_composition_gamma",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allreduce_intra_composition_alpha */
    "MPIDI_Allreduce_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allreduce_intra_composition_beta */
    "MPIDI_Allreduce_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allreduce_intra_composition_gamma */
    "MPIDI_Allreduce_intra_composition_gamma",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allreduce_intra_composition_delta */
    "MPIDI_Allreduce_intra_composition_delta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Alltoall_intra_composition_alpha */
    "MPIDI_Alltoall_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Alltoall_intra_composition_beta */
    "MPIDI_Alltoall_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Alltoallv_intra_composition_alpha */
    "MPIDI_Alltoallv_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Alltoallw_intra_composition_alpha */
    "MPIDI_Alltoallw_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allgather_intra_composition_alpha */
    "MPIDI_Allgather_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allgather_intra_composition_beta */
    "MPIDI_Allgather_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Allgatherv_intra_composition_alpha */
    "MPIDI_Allgatherv_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Gather_intra_composition_alpha */
    "MPIDI_Gather_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Gatherv_intra_composition_alpha */
    "MPIDI_Gatherv_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Scatter_intra_composition_alpha */
    "MPIDI_Scatter_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Scatterv_intra_composition_alpha */
    "MPIDI_Scatterv_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Reduce_scatter_intra_composition_alpha */
    "MPIDI_Reduce_scatter_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Reduce_scatter_block_intra_composition_alpha */
    "MPIDI_Reduce_scatter_block_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Scan_intra_composition_alpha */
    "MPIDI_Scan_intra_composition_alpha",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Scan_intra_composition_beta */
    "MPIDI_Scan_intra_composition_beta",
    /* MPII_CSEL_CONTAINER_TYPE__COMPOSITION__MPIDI_Exscan_intra_composition_alpha */
    "MPIDI_Exscan_intra_composition_alpha",

    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_OFI_Bcast_intra_triggered_tagged */
    "MPIDI_OFI_Bcast_intra_triggered_tagged",
    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIDI_OFI_Bcast_intra_triggered_rma */
    "MPIDI_OFI_Bcast_intra_triggered_rma",

    /* MPII_CSEL_CONTAINER_TYPE__ALGORITHM__Algorithm_count */ "END_OF_MPIR_ALGO"
};
