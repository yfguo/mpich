!   -*- Mode: Fortran; -*-
!
!   (C) 2019 by Argonne National Laboratory.
!   See COPYRIGHT in top-level directory.
!
!   Portions of this code were written by Intel Corporation.
!   Copyright (C) 2011-2019 Intel Corporation.  Intel provides this material
!   to Argonne National Laboratory subject to Software Grant and Corporate
!   Contributor License Agreement dated February 8, 2012.

subroutine MPIX_Allgatherv_init_f08ts(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, &
    recvtype, comm, info, request, ierror)
    use, intrinsic :: iso_c_binding, only : c_int
    use :: mpi_f08, only : MPI_Datatype, MPI_Comm, MPI_Info, MPI_Request
    use :: mpi_c_interface, only : c_Datatype, c_Comm, c_Info, c_Request
    use :: mpi_c_interface, only : MPIR_Allgatherv_init_cdesc, MPIR_Comm_size_c

    implicit none

    type(*), dimension(..), intent(in) :: sendbuf
    type(*), dimension(..) :: recvbuf
    integer, intent(in) :: sendcount
    integer, intent(in) :: recvcounts(*)
    integer, intent(in) :: displs(*)
    type(MPI_Datatype), intent(in) :: sendtype
    type(MPI_Datatype), intent(in) :: recvtype
    type(MPI_Comm), intent(in) :: comm
    type(MPI_Info), intent(in) :: info
    type(MPI_Request), intent(out) :: request
    integer, optional, intent(out) :: ierror

    integer(c_int) :: sendcount_c
    integer(c_int), allocatable, dimension(:) :: recvcounts_c
    integer(c_int), allocatable, dimension(:) :: displs_c
    integer(c_Datatype) :: sendtype_c
    integer(c_Datatype) :: recvtype_c
    integer(c_Comm) :: comm_c
    integer(c_Info) :: info_c
    integer(c_Request) :: request_c
    integer(c_int) :: ierror_c
    integer(c_int) :: err, length ! To get length of assumed-size arrays

    if (c_int == kind(0)) then
        ierror_c = MPIR_Allgatherv_init_cdesc(sendbuf, sendcount, sendtype%MPI_VAL, recvbuf, recvcounts, displs, &
            recvtype%MPI_VAL, comm%MPI_VAL, info%MPI_VAL, request%MPI_VAL)
    else
        comm_c = comm%MPI_VAL
        err = MPIR_Comm_size_c(comm_c, length)
        sendcount_c = sendcount
        sendtype_c = sendtype%MPI_VAL
        recvcounts_c(1:length) = recvcounts(1:length)
        displs_c(1:length) = displs(1:length)
        recvtype_c = recvtype%MPI_VAL
        comm_c = comm%MPI_VAL
        info_c = info%MPI_VAL
        ierror_c = MPIR_Allgatherv_init_cdesc(sendbuf, sendcount_c, sendtype_c, recvbuf, recvcounts_c, displs_c, &
            recvtype_c, comm_c, info_c, request_c)
        request%MPI_VAL = request_c
    end if

    if (present(ierror)) ierror = ierror_c

end subroutine MPIX_Allgatherv_init_f08ts
