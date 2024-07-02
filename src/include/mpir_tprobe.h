/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIR_TPROBE_H_INCLUDED
#define MPIR_TPROBE_H_INCLUDED

#include "low_overhead_timers.h"

#define MPIR_TPROBE_MAX_RECORD (1 << 20)
#define MPIR_TPROBE_POS_MASK (MPIR_TPROBE_MAX_RECORD - 1)

#define MPIR_TPROBE_ALL (0xffffffffffffffffull)

typedef enum {
    MPIR_TPROBE_EV__ISEND_ISSUE = 0x1ull,
    MPIR_TPROBE_EV__IRECV_ISSUE = 0x1ull << 1,
    MPIR_TPROBE_EV__PROGRESS_START = 0x1ull << 2,
    MPIR_TPROBE_EV__IQUEUE_TERM_ENQUEUE = 0x1ull << 3,
    MPIR_TPROBE_EV__IQUEUE_TERM_ENQUEUE_END = 0x1ull << 4,
    MPIR_TPROBE_EV__IQUEUE_TERM_DEQUEUE = 0x1ull << 5,
    MPIR_TPROBE_EV__IQUEUE_TERM_DEQUEUE_END = 0x1ull << 6,
    MPIR_TPROBE_EV__IQUEUE_COPYIN = 0x1ull << 7,
    MPIR_TPROBE_EV__IQUEUE_COPYIN_END = 0x1ull << 8,
    MPIR_TPROBE_EV__IQUEUE_COPYOUT = 0x1ull << 9,
    MPIR_TPROBE_EV__IQUEUE_COPYOUT_END = 0x1ull << 10,
    MPIR_TPROBE_EV__TAG_MATCH = 0x1ull << 11,
    MPIR_TPROBE_EV__HANDLE_POSTPONED = 0x1ull << 12,
    MPIR_TPROBE_EV__MPIDIG_COPYOUT = 0x1ull << 13,
    MPIR_TPROBE_EV__MPIDIG_COPYOUT_END = 0x1ull << 14,
    MPIR_TPROBE_EV__IQUEUE_FREEQ_ENQUEUE = 0x1ull << 15,
    MPIR_TPROBE_EV__IQUEUE_FREEQ_ENQUEUE_END = 0x1ull << 16,
    MPIR_TPROBE_EV__IQUEUE_FREEQ_DEQUEUE = 0x1ull << 17,
    MPIR_TPROBE_EV__IQUEUE_FREEQ_DEQUEUE_END = 0x1ull << 18,
    MPIR_TPROBE_EV__MPIDIG_SEG_COPYOUT = 0x1ull << 19,
    MPIR_TPROBE_EV__MPIDIG_SEG_COPYOUT_END = 0x1ull << 20
} MPIR_tprobe_event_e;

typedef struct {
    uint64_t timestamp;
    uint64_t event;
} MPIR_tprobe_record_t;

typedef struct {
    uint64_t ev_enabled;
    uint64_t max_record;
    uint64_t start_timestamp;
    uint64_t tsc_freq;
    uint64_t start_pos;
    uint64_t pos;
    int section;
    MPIR_tprobe_record_t *records;
} MPIR_tprobe_log_t;

extern MPIR_tprobe_log_t MPIR_tprobe_log;

int MPIR_tprobe_init(void);
void MPIR_tprobe_finalize(void);

int MPIR_tprobe_start(uint64_t ev_map);
int MPIR_tprobe_stop();
int MPIR_tprobe_sync();

static inline __attribute__ ((always_inline))
void MPIR_tprobe_record(uint64_t id)
{
    if (MPIR_tprobe_log.ev_enabled & id) {
        MPIR_tprobe_record_t *rec = &MPIR_tprobe_log.records[MPIR_tprobe_log.pos];
        rec->timestamp = rdtsc();
        rec->event = id;
        MPIR_tprobe_log.pos = (MPIR_tprobe_log.pos + 1) & MPIR_TPROBE_POS_MASK;
    }
}

#endif /* MPIR_TPROBE_H_INCLUDED */
