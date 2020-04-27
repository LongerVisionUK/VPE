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

#include <stdlib.h>
#include <string.h>

#include "vpe_types.h"
#include "vpi_log.h"
#include "vpi_video_dec_buffer.h"
#include "vpi_video_dec_picture_consume.h"
#include "vpi_video_dec_info.h"

void vpi_dec_buf_list_add(BufLink **head, BufLink *list)
{
    BufLink *temp;

    if(NULL == *head) {
        *head = list;
        (*head)->next = NULL;
    } else {
        temp = *head;
        while(temp) {
            if(NULL == temp->next) {
                temp->next = list;
                list->next = NULL;
                return;
            }
            temp = temp->next;
        }
    }
}

BufLink* vpi_dec_buf_list_delete(BufLink *head)
{
    if (NULL == head || NULL == head->next) {
        return NULL;
    }

    return head->next;
}

int vpi_send_packet_to_decode_buffer(VpiDecCtx *vpi_ctx, VpePacket *vpi_packet,
                                     struct DWLLinearMem stream_buffer)
{
    int ret = 0;

#ifdef NEW_MEM_ALLOC
    if (vpi_packet->data) {
        ret = dwl_edma_rc2ep_nolink(vpi_ctx->dwl_inst,
                                    (uint64_t)vpi_packet->data,
                                    stream_buffer.bus_address,
                                    vpi_packet->size);
    }
#endif

    return ret;
}

