/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpiimpl.h"
#include <strings.h>    /* for strcasecmp */

MPIR_tprobe_log_t MPIR_tprobe_log;

int MPIR_tprobe_init(void)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_tprobe_log.ev_enabled = 0x0;
    MPIR_tprobe_log.max_record = MPIR_TPROBE_MAX_RECORD;
    MPIR_tprobe_log.start_pos = 0;
    MPIR_tprobe_log.pos = 0;
    MPIR_tprobe_log.section = 0;
    MPIR_tprobe_log.records = (MPIR_tprobe_record_t *) MPL_malloc(MPIR_tprobe_log.max_record
                                                                  * sizeof(MPIR_tprobe_record_t),
                                                                  MPL_MEM_OTHER);
    MPIR_tprobe_log.tsc_freq = get_TSC_frequency();
    MPIR_tprobe_log.start_timestamp = rdtsc();
    return mpi_errno;
}

void MPIR_tprobe_finalize(void)
{
    MPL_free(MPIR_tprobe_log.records);
}

int MPIR_tprobe_sync()
{
    int mpi_errno = MPI_SUCCESS;
    mpi_errno = MPIR_Barrier(MPIR_Process.comm_world, MPIR_ERR_NONE);
    MPIR_tprobe_log.start_timestamp = rdtsc();
    mpi_errno = MPIR_Barrier(MPIR_Process.comm_world, MPIR_ERR_NONE);
    return mpi_errno;
}

int MPIR_tprobe_start(uint64_t ev_map)
{
    MPIR_tprobe_log.ev_enabled = ev_map;
    return MPI_SUCCESS;
}

int MPIR_tprobe_pause()
{
    MPIR_tprobe_log.ev_enabled = 0x0;
    return MPI_SUCCESS;
}

int MPIR_tprobe_stop()
{
    char filename[100];         // Allocate enough space for the filename

    MPIR_tprobe_log.ev_enabled = 0x0;

    snprintf(filename, sizeof(filename), "tprobe_%d_%d.log", MPIR_tprobe_log.section,
             MPIR_Process.rank);

    FILE *f = fopen(filename, "w");

    fprintf(f, "tsc_freq=%" PRIu64 "\n", MPIR_tprobe_log.tsc_freq);
    fprintf(f, "start_timestamp=%" PRIu64 "\n", MPIR_tprobe_log.start_timestamp);
    fprintf(f, "rank=%d\n", MPIR_Process.rank);
    fprintf(f, "##\n");

    if (MPIR_tprobe_log.pos < MPIR_tprobe_log.start_pos) {
        for (int i = MPIR_tprobe_log.start_pos; i < MPIR_tprobe_log.max_record; i++) {
            fprintf(f, "%" PRIu64 ",%" PRIu64 "\n", MPIR_tprobe_log.records[i].timestamp,
                    MPIR_tprobe_log.records[i].event);
        }
        for (int i = 0; i < MPIR_tprobe_log.pos; i++) {
            fprintf(f, "%" PRIu64 ",%" PRIu64 "\n", MPIR_tprobe_log.records[i].timestamp,
                    MPIR_tprobe_log.records[i].event);
        }
    } else {
        for (int i = MPIR_tprobe_log.start_pos; i < MPIR_tprobe_log.pos; i++) {
            fprintf(f, "%" PRIu64 ",%" PRIu64 "\n", MPIR_tprobe_log.records[i].timestamp,
                    MPIR_tprobe_log.records[i].event);
        }
    }

    fclose(f);

    MPIR_tprobe_log.section++;
    MPIR_tprobe_log.start_pos = 0;
    MPIR_tprobe_log.pos = 0;
    return MPI_SUCCESS;
}
