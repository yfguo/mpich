/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef CH4I_COMM_H_INCLUDED
#define CH4I_COMM_H_INCLUDED

#include "ch4_types.h"
#include "ch4r_comm.h"
#include "utlist.h"

int MPIDI_map_size(MPIR_Comm_map_t map);
int MPIDI_detect_regular_model(int *lpid, int size, int *offset, int *blocksize, int *stride);
int MPIDI_src_comm_to_lut(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                          int size, int total_mapper_size, int mapper_offset);
int MPIDI_src_comm_to_mlut(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                           int size, int total_mapper_size, int mapper_offset);
int MPIDI_src_mlut_to_mlut(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                           MPIR_Comm_map_t * mapper, int total_mapper_size, int mapper_offset);
int MPIDI_src_map_to_lut(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                         MPIR_Comm_map_t * mapper, int total_mapper_size, int mapper_offset);
void MPIDI_direct_of_src_rmap(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                              MPIR_Comm_map_t * mapper);
void MPIDI_offset_of_src_rmap(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                              MPIR_Comm_map_t * mapper, int offset);
void MPIDI_stride_of_src_rmap(MPIDI_rank_map_t * src, MPIDI_rank_map_t * dest,
                              MPIR_Comm_map_t * mapper, int stride, int blocksize, int offset);
int MPIDI_check_convert_mlut_to_lut(MPIDI_rank_map_t * src);
int MPIDI_check_convert_lut_to_regular(MPIDI_rank_map_t * src);
int MPIDI_set_map(MPIDI_rank_map_t * src_rmap, MPIDI_rank_map_t * dest_rmap,
                  MPIR_Comm_map_t * mapper, int src_comm_size,
                  int total_mapper_size, int mapper_offset);
int MPIDI_comm_create_rank_map(MPIR_Comm * comm);
int MPIDI_check_disjoint_lupids(int lupids1[], int n1, int lupids2[], int n2);

#endif /* CH4I_COMM_H_INCLUDED */