int vpi_dec_get_stream_buffer_index(VpiDecCtx *vpi_ctx, int status)
{
    int i;

    pthread_mutex_lock(&vpi_ctx->dec_thread_mutex);
    for(i = 0; i < vpi_ctx->allocated_buffers; i++) {
        if (vpi_ctx->stream_mem_used[i] == status) {
            VPILOGD("buffer %d status %d\n", i, status);
            pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
    return -1;
}

uint32_t vpi_dec_find_ext_buffer_index(VpiDecCtx *vpi_ctx, uint32_t *addr)
{
    uint32_t i;

    for (i = 0; i < vpi_ctx->num_buffers; i++) {
        if (vpi_ctx->ext_buffers[i].bus_address == (addr_t)addr) {
            break;
        }
    }

    ASSERT(i < vpi_ctx->num_buffers);
    return i;
}

uint32_t vpi_dec_find_empty_index(VpiDecCtx *vpi_ctx)
{
    uint32_t i;

    for (i = 0; i < MAX_BUFFERS; i++) {
        if (vpi_ctx->ext_buffers[i].bus_address == 0) {
            break;
        }
    }

    ASSERT(i < MAX_BUFFERS);
    return i;
}

void vpi_dec_release_ext_buffers(VpiDecCtx *vpi_ctx)
{
    int i;

    pthread_mutex_lock(&vpi_ctx->ext_buffer_ctrl);
    for (i = 0; i < vpi_ctx->num_buffers; i++) {
        if (vpi_ctx->pp_enabled) {
            DWLFreeLinear(vpi_ctx->dwl_inst, &vpi_ctx->ext_buffers[i]);
        } else {
            DWLFreeRefFrm(vpi_ctx->dwl_inst, &vpi_ctx->ext_buffers[i]);
        }
        DWLmemset(&vpi_ctx->ext_buffers[i], 0, sizeof(vpi_ctx->ext_buffers[i]));
    }
    pthread_mutex_unlock(&vpi_ctx->ext_buffer_ctrl);
}

VpiRet vpi_dec_output_frame(VpiDecCtx *vpi_ctx, VpeFrame *vpi_frame,
                            struct DecPicturePpu *decoded_pic)
{
    int i;
    //struct DecPicturePpu *pic = vpi_ctx->out_pic;
    struct DecPicturePpu *pic = malloc(sizeof(struct DecPicturePpu));
    if (!pic) {
        return VPI_ERR_MALLOC;
    }

    memset(pic, 0, sizeof(struct DecPicturePpu));
    memcpy(pic, decoded_pic, sizeof(struct DecPicturePpu));
    vpi_report_dec_pic_info(vpi_ctx, pic);
    vpi_ctx->cycle_count += pic->pictures[0].picture_info.cycles_per_mb;

    vpi_frame->width       = pic->pictures[1].pic_width;
    vpi_frame->height      = pic->pictures[1].pic_height;
    vpi_frame->linesize[0] = pic->pictures[1].pic_width;
    vpi_frame->linesize[1] = pic->pictures[1].pic_width / 2;
    vpi_frame->linesize[2] = pic->pictures[1].pic_width / 2;
    vpi_frame->key_frame =
        (pic->pictures[1].picture_info.pic_coding_type == DEC_PIC_TYPE_I);

    for (i = 0; i < MAX_STRM_BUFFERS; i++) {
        if (vpi_ctx->time_stamp_info[i].decode_id ==
            pic->pictures[0].picture_info.decode_id) {
            vpi_frame->pts     = vpi_ctx->time_stamp_info[i].pts;
            vpi_frame->pkt_dts = vpi_ctx->time_stamp_info[i].pkt_dts;
            vpi_ctx->time_stamp_info[i].used = 0;
            break;
        }
    }

    if (i == MAX_STRM_BUFFERS) {
        vpi_frame->pts     = VDEC_NOPTS_VALUE;
        vpi_frame->pkt_dts = VDEC_NOPTS_VALUE;
    }
    VPILOGD("width = %d, height = %d, linesize[0] = %d, key_frame = %d\n",
            vpi_frame->width, vpi_frame->height, vpi_frame->linesize[0],
            vpi_frame->key_frame);

    PpUnitConfig *pp0 = &vpi_ctx->vpi_dec_config.ppu_cfg[0];
    if (pp0->enabled == 1) {
        pic->pictures[0].pp_enabled = 0;
    } else {
        pic->pictures[0].pp_enabled = 1;
    }
    for (i = 1; i < 5; i++) {
        PpUnitConfig *pp = &vpi_ctx->vpi_dec_config.ppu_cfg[i - 1];
        if (pp->enabled == 1) {
            pic->pictures[i].pp_enabled = 1;
            VPILOGD("pic.pictures[%d].pp_enabled = %d,comperss_status=%d\n", i,
                    pic->pictures[i].pp_enabled,
                    pic->pictures[i].pic_compressed_status);
        } else {
            pic->pictures[i].pp_enabled = 0;
        }
    }

    for (i = 1; i < 5; i++) {
        VPILOGD("pic.pictures[%d].pp_enabled = %d, \
                comperss_status = %d, bit_depth_luma = %d\n",
                i, pic->pictures[i].pp_enabled,
                pic->pictures[i].pic_compressed_status,
                pic->pictures[i].sequence_info.bit_depth_luma);
    }
    VPILOGD("vpi_ctx->pic_rdy = 1\n");

    if (pic->pictures[1].pp_enabled) {
        //pic[1] means pp0, if pp0 enabled pic_info[0](rfc) should not set
        vpi_frame->pic_info[0].enabled = 0;
    } else {
        vpi_frame->pic_info[0].width   = vpi_frame->width;
        vpi_frame->pic_info[0].height  = vpi_frame->height;
        vpi_frame->pic_info[0].enabled = 1;

        vpi_frame->pic_info[0].picdata.is_interlaced =
            pic->pictures[0].sequence_info.is_interlaced;
        vpi_frame->pic_info[0].picdata.pic_stride = pic->pictures[0].pic_stride;
        vpi_frame->pic_info[0].picdata.crop_out_width =
            pic->pictures[0].sequence_info.crop_params.crop_out_width;
        vpi_frame->pic_info[0].picdata.crop_out_height =
            pic->pictures[0].sequence_info.crop_params.crop_out_height;
        vpi_frame->pic_info[0].picdata.pic_format =
            pic->pictures[0].picture_info.format;
        vpi_frame->pic_info[0].picdata.pic_pixformat =
            pic->pictures[0].picture_info.pixel_format;
        vpi_frame->pic_info[0].picdata.bit_depth_luma =
            pic->pictures[0].sequence_info.bit_depth_luma;
        vpi_frame->pic_info[0].picdata.bit_depth_chroma =
            pic->pictures[0].sequence_info.bit_depth_chroma;
        vpi_frame->pic_info[0].picdata.pic_compressed_status =
            pic->pictures[0].pic_compressed_status;

        if (vpi_ctx->resizes[0].x || vpi_ctx->resizes[0].y ||
            vpi_ctx->resizes[0].cw || vpi_ctx->resizes[0].ch) {
            vpi_frame->pic_info[0].crop.enabled = 1;
            vpi_frame->pic_info[0].crop.x       = vpi_ctx->resizes[0].x;
            vpi_frame->pic_info[0].crop.y       = vpi_ctx->resizes[0].y;
            vpi_frame->pic_info[0].crop.w       = vpi_ctx->resizes[0].cw;
            vpi_frame->pic_info[0].crop.h       = vpi_ctx->resizes[0].ch;
        }
    }

    for (i = 1; i < 5; i++) {
        PpUnitConfig *pp = &vpi_ctx->vpi_dec_config.ppu_cfg[i - 1];

        if (!pic->pictures[i].pp_enabled) {
            continue;
        }

        vpi_frame->pic_info[i].enabled = pic->pictures[i].pp_enabled;
        vpi_frame->pic_info[i].width = pp->scale.enabled ?
                pp->scale.width :
                (pp->crop.enabled ? pp->crop.width : vpi_frame->width);
        vpi_frame->pic_info[i].height = pp->scale.enabled ?
                pp->scale.height :
                (pp->crop.enabled ? pp->crop.height : vpi_frame->height);
        vpi_frame->pic_info[i].pic_width  = pic->pictures[i].pic_width;
        vpi_frame->pic_info[i].pic_height = pic->pictures[i].pic_height;

        vpi_frame->pic_info[i].picdata.is_interlaced =
            pic->pictures[i].sequence_info.is_interlaced;
        vpi_frame->pic_info[i].picdata.pic_stride = pic->pictures[i].pic_stride;
        vpi_frame->pic_info[i].picdata.crop_out_width =
            pic->pictures[i].sequence_info.crop_params.crop_out_width;
        vpi_frame->pic_info[i].picdata.crop_out_height =
            pic->pictures[i].sequence_info.crop_params.crop_out_height;
        vpi_frame->pic_info[i].picdata.pic_format =
            pic->pictures[i].picture_info.format;
        vpi_frame->pic_info[i].picdata.pic_pixformat =
            pic->pictures[i].picture_info.pixel_format;
        vpi_frame->pic_info[i].picdata.bit_depth_luma =
            pic->pictures[i].sequence_info.bit_depth_luma;
        vpi_frame->pic_info[i].picdata.bit_depth_chroma =
            pic->pictures[i].sequence_info.bit_depth_chroma;
        vpi_frame->pic_info[i].picdata.pic_compressed_status =
            pic->pictures[i].pic_compressed_status;

        VPILOGD("vpi_frame->pic_info[%d].enabled = %d\n", i,
                vpi_frame->pic_info[i].enabled);
        VPILOGD("vpi_frame->pic_info[%d].width = %d\n", i,
                vpi_frame->pic_info[i].width);
        VPILOGD("vpi_frame->pic_info[%d].height = %d\n", i,
                vpi_frame->pic_info[i].height);
        VPILOGD("vpi_frame->pic_info[%d].pic_width = %d\n", i,
                vpi_frame->pic_info[i].pic_width);
        VPILOGD("vpi_frame->pic_info[%d].pic_height = %d\n", i,
                vpi_frame->pic_info[i].pic_height);
        VPILOGD("vpi_frame->pic_info[%d].is_interlaced = %d\n", i,
                vpi_frame->pic_info[i].picdata.is_interlaced);
        VPILOGD("vpi_frame->pic_info[%d].pic_format = %d\n", i,
                vpi_frame->pic_info[i].picdata.pic_format);
        VPILOGD("vpi_frame->pic_info[%d].pic_pixformat = %d\n", i,
                vpi_frame->pic_info[i].picdata.pic_pixformat);
        VPILOGD("vpi_frame->pic_info[%d].bit_depth_luma = %d\n", i,
                vpi_frame->pic_info[i].picdata.bit_depth_luma);
        VPILOGD("vpi_frame->pic_info[%d].bit_depth_chroma = %d\n", i,
                vpi_frame->pic_info[i].picdata.bit_depth_chroma);
        VPILOGD("vpi_frame->pic_info[%d].pic_compressed_status = %d\n", i,
                vpi_frame->pic_info[i].picdata.pic_compressed_status);
    }

    VPILOGD("[%d][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)]\n",
            vpi_frame->pic_info[0].enabled, vpi_frame->pic_info[1].enabled,
            vpi_frame->pic_info[1].width, vpi_frame->pic_info[1].height,
            vpi_frame->pic_info[2].enabled, vpi_frame->pic_info[2].width,
            vpi_frame->pic_info[2].height, vpi_frame->pic_info[3].enabled,
            vpi_frame->pic_info[3].width, vpi_frame->pic_info[3].height,
            vpi_frame->pic_info[4].enabled, vpi_frame->pic_info[4].width,
            vpi_frame->pic_info[4].height);

    if (vpi_ctx->vce_ds_enable) {
        if (!vpi_frame->pic_info[1].enabled ||
            !vpi_frame->pic_info[2].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, \
                    pp0 and pp1 should be enabled!\n");
            return VPI_ERR_WRONG_STATE;
        }
        if (vpi_frame->pic_info[0].enabled || vpi_frame->pic_info[3].enabled ||
            vpi_frame->pic_info[4].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, \
                    except pp0 and pp1 should not be enabled!\n");
            return VPI_ERR_WRONG_STATE;
        }
        vpi_frame->pic_info[2].flag = 1;
    }

    vpi_frame->data[0]         = (uint8_t *)pic;
    vpi_frame->pic_struct_size = sizeof(struct DecPicturePpu);
    add_dec_pic_wait_consume_list(vpi_ctx, pic);

    return VPI_SUCCESS;
}
