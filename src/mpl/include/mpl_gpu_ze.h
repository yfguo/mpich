/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPL_GPU_ZE_H_INCLUDED
#define MPL_GPU_ZE_H_INCLUDED

#include "level_zero/ze_api.h"

typedef struct {
    ze_memory_allocation_properties_t prop;
    ze_device_handle_t device;
} ze_alloc_attr_t;

typedef ze_ipc_mem_handle_t MPL_gpu_ipc_mem_handle_t;
typedef ze_device_handle_t MPL_gpu_device_handle_t;
typedef ze_alloc_attr_t MPL_gpu_device_attr;
typedef void *MPL_gpu_stream_t;
typedef void *MPL_gpu_host_fn_t;
#define MPL_GPU_DEVICE_INVALID NULL

#endif /* ifndef MPL_GPU_ZE_H_INCLUDED */
