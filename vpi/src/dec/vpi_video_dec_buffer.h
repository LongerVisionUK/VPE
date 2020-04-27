/*
 * Copyright 2019 VeriSilicon, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __VPI_VIDEO_DEC_BUFFER_H__
#define __VPI_VIDEO_DEC_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vpi_video_dec.h"

void vpi_dec_buf_list_add(BufLink **head, BufLink *list);
BufLink* vpi_dec_buf_list_delete(BufLink *head);
int vpi_send_packet_to_decode_buffer(VpiDecCtx *vpi_ctx, VpePacket *vpi_packet,
                                     struct DWLLinearMem stream_buffer);
int vpi_dec_get_stream_buffer_index(VpiDecCtx *vpi_ctx, int status);
uint32_t vpi_dec_find_ext_buffer_index(VpiDecCtx *vpi_ctx, uint32_t *addr);
uint32_t vpi_dec_find_empty_index(VpiDecCtx *vpi_ctx);
void vpi_dec_release_ext_buffers(VpiDecCtx *vpi_ctx);
VpiRet vpi_dec_output_frame(VpiDecCtx *vpi_ctx, VpeFrame *vpi_frame,
                            struct DecPicturePpu *decoded_pic);

#ifdef __cplusplus
}
#endif

#endif
