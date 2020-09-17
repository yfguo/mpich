/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPIDIG_AM_SEND_UTILS_H_INCLUDED
#define MPIDIG_AM_SEND_UTILS_H_INCLUDED

/* This file is for supporting routines used for pipelined data send. These routines mainly is for
 * managing the send request counters, completion counters and DT refcount */

/* prepare the send counters for size of data and datatype that we need to refcount */
MPL_STATIC_INLINE_PREFIX void MPIDIG_send_init(MPIR_Request * sreq, MPI_Datatype datatype,
                                               size_t data_sz)
{
    MPIDIG_sreq_async_t *send_async = &MPIDIG_REQUEST(sreq, req->send_async);
    send_async->datatype = datatype;
    send_async->data_sz_left = data_sz;
    send_async->offset = 0;
    send_async->seg_issued = 0;
    send_async->seg_fin = 0;
    send_async->started = false;
    MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE,
                    (MPL_DBG_FDEST,
                     "send init handle=0x%x cc=%d, data_sz %ld, offset %ld, seg_issued %d, %d",
                     sreq->handle, *(sreq->cc_ptr), send_async->data_sz_left, send_async->offset,
                     send_async->seg_issued, send_async->started));
}

/* start the send by holding cc for request and refcount for datatype. This ensures they are not
 * prematurally freed */
MPL_STATIC_INLINE_PREFIX void MPIDIG_send_start(MPIR_Request * sreq)
{
    MPIDIG_sreq_async_t *send_async = &MPIDIG_REQUEST(sreq, req->send_async);
    int c;
    if (!send_async->started) {
        send_async->started = true;
        MPIR_cc_incr(sreq->cc_ptr, &c);
        MPIR_Datatype_add_ref_if_not_builtin(send_async->datatype);
        MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE,
                        (MPL_DBG_FDEST,
                         "send start handle=0x%x cc=%d, %d", sreq->handle, *(sreq->cc_ptr),
                         send_async->started));
    }
}

MPL_STATIC_INLINE_PREFIX size_t MPIDIG_send_get_data_sz_left(MPIR_Request * sreq)
{
    return MPIDIG_REQUEST(sreq, req->send_async).data_sz_left;
}

MPL_STATIC_INLINE_PREFIX size_t MPIDIG_send_get_offset(MPIR_Request * sreq)
{
    return MPIDIG_REQUEST(sreq, req->send_async).offset;
}

/* indicating one segment is issued, update counter with actual send_size */
MPL_STATIC_INLINE_PREFIX void MPIDIG_send_issue_seg(MPIR_Request * sreq, size_t send_size)
{
    MPIDIG_sreq_async_t *send_async = &MPIDIG_REQUEST(sreq, req->send_async);
    if (send_async->started) {
        send_async->data_sz_left -= send_size;
        send_async->offset += send_size;
        send_async->seg_issued += 1;
    }
    MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE,
                    (MPL_DBG_FDEST,
                     "issue seg handle=0x%x cc=%d, offset %ld, seg_iss %d, seg_fin %d",
                     sreq->handle, *(sreq->cc_ptr), send_async->offset, send_async->seg_issued,
                     send_async->seg_fin));
}

/* origin side completion of one issued segment, release cc and refcount if all data is set and all
 * segments are completed
 * The send is considered completed if 1. data_sz_left is zero, 2. all issued segment are finished
 * */
MPL_STATIC_INLINE_PREFIX int MPIDIG_send_finish_seg(MPIR_Request * sreq)
{
    MPIDIG_sreq_async_t *send_async = &MPIDIG_REQUEST(sreq, req->send_async);
    if (send_async->started) {
        send_async->seg_fin += 1;
        MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE,
                        (MPL_DBG_FDEST,
                         "fin seg handle=0x%x cc=%d, offset %ld, seg_iss %d, seg_fin %d",
                         sreq->handle, *(sreq->cc_ptr), send_async->offset, send_async->seg_issued,
                         send_async->seg_fin));
        if (send_async->data_sz_left == 0 && send_async->seg_issued == send_async->seg_fin) {
            MPIR_Datatype_release_if_not_builtin(send_async->datatype);
            MPID_Request_complete(sreq);
            return 1;
        }
    }
    return 0;
}

#endif /* MPIDIG_AM_SEND_UTILS_H_INCLUDED */
