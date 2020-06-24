/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpi.h"
#include "mpitest.h"
#include <stdio.h>
#include <string.h>
#include "dtypes.h"
#include "mpitest.h"

int main(int argc, char **argv)
{
    int err = 0;
    int recv_tag = 0;
    int world_rank;
    MPI_Request req;
    MPI_Status status;
    char buf = 0;

    MTest_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == 0) {
        MPI_Isend(&buf, 0, MPI_DATATYPE_NULL, 1, 1, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
    } else {
        MPI_Irecv(&buf, 0, MPI_DATATYPE_NULL, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
        recv_tag = status.MPI_TAG;
        if (recv_tag != 1) {
            err = 1;
        }
    }

    MTest_Finalize(err);
    return MTestReturnValue(err);
}
