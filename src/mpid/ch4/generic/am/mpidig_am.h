/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDIG_AM_H_INCLUDED
#define MPIDIG_AM_H_INCLUDED

#define MPIDI_AM_HANDLERS_MAX (64)

enum {
    MPIDIG_SEND = 0,            /* Eager send */

    /* new pipeline protocol, RTS, CTS and segment transmission. The protocol start by sender
     * sending RTS to receiver. The RTS contain the headers and the first segment of data.
     * The receiver will to match a posted recv request. If no posted recv, it will create a
     * recv request for the unexpected data and the first chunk will be received into the
     * unexpected buffer (that is allocated from a GenQ private queue). The receiver will only
     * send back the CTS when the incoming/unexpected message is matched. The sender will wait
     * for the CTS before sending the 2nd and rest segments.
     * Note if the receiver received the RTS for the pipeline protocol, it should not refuse
     * and prefer a different protocol. This is because the pipeline is the ultimate fallback
     * protocol for long messages. If the sender is sending the pipeline RTS, it means either
     * pipeline is the only protocol available for this data. This could be due to the SHM/NM
     * only supports pipeline or a different protocol was previously rejected by the receiver.
     * For example, the sender could attemp to send using RDMA read and the receiver cannot/do
     * not want to receive using RDMA read. Thus the sender fallbacks to pipeline. */
    MPIDIG_SEND_PIPELINE_RTS,
    MPIDIG_SEND_PIPELINE_CTS,
    MPIDIG_SEND_PIPELINE_SEG,

    MPIDIG_SSEND_REQ,
    MPIDIG_SSEND_ACK,

    MPIDIG_PUT_REQ,
    MPIDIG_PUT_ACK,
    MPIDIG_PUT_DT_REQ,
    MPIDIG_PUT_DAT_REQ,
    MPIDIG_PUT_DT_ACK,

    MPIDIG_GET_REQ,
    MPIDIG_GET_ACK,

    MPIDIG_ACC_REQ,
    MPIDIG_ACC_ACK,
    MPIDIG_ACC_DT_REQ,
    MPIDIG_ACC_DAT_REQ,
    MPIDIG_ACC_DT_ACK,

    MPIDIG_GET_ACC_REQ,
    MPIDIG_GET_ACC_ACK,
    MPIDIG_GET_ACC_DT_REQ,
    MPIDIG_GET_ACC_DAT_REQ,
    MPIDIG_GET_ACC_DT_ACK,

    MPIDIG_CSWAP_REQ,
    MPIDIG_CSWAP_ACK,
    MPIDIG_FETCH_OP,

    MPIDIG_WIN_COMPLETE,
    MPIDIG_WIN_POST,
    MPIDIG_WIN_LOCK,
    MPIDIG_WIN_LOCK_ACK,
    MPIDIG_WIN_UNLOCK,
    MPIDIG_WIN_UNLOCK_ACK,
    MPIDIG_WIN_LOCKALL,
    MPIDIG_WIN_LOCKALL_ACK,
    MPIDIG_WIN_UNLOCKALL,
    MPIDIG_WIN_UNLOCKALL_ACK,

    MPIDIG_COMM_ABORT,

    MPIDI_OFI_INTERNAL_HANDLER_CONTROL,

    MPIDIG_HANDLER_STATIC_MAX
};

enum {
    MPIDIG_AM_PROTOCOL__EAGER,
    MPIDIG_AM_PROTOCOL__PIPELINE,
    MPIDIG_AM_PROTOCOL__RDMA_READ
};

typedef int (*MPIDIG_am_target_cmpl_cb) (MPIR_Request * req);
typedef int (*MPIDIG_am_origin_cb) (MPIR_Request * req);

/* Target message callback, or handler function
 *
 * If req on input is NULL, the callback may allocate a request object. If a request
 * object is returned, the caller is expected to transfer the payload to the request,
 * and call target_cmpl_cb upon complete.
 *
 * If is_async is false/0, a request object will never be returned.
 */
typedef int (*MPIDIG_am_target_msg_cb) (int handler_id, void *am_hdr,
                                        void *data, MPI_Aint data_sz,
                                        int is_local, int is_async, MPIR_Request ** req);

typedef struct MPIDIG_global_t {
    MPIDIG_am_target_msg_cb target_msg_cbs[MPIDI_AM_HANDLERS_MAX];
    MPIDIG_am_origin_cb origin_cbs[MPIDI_AM_HANDLERS_MAX];
} MPIDIG_global_t;
extern MPIDIG_global_t MPIDIG_global;

void MPIDIG_am_reg_cb(int handler_id,
                      MPIDIG_am_origin_cb origin_cb, MPIDIG_am_target_msg_cb target_msg_cb);
int MPIDIG_am_reg_cb_dynamic(MPIDIG_am_origin_cb origin_cb, MPIDIG_am_target_msg_cb target_msg_cb);

int MPIDIG_am_init(void);
void MPIDIG_am_finalize(void);

/* am protocol prototypes */

void MPIDIG_am_comm_abort_init(void);
int MPIDIG_am_comm_abort(MPIR_Comm * comm, int exit_code);

int MPIDIG_am_check_init(void);

static inline bool MPIDIG_am_check_size_le_eager_limit(size_t data_sz, int am_op,
                                                       size_t eager_limit)
{
    switch (am_op) {
        case MPIDIG_SEND:
            return (data_sz + sizeof(MPIDIG_hdr_t) <= eager_limit);
            break;
        case MPIDIG_SEND_PIPELINE_RTS:
        case MPIDIG_SEND_PIPELINE_CTS:
        case MPIDIG_SEND_PIPELINE_SEG:
            MPIR_Assert(0);
            break;
        case MPIDIG_SSEND_REQ:
            return (data_sz + sizeof(MPIDIG_hdr_t) + sizeof(MPIDIG_ssend_req_msg_t) <= eager_limit);
            break;
        case MPIDIG_SSEND_ACK:
        case MPIDIG_PUT_REQ:
        case MPIDIG_PUT_ACK:
        case MPIDIG_PUT_DT_REQ:
        case MPIDIG_PUT_DAT_REQ:
        case MPIDIG_PUT_DT_ACK:
        case MPIDIG_GET_REQ:
        case MPIDIG_GET_ACK:
        case MPIDIG_ACC_REQ:
        case MPIDIG_ACC_ACK:
        case MPIDIG_ACC_DT_REQ:
        case MPIDIG_ACC_DAT_REQ:
        case MPIDIG_ACC_DT_ACK:
        case MPIDIG_GET_ACC_REQ:
        case MPIDIG_GET_ACC_ACK:
        case MPIDIG_GET_ACC_DT_REQ:
        case MPIDIG_GET_ACC_DAT_REQ:
        case MPIDIG_GET_ACC_DT_ACK:
        case MPIDIG_CSWAP_REQ:
        case MPIDIG_CSWAP_ACK:
        case MPIDIG_FETCH_OP:
        case MPIDIG_WIN_COMPLETE:
        case MPIDIG_WIN_POST:
        case MPIDIG_WIN_LOCK:
        case MPIDIG_WIN_LOCK_ACK:
        case MPIDIG_WIN_UNLOCK:
        case MPIDIG_WIN_UNLOCK_ACK:
        case MPIDIG_WIN_LOCKALL:
        case MPIDIG_WIN_LOCKALL_ACK:
        case MPIDIG_WIN_UNLOCKALL:
        case MPIDIG_WIN_UNLOCKALL_ACK:
            MPIR_Assert(0);
            break;
        default:
            MPIR_Assert(0);
    }
    return false;
}

#endif /* MPIDIG_AM_H_INCLUDED */
