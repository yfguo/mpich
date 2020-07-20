/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef POSIX_TYPES_H_INCLUDED
#define POSIX_TYPES_H_INCLUDED

#include "mpidu_init_shm.h"
#include "mpidu_genq.h"

enum {
    MPIDI_POSIX_OK,
    MPIDI_POSIX_NOK
};

#define MPIDI_POSIX_AM_BUFF_SZ               (1 * 1024 * 1024)
#define MPIDI_POSIX_AM_HDR_POOL_CELL_SIZE            (1024)
#define MPIDI_POSIX_AM_HDR_POOL_NUM_CELLS_PER_CHUNK     (1024)
#define MPIDI_POSIX_AM_HDR_POOL_MAX_NUM_CELLS           (257 * 1024)

#define MPIDI_POSIX_AMREQUEST(req,field)      ((req)->dev.ch4.am.shm_am.posix.field)
#define MPIDI_POSIX_AMREQUEST_HDR(req, field) ((req)->dev.ch4.am.shm_am.posix.req_hdr->field)
#define MPIDI_POSIX_AMREQUEST_HDR_PTR(req)    ((req)->dev.ch4.am.shm_am.posix.req_hdr)
#define MPIDI_POSIX_REQUEST(req, field)       ((req)->dev.ch4.shm.posix.field)
#define MPIDI_POSIX_COMM(comm, field)         ((comm)->dev.ch4.shm.posix.field)

#define MPIDI_MAX_POSIX_EAGER_STRING_LEN 64

typedef enum {
    MPIDI_POSIX_DEFERRED_AM_ISEND_OP__EAGER,
    MPIDI_POSIX_DEFERRED_AM_ISEND_OP__PIPELINE
} MPIDI_POSIX_deferred_am_isend_op_e;

typedef struct MPIDI_POSIX_deferred_am_isend_req {
    int op;
    int grank;
    const void *buf;
    size_t count;
    MPI_Datatype datatype;
    MPIR_Request *sreq;
    MPIDI_POSIX_am_header_t msg_hdr;
    void *am_hdr;
    int header_only;

    struct MPIDI_POSIX_deferred_am_isend_req *prev;
    struct MPIDI_POSIX_deferred_am_isend_req *next;
} MPIDI_POSIX_deferred_am_isend_req_t;

typedef struct {
    MPIDU_genq_private_pool_t am_hdr_buf_pool;

    /* Postponed queue */
    MPIDI_POSIX_deferred_am_isend_req_t *deferred_am_isend_q;

    void *shm_ptr;

    /* Keep track of all of the local processes in MPI_COMM_WORLD and what their original rank was
     * in that communicator. */
    int num_local;
    int my_local_rank;
    int *local_ranks;
    int *local_procs;
    int local_rank_0;
} MPIDI_POSIX_global_t;

/* Each cell contains some data being communicated from one process to another. */
typedef struct MPIDI_POSIX_eager_iqueue_cell {
    uint16_t from;              /* Who is the message in the cell from */
    MPIDI_POSIX_am_header_t am_header;  /* If this cell is the beginning of a message, it will have
                                         * an active message header and this will point to it. */
} MPIDI_POSIX_eager_iqueue_cell_t;

typedef struct MPIDI_POSIX_eager_iqueue_transport {
    int num_cells;              /* The number of cells allocated to each terminal in this transport */
    int size_of_cell;           /* The size of each of the cells in this transport */
    MPIDU_genq_shmem_queue_u *terminals;        /* The list of all the terminals that
                                                 * describe each of the cells */
    MPIDU_genq_shmem_queue_t my_terminal;
    MPIDU_genq_shmem_pool_t cell_pool;
} MPIDI_POSIX_eager_iqueue_transport_t;

typedef struct MPIDI_POSIX_eager_iqueue_recv_transaction {
    void *pointer_to_cell;
} MPIDI_POSIX_eager_iqueue_recv_transaction_t;

typedef struct MPIDI_POSIX_eager_recv_transaction {
    /* Public */
    void *msg_hdr;
    void *payload;
    int src_grank;
    /* Private */
    MPIDI_POSIX_eager_iqueue_recv_transaction_t transport;
} MPIDI_POSIX_eager_recv_transaction_t;

#define MPIDI_POSIX_EAGER_IQUEUE_CELL_PAYLOAD(cell) \
    ((char*)(cell) + sizeof(MPIDI_POSIX_eager_iqueue_cell_t))

#define MPIDI_POSIX_EAGER_IQUEUE_CELL_CAPACITY(transport) \
    ((transport)->size_of_cell - sizeof(MPIDI_POSIX_eager_iqueue_cell_t))

extern MPIDI_POSIX_global_t MPIDI_POSIX_global;
extern MPIDI_POSIX_eager_iqueue_transport_t MPIDI_POSIX_eager_iqueue_transport_global;

#endif /* POSIX_TYPES_H_INCLUDED */
