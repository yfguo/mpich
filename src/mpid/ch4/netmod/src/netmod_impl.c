/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2018 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpl.h"

MPL_SUPPRESS_OSX_HAS_NO_SYMBOLS_WARNING;

#ifndef NETMOD_INLINE
#ifndef NETMOD_DISABLE_INLINES

#include "mpidimpl.h"

int MPIDI_NM_mpi_win_free(MPIR_Win ** win_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_FREE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_FREE);

    ret = MPIDI_NM_native_func->mpi_win_free(win_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_FREE);
    return ret;
}

int MPIDI_NM_mpi_win_create(void *base, MPI_Aint length, int disp_unit, MPIR_Info * info,
                            MPIR_Comm * comm_ptr, MPIR_Win ** win_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE);

    ret = MPIDI_NM_native_func->mpi_win_create(base, length, disp_unit, info, comm_ptr, win_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE);
    return ret;
}

int MPIDI_NM_mpi_win_attach(MPIR_Win * win, void *base, MPI_Aint size)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH);

    ret = MPIDI_NM_native_func->mpi_win_attach(win, base, size);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH);
    return ret;
}

int MPIDI_NM_mpi_win_allocate_shared(MPI_Aint size, int disp_unit, MPIR_Info * info_ptr,
                                     MPIR_Comm * comm_ptr, void **base_ptr, MPIR_Win ** win_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED);

    ret = MPIDI_NM_native_func->mpi_win_allocate_shared(size, disp_unit, info_ptr, comm_ptr,
                                                        base_ptr, win_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED);
    return ret;
}

int MPIDI_NM_mpi_win_detach(MPIR_Win * win, const void *base)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH);

    ret = MPIDI_NM_native_func->mpi_win_detach(win, base);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH);
    return ret;
}

int MPIDI_NM_mpi_win_shared_query(MPIR_Win * win, int rank, MPI_Aint * size, int *disp_unit,
                                  void *baseptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_SHARED_QUERY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_SHARED_QUERY);

    ret = MPIDI_NM_native_func->mpi_win_shared_query(win, rank, size, disp_unit, baseptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_SHARED_QUERY);
    return ret;
}

int MPIDI_NM_mpi_win_allocate(MPI_Aint size, int disp_unit, MPIR_Info * info, MPIR_Comm * comm,
                              void *baseptr, MPIR_Win ** win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE);

    ret = MPIDI_NM_native_func->mpi_win_allocate(size, disp_unit, info, comm, baseptr, win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE);
    return ret;
}

int MPIDI_NM_mpi_win_create_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_HOOK);

    ret = MPIDI_NM_func->mpi_win_create_hook(win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_allocate_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_HOOK);

    ret = MPIDI_NM_func->mpi_win_allocate_hook(win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_allocate_shared_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED_HOOK);

    ret = MPIDI_NM_func->mpi_win_allocate_shared_hook(win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ALLOCATE_SHARED_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_create_dynamic_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC_HOOK);

    ret = MPIDI_NM_func->mpi_win_create_dynamic_hook(win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_attach_hook(MPIR_Win * win, void *base, MPI_Aint size)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH_HOOK);

    ret = MPIDI_NM_func->mpi_win_attach_hook(win, base, size);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_ATTACH_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_detach_hook(MPIR_Win * win, const void *base)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH_HOOK);

    ret = MPIDI_NM_func->mpi_win_detach_hook(win, base);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_DETACH_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_free_hook(MPIR_Win * win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_FREE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_FREE_HOOK);

    ret = MPIDI_NM_func->mpi_win_free_hook(win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_FREE_HOOK);
    return ret;
}

int MPIDI_NM_mpi_win_create_dynamic(MPIR_Info * info, MPIR_Comm * comm, MPIR_Win ** win)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC);

    ret = MPIDI_NM_native_func->mpi_win_create_dynamic(info, comm, win);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_WIN_CREATE_DYNAMIC);
    return ret;
}

int MPIDI_NM_mpi_comm_connect(const char *port_name, MPIR_Info * info, int root, int timeout,
                              MPIR_Comm * comm, MPIR_Comm ** newcomm_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_COMM_CONNECT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_COMM_CONNECT);

    ret = MPIDI_NM_func->mpi_comm_connect(port_name, info, root, timeout, comm, newcomm_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_COMM_CONNECT);
    return ret;
}

int MPIDI_NM_mpi_comm_disconnect(MPIR_Comm * comm_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_COMM_DISCONNECT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_COMM_DISCONNECT);

    ret = MPIDI_NM_func->mpi_comm_disconnect(comm_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_COMM_DISCONNECT);
    return ret;
}

int MPIDI_NM_mpi_open_port(MPIR_Info * info_ptr, char *port_name)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_OPEN_PORT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_OPEN_PORT);

    ret = MPIDI_NM_func->mpi_open_port(info_ptr, port_name);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_OPEN_PORT);
    return ret;
}

int MPIDI_NM_mpi_close_port(const char *port_name)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_CLOSE_PORT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_CLOSE_PORT);

    ret = MPIDI_NM_func->mpi_close_port(port_name);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_CLOSE_PORT);
    return ret;
}

int MPIDI_NM_mpi_comm_accept(const char *port_name, MPIR_Info * info, int root, MPIR_Comm * comm,
                             MPIR_Comm ** newcomm_ptr)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_NM_MPI_COMM_ACCEPT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_NM_MPI_COMM_ACCEPT);

    ret = MPIDI_NM_func->mpi_comm_accept(port_name, info, root, comm, newcomm_ptr);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_NM_MPI_COMM_ACCEPT);
    return ret;
}

#endif
#endif
