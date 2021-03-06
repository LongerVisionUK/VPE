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

#include <string.h>
#include <assert.h>

#include "deccfg.h"
#include "decapicommon.h"
#include "ppu.h"

#include "vpe_types.h"
#include "vpi_log.h"
#include "vpi_video_h264dec.h"
#include "vpi_video_dec_info.h"
#include "vpi_video_dec_buffer.h"
#include "vpi_video_dec_cfg.h"
#include "vpi_video_dec_picture_consume.h"

uint32_t is_mono_chrome = 0;

VpiRet vpi_dec_h264_init(const void **inst, struct DecConfig config,
                         const void *dwl)
{
    struct H264DecConfig dec_cfg;
    H264DecMCConfig mcinit_cfg;
    enum DecRet rv = DEC_OK;

    mcinit_cfg.mc_enable = config.mc_cfg.mc_enable;
    mcinit_cfg.stream_consumed_callback =
        config.mc_cfg.stream_consumed_callback;

    dec_cfg.no_output_reordering = config.disable_picture_reordering;
    dec_cfg.error_handling       = DEC_EC_FAST_FREEZE;
    //dec_cfg.use_video_compressor = config.use_video_compressor;
    //dec_cfg.use_ringbuffer = 0;
    //dec_cfg.output_format = config.output_format;
    dec_cfg.decoder_mode          = config.decoder_mode;
    dec_cfg.dpb_flags             = DEC_REF_FRM_TILED_DEFAULT;
    dec_cfg.use_display_smoothing = 0;
    //dec_cfg.tile_by_tile = config.tile_by_tile;
#ifdef USE_EXTERNAL_BUFFER
    dec_cfg.guard_size           = 0;
    dec_cfg.use_adaptive_buffers = 0;
#endif
#if 0
    if (config.use_8bits_output) {
        dec_cfg.pixel_format = DEC_OUT_PIXEL_CUT_8BIT;
    } else if (config.use_p010_output) {
        dec_cfg.pixel_format = DEC_OUT_PIXEL_P010;
    } else if (config.use_1010_output) {
        dec_cfg.pixel_format = DEC_OUT_PIXEL_1010;
    } else if (config.use_bige_output) {
        dec_cfg.pixel_format = DEC_OUT_PIXEL_CUSTOMER1;
    } else {
        dec_cfg.pixel_format = DEC_OUT_PIXEL_DEFAULT;
    }
#endif
    memcpy(dec_cfg.ppu_config, config.ppu_cfg, sizeof(config.ppu_cfg));
    rv = H264DecInit(inst, dwl, dec_cfg.decoder_mode,
                     dec_cfg.no_output_reordering, dec_cfg.error_handling,
                     dec_cfg.use_display_smoothing, dec_cfg.dpb_flags, 0,
                     //dec_cfg.use_video_compressor,
                     dec_cfg.use_adaptive_buffers, dec_cfg.guard_size,
                     &mcinit_cfg);
    if (rv) {
        VPILOGD("H264DecInit ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

VpiRet vpi_dec_h264_get_info(VpiDecInst inst, struct DecSequenceInfo *info)
{
    H264DecInfo h264_info;
    enum DecRet rv                     = H264DecGetInfo(inst, &h264_info);
    info->pic_width                    = h264_info.pic_width;
    info->pic_height                   = h264_info.pic_height;
    info->sar_width                    = h264_info.sar_width;
    info->sar_height                   = h264_info.sar_height;
    info->crop_params.crop_left_offset = h264_info.crop_params.crop_left_offset;
    info->crop_params.crop_out_width   = h264_info.crop_params.crop_out_width;
    info->crop_params.crop_top_offset  = h264_info.crop_params.crop_top_offset;
    info->crop_params.crop_out_height  = h264_info.crop_params.crop_out_height;
    info->video_range                  = h264_info.video_range;
    info->matrix_coefficients          = h264_info.matrix_coefficients;
    info->is_mono_chrome               = h264_info.mono_chrome;
    is_mono_chrome                     = h264_info.mono_chrome;
    info->is_interlaced                = h264_info.interlaced_sequence;
    info->num_of_ref_frames            = h264_info.pic_buff_size;
    info->bit_depth_chroma = info->bit_depth_luma = 8;

    if (rv) {
        VPILOGD("H264DecGetInfo ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

VpiRet vpi_dec_h264_set_info(VpiDecInst inst, struct DecConfig config,
                             struct DecSequenceInfo *info)
{
    struct H264DecConfig dec_cfg;
    enum DecRet rv;

    dec_cfg.no_output_reordering = config.disable_picture_reordering;
    //dec_cfg.use_video_freeze_concealment = config.concealment_mode;
    //dec_cfg.use_video_compressor = config.use_video_compressor;
    //dec_cfg.use_ringbuffer = config.use_ringbuffer;
    //dec_cfg.output_format = config.output_format;
    dec_cfg.decoder_mode = config.decoder_mode;
#ifdef USE_EXTERNAL_BUFFER
    dec_cfg.guard_size           = 0;
    dec_cfg.use_adaptive_buffers = 0;
#endif
    memcpy(dec_cfg.ppu_config, config.ppu_cfg, sizeof(config.ppu_cfg));

    if (config.fscale_cfg.fixed_scale_enabled) {
        /* Convert fixed ratio scale to ppu_config[0] */
        dec_cfg.ppu_config[0].enabled = 1;
        if (!config.ppu_cfg[0].crop.enabled) {
            dec_cfg.ppu_config[0].crop.enabled = 1;
            dec_cfg.ppu_config[0].crop.x = info->crop_params.crop_left_offset;
            dec_cfg.ppu_config[0].crop.y = info->crop_params.crop_top_offset;
            dec_cfg.ppu_config[0].crop.width = info->crop_params.crop_out_width;
            dec_cfg.ppu_config[0].crop.height =
                info->crop_params.crop_out_height;
        }
        if (!config.ppu_cfg[0].scale.enabled &&
            config.fscale_cfg.down_scale_x && config.fscale_cfg.down_scale_y) {
            dec_cfg.ppu_config[0].scale.enabled = 1;
            dec_cfg.ppu_config[0].scale.width =
                dec_cfg.ppu_config[0].crop.width /
                config.fscale_cfg.down_scale_x;
            dec_cfg.ppu_config[0].scale.height =
                dec_cfg.ppu_config[0].crop.height /
                config.fscale_cfg.down_scale_y;
        }
    }
    dec_cfg.align         = config.align;
    dec_cfg.error_conceal = 0;
    /* TODO(min): assume 1-byte aligned is only applied for pp output */
    if (dec_cfg.align == DEC_ALIGN_1B) {
        dec_cfg.align = DEC_ALIGN_64B;
    }

    rv = H264DecSetInfo(inst, &dec_cfg);
    if (rv) {
        VPILOGD("H264DecSetInfo ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

enum DecRet vpi_dec_h264_next_picture(VpiDecInst inst,
                                      struct DecPicturePpu *pic)
{
    enum DecRet rv;
    uint32_t stride, stride_ch, i;
    H264DecPicture hpic;

    rv = H264DecNextPicture(inst, &hpic, 0);
    memset(pic, 0, sizeof(struct DecPicturePpu));
    if (rv != DEC_PIC_RDY) {
        return rv;
    }

    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
        stride    = hpic.pictures[i].pic_stride;
        stride_ch = hpic.pictures[i].pic_stride_ch;
#ifndef NEW_MEM_ALLOC
        pic->pictures[i].luma.virtual_address =
            (uint32_t *)hpic.pictures[i].output_picture;
#endif
        pic->pictures[i].luma.bus_address =
            hpic.pictures[i].output_picture_bus_address;
#if 0
        if (hpic.output_format == DEC_OUT_FRM_RASTER_SCAN) {
            hpic.pic_width = NEXT_MULTIPLE(hpic.pic_width, 16);
        }
#endif
        if (hpic.pictures[i].output_format == DEC_OUT_FRM_TILED_4X4) {
            pic->pictures[i].luma.size =
                stride * hpic.pictures[i].pic_height / 4;
            /*sunny correct*/
            if (((hpic.pictures[i].pic_height / 4) & 1) == 1) {
                pic->pictures[i].chroma.size =
                    stride_ch * (hpic.pictures[i].pic_height / 4 + 1) / 2;
            } else {
                pic->pictures[i].chroma.size =
                    stride_ch * hpic.pictures[i].pic_height / 8;
            }
        } else {
            pic->pictures[i].luma.size = stride * hpic.pictures[i].pic_height;
            pic->pictures[i].chroma.size =
                stride_ch * hpic.pictures[i].pic_height / 2;
        }
        /* TODO temporal solution to set chroma base here */
#ifndef NEW_MEM_ALLOC
        pic->pictures[i].chroma.virtual_address =
            (uint32_t *)hpic.pictures[i].output_picture_chroma;
#endif
        pic->pictures[i].chroma.bus_address =
            hpic.pictures[i].output_picture_chroma_bus_address;

        /* TODO(vmr): find out for real also if it is B frame */
        pic->pictures[i].picture_info.pic_coding_type =
            hpic.is_idr_picture[0] ? DEC_PIC_TYPE_I : DEC_PIC_TYPE_P;
        pic->pictures[i].picture_info.format = hpic.pictures[i].output_format;
        pic->pictures[i].picture_info.pixel_format  = DEC_OUT_PIXEL_DEFAULT;
        pic->pictures[i].picture_info.pic_id        = hpic.pic_id;
        pic->pictures[i].picture_info.decode_id     = hpic.decode_id[0];
        pic->pictures[i].picture_info.cycles_per_mb = hpic.cycles_per_mb;
        pic->pictures[i].sequence_info.pic_width  = hpic.pictures[i].pic_width;
        pic->pictures[i].sequence_info.pic_height = hpic.pictures[i].pic_height;
        pic->pictures[i].sequence_info.crop_params.crop_left_offset =
            hpic.crop_params.crop_left_offset;
        pic->pictures[i].sequence_info.crop_params.crop_top_offset =
            hpic.crop_params.crop_top_offset;
        if (hpic.interlaced) {
            if (hpic.crop_params.crop_out_width <= hpic.pictures[i].pic_width) {
                pic->pictures[i].sequence_info.crop_params.crop_out_width =
                    hpic.crop_params.crop_out_width;
            } else {
                pic->pictures[i].sequence_info.crop_params.crop_out_width =
                    hpic.pictures[i].pic_width;
            }
            if (hpic.crop_params.crop_out_height <=
                hpic.pictures[i].pic_height) {
                pic->pictures[i].sequence_info.crop_params.crop_out_height =
                    hpic.crop_params.crop_out_height;
            } else {
                pic->pictures[i].sequence_info.crop_params.crop_out_height =
                    hpic.pictures[i].pic_height;
            }
        } else {
            pic->pictures[i].sequence_info.crop_params.crop_out_width =
                hpic.crop_params.crop_out_width;
            pic->pictures[i].sequence_info.crop_params.crop_out_height =
                hpic.crop_params.crop_out_height;
        }

        pic->pictures[i].sequence_info.sar_width           = hpic.sar_width;
        pic->pictures[i].sequence_info.sar_height          = hpic.sar_height;
        pic->pictures[i].sequence_info.video_range         = 0;
        pic->pictures[i].sequence_info.matrix_coefficients = 0;
        pic->pictures[i].sequence_info.is_mono_chrome      = 0;
        pic->pictures[i].sequence_info.is_interlaced       = hpic.interlaced;
        pic->pictures[i].sequence_info.num_of_ref_frames   = 0;
        pic->pictures[i].sequence_info.bit_depth_luma   = hpic.bit_depth_luma;
        pic->pictures[i].sequence_info.bit_depth_chroma = hpic.bit_depth_chroma;
        pic->pictures[i].sequence_info.pic_stride = hpic.pictures[i].pic_stride;
        pic->pictures[i].sequence_info.pic_stride_ch =
            hpic.pictures[i].pic_stride_ch;
        pic->pictures[i].pic_width     = hpic.pictures[i].pic_width;
        pic->pictures[i].pic_height    = hpic.pictures[i].pic_height;
        pic->pictures[i].pic_stride    = hpic.pictures[i].pic_stride;
        pic->pictures[i].pic_stride_ch = hpic.pictures[i].pic_stride_ch;
        pic->pictures[i].pp_enabled    = 0;
    }

#ifdef SUPPORT_DEC400
    /*add for dec400 tables*/
    addr_t dec400_table_bus_address_base = 0;
    const uint32_t *dec400_table_base    = NULL;
    int dec400_table_offset              = 0;
    uint32_t y_size = 0, uv_size = 0;
    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
        pic->pictures[i].pic_compressed_status =
            hpic.pictures[i].pic_compressed_status;
#ifndef NEW_MEM_ALLOC
        if (pic->pictures[i].luma.virtual_address != NULL) {
#else
        if (pic->pictures[i].luma.bus_address != 0) {
#endif
            if (i == 0) {
                y_size = NEXT_MULTIPLE(pic->pictures[i].pic_height, 4) / 4 *
                         pic->pictures[i].pic_stride;
                uv_size = NEXT_MULTIPLE(pic->pictures[i].pic_height / 2, 4) /
                          4 * pic->pictures[i].pic_stride_ch;
#ifndef NEW_MEM_ALLOC
                pic->pictures[i].luma_table.virtual_address =
                    (uint32_t *)((uint8_t *)pic->pictures[i]
                                     .luma.virtual_address -
                                 DEC400_PP_TABLE_SIZE);
#endif
                pic->pictures[i].luma_table.bus_address =
                    pic->pictures[i].luma.bus_address - DEC400_PP_TABLE_SIZE;
                pic->pictures[i].luma_table.size =
                    NEXT_MULTIPLE(y_size / 1024 / 4 + ((y_size % 4096) ? 1 : 0),
                                  16);
#ifndef NEW_MEM_ALLOC
                pic->pictures[i].chroma_table.virtual_address =
                    (uint32_t *)((uint8_t *)pic->pictures[i]
                                     .luma.virtual_address -
                                 DEC400_YUV_TABLE_SIZE);
#endif
                pic->pictures[i].chroma_table.bus_address =
                    pic->pictures[i].luma.bus_address - DEC400_YUV_TABLE_SIZE;
                pic->pictures[i].chroma_table.size =
                    NEXT_MULTIPLE(uv_size / 1024 / 4 +
                                      ((uv_size % 4096) ? 1 : 0),
                                  16);
            } else {
                if (dec400_table_bus_address_base == 0) {
                    dec400_table_bus_address_base =
                        pic->pictures[i].luma.bus_address;
#ifndef NEW_MEM_ALLOC
                    dec400_table_base = pic->pictures[i].luma.virtual_address;
#endif
                }

                y_size = NEXT_MULTIPLE(pic->pictures[i].pic_height, 4) / 4 *
                         pic->pictures[i].pic_stride;
                uv_size = NEXT_MULTIPLE(pic->pictures[i].pic_height / 2, 4) /
                          4 * pic->pictures[i].pic_stride_ch;

                pic->pictures[i].luma_table.size =
                    NEXT_MULTIPLE(y_size / 1024 / 4 + ((y_size % 4096) ? 1 : 0),
                                  16);
                pic->pictures[i].chroma_table.size =
                    NEXT_MULTIPLE(uv_size / 1024 / 4 +
                                      ((uv_size % 4096) ? 1 : 0),
                                  16);
                dec400_table_offset += y_size + PP_LUMA_BUF_RES;
                dec400_table_offset += uv_size + PP_CHROMA_BUF_RES;
            }
        }
    }

    for (i = 1; i < DEC_MAX_OUT_COUNT; i++) {
#ifndef NEW_MEM_ALLOC
        if (pic->pictures[i].luma.virtual_address != NULL) {
#else
        if (pic->pictures[i].luma.bus_address != 0) {
#endif
#ifndef NEW_MEM_ALLOC
            pic->pictures[i].luma_table.virtual_address =
                (uint32_t *)((uint8_t *)dec400_table_base +
                             dec400_table_offset +
                             DEC400_PPn_Y_TABLE_OFFSET(i - 1));
#endif
            pic->pictures[i].luma_table.bus_address =
                dec400_table_bus_address_base + dec400_table_offset +
                DEC400_PPn_Y_TABLE_OFFSET(i - 1);
#ifndef NEW_MEM_ALLOC
            pic->pictures[i].chroma_table.virtual_address =
                (uint32_t *)((uint8_t *)dec400_table_base +
                             dec400_table_offset +
                             DEC400_PPn_UV_TABLE_OFFSET(i - 1));
#endif
            pic->pictures[i].chroma_table.bus_address =
                dec400_table_bus_address_base + dec400_table_offset +
                DEC400_PPn_UV_TABLE_OFFSET(i - 1);
        }
    }

#else
    uint32_t y_size = 0;

    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
        pic->pictures[i].pic_compressed_status = 0;
    }

#endif

    return rv;
}

VpiRet vpi_dec_h264_picture_consumed(VpiDecInst inst, struct DecPicturePpu pic)
{
    H264DecPicture hpic;
    uint32_t i;
    enum DecRet rv;

    memset(&hpic, 0, sizeof(H264DecPicture));
    /* TODO update chroma luma/chroma base */
    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
#ifndef NEW_MEM_ALLOC
        hpic.pictures[i].output_picture = pic.pictures[i].luma.virtual_address;
#endif
        hpic.pictures[i].output_picture_bus_address =
            pic.pictures[i].luma.bus_address;
    }
    hpic.is_idr_picture[0] =
        pic.pictures[0].picture_info.pic_coding_type == DEC_PIC_TYPE_I;
    rv = H264DecPictureConsumed(inst, &hpic);
    if (rv) {
        VPILOGD("H264DecPictureConsumed ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

VpiRet vpi_dec_h264_end_of_stream(VpiDecInst inst)
{
    enum DecRet rv;

    rv = H264DecEndOfStream(inst, 1);
    if (rv) {
        VPILOGD("H264DecEndOfStream ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

void vpi_dec_h264_release(VpiDecInst inst)
{
    H264DecRelease(inst);
}

VpiRet vpi_dec_h264_use_extra_frm_buffers(const VpiDecInst inst, uint32_t num)
{
    enum DecRet rv;

    rv = H264DecUseExtraFrmBuffers(inst, num);
    if (rv) {
        VPILOGD("H264DecUseExtraFrmBuffers ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

#ifdef USE_EXTERNAL_BUFFER
VpiRet vpi_dec_h264_get_buffer_info(VpiDecInst inst,
                                    struct DecBufferInfo *buf_info)
{
    H264DecBufferInfo hbuf;
    enum DecRet rv;

    rv                      = H264DecGetBufferInfo(inst, &hbuf);
    buf_info->buf_to_free   = hbuf.buf_to_free;
    buf_info->next_buf_size = hbuf.next_buf_size;
    buf_info->buf_num       = hbuf.buf_num;
#ifdef ASIC_TRACE_SUPPORT
    buf_info->is_frame_buffer = 1;
#endif
    if (rv) {
        VPILOGD("H264DecGetBufferInfo ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

VpiRet vpi_dec_h264_add_buffer(VpiDecInst inst, struct DWLLinearMem *buf)
{
    enum DecRet rv;

    rv = H264DecAddBuffer(inst, buf);
    if (rv) {
        VPILOGD("H264DecAddBuffer ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}
#endif

static VpiRet h264_picture_consumed_noDWL(VpiDecInst inst,
                                          struct DecPicturePpu pic)
{
    enum DecRet rv;
    H264DecPicture hpic;
    uint32_t i;

    memset(&hpic, 0, sizeof(H264DecPicture));
    /* TODO update chroma luma/chroma base */
    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
#ifndef NEW_MEM_ALLOC
        hpic.pictures[i].output_picture = pic.pictures[i].luma.virtual_address;
#endif
        hpic.pictures[i].output_picture_bus_address =
            pic.pictures[i].luma.bus_address;
    }
    hpic.is_idr_picture[0] =
        pic.pictures[0].picture_info.pic_coding_type == DEC_PIC_TYPE_I;

    rv = H264DecPictureConsumed(inst, &hpic);
    if (rv) {
        VPILOGD("H264DecPictureConsumed ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    return VPI_SUCCESS;
}

static void vpi_decode_h264_picture_consume(VpiDecCtx *vpi_ctx, void *data)
{
    struct DecPicturePpu pic = *((struct DecPicturePpu *)data);

    pthread_mutex_lock(&vpi_ctx->dec_thread_mutex);
    del_dec_pic_wait_consume_list(vpi_ctx, (uint8_t *)data);
    h264_picture_consumed_noDWL(vpi_ctx->dec_inst, pic);
    free(data);
    if (vpi_ctx->waiting_for_dpb == 1) {
        pthread_cond_signal(&vpi_ctx->dec_thread_cond);
        vpi_ctx->waiting_for_dpb = 0;
    }
    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
}

static void vpi_h264_release_ext_buffers(VpiDecCtx *vpi_ctx)
{
    int i;

    pthread_mutex_lock(&vpi_ctx->ext_buffer_ctrl);
    for (i = 0; i < vpi_ctx->num_buffers; i++) {
        VPILOGD("Freeing buffer %p\n",
                (void *)vpi_ctx->ext_buffers[i].virtual_address);
        if (vpi_ctx->pp_enabled) {
            DWLFreeLinear(vpi_ctx->dwl_inst, &vpi_ctx->ext_buffers[i]);
        } else {
            DWLFreeRefFrm(vpi_ctx->dwl_inst, &vpi_ctx->ext_buffers[i]);
        }
        DWLmemset(&vpi_ctx->ext_buffers[i], 0, sizeof(vpi_ctx->ext_buffers[i]));
    }
    pthread_mutex_unlock(&vpi_ctx->ext_buffer_ctrl);
}

VpiRet vpi_decode_h264_init(VpiDecCtx *vpi_ctx)
{
    H264DecApiVersion dec_api;
    H264DecBuild dec_build;
    uint32_t n_cores;
    uint32_t major_version, minor_version, prod_id;
    H264DecMCConfig mc_init_cfg = { 0 };
    enum DecDpbFlags flags      = 0;
    enum DecRet rv;

    /* Print API version number */
    dec_api   = H264DecGetAPIVersion();
    dec_build = H264DecGetBuild((void *)vpi_ctx->dwl_inst);
    n_cores   = H264DecMCGetCoreCount((void *)vpi_ctx->dwl_inst);
    VPILOGD("\nX170 H.264 Decoder API v%d.%d - \
               SW build: %d.%d - HW build: %x, %d cores\n\n",
            dec_api.major, dec_api.minor, dec_build.sw_build >> 16,
            dec_build.sw_build & 0xFFFF, dec_build.hw_build, n_cores);
    prod_id       = dec_build.hw_build >> 16;
    major_version = (dec_build.hw_build & 0xF000) >> 12;
    minor_version = (dec_build.hw_build & 0x0FF0) >> 4;
    if (prod_id == 0x6731) {
        vpi_ctx->max_strm_len = DEC_X170_MAX_STREAM;
    } else {
        vpi_ctx->max_strm_len = DEC_X170_MAX_STREAM_VC8000D;
    }
    /* number of stream buffers to allocate */
    vpi_ctx->allocated_buffers = n_cores + 1;

    if (vpi_ctx->tiled_output) {
        flags |= DEC_REF_FRM_TILED_DEFAULT;
    }
    if (vpi_ctx->dpb_mode == DEC_DPB_INTERLACED_FIELD) {
        flags |= DEC_DPB_ALLOW_FIELD_ORDERING;
    }
    vpi_ctx->enable_mc    = 1;
    mc_init_cfg.mc_enable = 1;

    rv = H264DecInit(&vpi_ctx->dec_inst, vpi_ctx->dwl_inst, DEC_NORMAL, 0,
                     DEC_EC_FAST_FREEZE, 0, flags, 0, 0, 0, &mc_init_cfg);
    if (rv) {
        VPILOGD("H264DecInit ret %s\n", dec_ret_string(rv));
        return VPI_ERR_UNKNOWN;
    }
    /* configure decoder to decode both views of MVC stereo high streams  */
    if (vpi_ctx->enable_mvc) {
        H264DecSetMvc(vpi_ctx->dec_inst);
    }

    return VPI_SUCCESS;
}

int vpi_decode_h264_put_packet(VpiDecCtx *vpi_ctx, void *indata)
{
    VpePacket *vpi_packet = (VpePacket *)indata;
    int idx;
    int i;

    pthread_mutex_lock(&vpi_ctx->dec_thread_mutex);
    if (vpi_packet->size == 0) {
        if (vpi_ctx->eos_received) {
            pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
            return 0;
        }
        VPILOGD("received EOS\n");
    }

    idx = vpi_ctx->stream_mem_index;
    if (vpi_ctx->strm_buf_list[idx]->mem_idx != 0xFFFFFFFF) {
        VPILOGD("No empty stream memory\n");
        pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
        return 0;
    }

    vpi_ctx->strm_buf_list[idx]->mem_idx   = vpi_ctx->stream_mem_index;
    vpi_ctx->strm_buf_list[idx]->item_size = vpi_packet->size;
    vpi_ctx->strm_buf_list[idx]->item      = (void *)vpi_packet->data;
    vpi_ctx->strm_buf_list[idx]->opaque    = vpi_packet->opaque;
    vpi_dec_buf_list_add(&vpi_ctx->strm_buf_head, vpi_ctx->strm_buf_list[idx]);

    for (i = 0; i < MAX_STRM_BUFFERS; i++) {
        if (vpi_ctx->time_stamp_info[i].used == 0) {
            vpi_ctx->time_stamp_info[i].pts = vpi_packet->pts;
            vpi_ctx->time_stamp_info[i].pkt_dts = vpi_packet->pkt_dts;
            vpi_ctx->time_stamp_info[i].decode_id = vpi_ctx->got_package_number;
            vpi_ctx->time_stamp_info[i].used = 1;
            break;
        }
    }
    vpi_ctx->got_package_number++;
    vpi_ctx->stream_mem_used[idx] = 1;

    vpi_ctx->stream_mem_index++;
    if (vpi_ctx->stream_mem_index == vpi_ctx->allocated_buffers) {
        vpi_ctx->stream_mem_index = 0;
    }


    if (vpi_packet->size == 0) {
        vpi_ctx->eos_received = 1;
    }
    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
    return vpi_packet->size;
}

int vpi_decode_h264_get_frame(VpiDecCtx *vpi_ctx, void *outdata)
{
    VpeFrame *out_frame = (VpeFrame *)outdata;
    VpeFrame *vpi_frame = NULL;
    int i;
    int ret;

    pthread_mutex_lock(&vpi_ctx->dec_thread_mutex);
    if (NULL == vpi_ctx->frame_buf_head) {
        if (vpi_ctx->last_pic_flag == 1) {
            ret = 2;
        } else {
            ret = 0;
        }
        pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
        return ret;
    }

    vpi_frame             = (VpeFrame *)vpi_ctx->frame_buf_head->item;
    vpi_frame->src_width  = out_frame->src_width;
    vpi_frame->src_height = out_frame->src_height;
    vpi_frame->task_id    = out_frame->task_id;
    memcpy(out_frame, vpi_frame, sizeof(VpeFrame));
    VPILOGD("src_width %d, src_height %d\n", out_frame->src_width,
            out_frame->src_height);
    for (i = 0; i < vpi_ctx->num_buffers; i++) {
        if (vpi_ctx->frame_buf_head->mem_idx
            == vpi_ctx->frame_buf_list[i]->mem_idx) {
            vpi_ctx->frame_buf_list[i]->mem_idx = 0xFFFFFFFF;
        }
    }
    vpi_ctx->frame_buf_head =
        vpi_dec_buf_list_delete(vpi_ctx->frame_buf_head);
    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);

    return 1;
}

int vpi_decode_h264_get_used_strm_mem(VpiDecCtx *vpi_ctx, void *mem)
{
    uint64_t *rls_mem = NULL;

    rls_mem = (uint64_t *)mem;
    if (vpi_ctx->rls_strm_buf_head) {
        *rls_mem = (uint64_t)vpi_ctx->rls_strm_buf_head->opaque;
        vpi_ctx->rls_strm_buf_head =
            vpi_dec_buf_list_delete(vpi_ctx->rls_strm_buf_head);
        return 0;
    } else {
        *rls_mem = 0;
        return -1;
    }
}

static int vpi_decode_h264_frame_decoding(VpiDecCtx *vpi_ctx)
{
    enum DecRet ret;
    VpiRet vpi_ret;
    int i;

    do {
        vpi_ctx->h264_dec_input.pic_id = vpi_ctx->pic_decode_number;
        ret = H264DecDecode(vpi_ctx->dec_inst, &vpi_ctx->h264_dec_input,
                            &vpi_ctx->h264_dec_output);
        print_decode_return(ret);
        switch (ret) {
        case DEC_STREAM_NOT_SUPPORTED:
            VPILOGE("ERROR: UNSUPPORTED STREAM!\n");
            return -1;
        case DEC_HDRS_RDY:
#ifdef DPB_REALLOC_DISABLE
            if (vpi_ctx->hdrs_rdy) {
                VPILOGD("Decoding ended, flush the DPB\n");
                /* the err_exit of stream is not reached yet */
                H264DecEndOfStream(vpi_ctx->dec_inst, 0);
            }
#endif
            vpi_ctx->hdrs_rdy = 1;

            START_SW_PERFORMANCE;
            vpi_ret =
                vpi_ctx->vpi_dec_wrapper.get_info(vpi_ctx->dec_inst,
                                                  &vpi_ctx->sequence_info);
            END_SW_PERFORMANCE;
            if (vpi_ret != VPI_SUCCESS) {
                VPILOGE("ERROR in getting stream info!\n");
                return -1;
            }
#if 1
            VPILOGD("Width %d Height %d\n", vpi_ctx->sequence_info.pic_width,
                    vpi_ctx->sequence_info.pic_height);

            VPILOGD("Cropping params: (%d, %d) %dx%d\n",
                    vpi_ctx->sequence_info.crop_params.crop_left_offset,
                    vpi_ctx->sequence_info.crop_params.crop_top_offset,
                    vpi_ctx->sequence_info.crop_params.crop_out_width,
                    vpi_ctx->sequence_info.crop_params.crop_out_height);

            VPILOGD("MonoChrome = %d\n", vpi_ctx->sequence_info.is_mono_chrome);
            VPILOGD("Interlaced = %d\n", vpi_ctx->sequence_info.is_interlaced);
            VPILOGD("num_of_ref_frames = %d\n",
                    vpi_ctx->sequence_info.num_of_ref_frames);

#endif
#ifdef ALWAYS_OUTPUT_REF
            H264DecUseExtraFrmBuffers(vpi_ctx->dec_inst,
                                      vpi_ctx->use_extra_buffers_num);
#endif
            vpi_dec_cfg_by_seqeuence_info(vpi_ctx);
            vpi_ret =
                vpi_ctx->vpi_dec_wrapper.set_info(vpi_ctx->dec_inst,
                                                  vpi_ctx->vpi_dec_config,
                                                  &vpi_ctx->sequence_info);

            if (vpi_ret != VPI_SUCCESS) {
                VPILOGE("Invalid pp parameters\n");
                return -1;
            }
            break;
        case DEC_ADVANCED_TOOLS:
            if (vpi_ctx->enable_mc) {
                /* ASO/FMO detected and not supported in multicore mode */
                VPILOGD("ASO/FMO detected in multicore, dec will stop\n");
                return -1;
            }

            /* ASO/STREAM ERROR was noticed in the stream.
                 * The decoder has to reallocate resources */
            /* we should have some data left used to indicate that
                 * decoding needs to finalized prior to corrupting next pic */
            assert(vpi_ctx->h264_dec_output.data_left);
#ifdef LOW_LATENCY_FRAME_MODE
            if (vpi_ctx->low_latency) {
                vpi_ctx->process_end_flag = 1;
                //pic_decoded = 1;
                // sem_post(&next_frame_start_sem);
                // wait_for_task_completion(task);
                vpi_ctx->h264_dec_input.data_len = 0;
                //task_has_freed = 1;
            }
#endif
            break;
        case DEC_PENDING_FLUSH:
        case DEC_PIC_DECODED:
            /* lint -esym(644,tmp_image,pic_size) variable initialized at
                 * DEC_HDRS_RDY_BUFF_NOT_EMPTY case */

            if (ret == DEC_PIC_DECODED) {
                /* If enough pictures decoded -> force decoding to err_exit
                     * by setting that no more stream is available */
                if (vpi_ctx->pic_decode_number == vpi_ctx->max_num_pics) {
                    vpi_ctx->process_end_flag        = 1;
                    vpi_ctx->h264_dec_input.data_len = 0;
                }

                VPILOGD("DECODED PIC %d\n", vpi_ctx->pic_decode_number);
                /* Increment decoding number for every decoded picture */
                vpi_ctx->pic_decode_number++;
            }
            break;
        case DEC_STRM_PROCESSED:
        case DEC_BUF_EMPTY:
        case DEC_NONREF_PIC_SKIPPED:
        case DEC_STRM_ERROR:
            /* Used to indicate that picture decoding
                 * needs to finalized prior to corrupting next picture
                 * pic_rdy = 0; */
            break;
        case DEC_NO_DECODING_BUFFER:
            // No DPB buffer, return to wait until new DPB buffer ready
            return 1;
        case DEC_WAITING_FOR_BUFFER:
            VPILOGD("Waiting for frame buffers\n");
            struct DWLLinearMem mem, mem_ext;
            int alloc_buffer_num;
            enum DecRet dec_ret;

            dec_ret =
                H264DecGetBufferInfo(vpi_ctx->dec_inst, &vpi_ctx->h264_hbuf);
            VPILOGD("H264DecGetBufferInfo ret %d\n", dec_ret);
            VPILOGD("buf_to_free %p, next_buf_size %d, buf_num %d\n",
                    (void *)vpi_ctx->h264_hbuf.buf_to_free.virtual_address,
                    vpi_ctx->h264_hbuf.next_buf_size,
                    vpi_ctx->h264_hbuf.buf_num);
            if (vpi_ctx->h264_hbuf.buf_to_free.virtual_address != NULL &&
                vpi_ctx->res_changed) {
                vpi_ctx->add_extra_flag = 0;
                vpi_h264_release_ext_buffers(vpi_ctx);
                vpi_ctx->buffer_release_flag = 1;
                vpi_ctx->num_buffers         = 0;
                vpi_ctx->res_changed         = 0;
            }
            vpi_ctx->buffer_size = vpi_ctx->h264_hbuf.next_buf_size;
            if (vpi_ctx->buffer_release_flag &&
                vpi_ctx->h264_hbuf.next_buf_size) {
                /* Only add minimum required buffers at first. */
#ifdef ALWAYS_OUTPUT_REF
                alloc_buffer_num = vpi_ctx->h264_hbuf.buf_num;
#else
                alloc_buffer_num =
                    vpi_ctx->h264_hbuf.buf_num + vpi_ctx->use_extra_buffers_num;
#endif
                for (i = 0; i < alloc_buffer_num; i++) {
                    enum DecRet dwl_ret;
                    mem.mem_type = DWL_MEM_TYPE_DPB;
                    if (vpi_ctx->pp_enabled) {
                        dwl_ret =
                            DWLMallocLinear(vpi_ctx->dwl_inst,
                                            vpi_ctx->h264_hbuf.next_buf_size,
                                            &mem);
                    } else {
                        dwl_ret =
                            DWLMallocRefFrm(vpi_ctx->dwl_inst,
                                            vpi_ctx->h264_hbuf.next_buf_size,
                                            &mem);
                    }
                    if (dwl_ret) {
                        vpi_ctx->num_buffers = i;
                        return -1;
                    }
                    mem_ext = mem;
                    dec_ret = H264DecAddBuffer(vpi_ctx->dec_inst, &mem);
                    if (dec_ret != DEC_OK &&
                        dec_ret != DEC_WAITING_FOR_BUFFER) {
                        if (vpi_ctx->pp_enabled) {
                            DWLFreeLinear(vpi_ctx->dwl_inst, &mem);
                        } else {
                            DWLFreeRefFrm(vpi_ctx->dwl_inst, &mem);
                        }
                    } else {
                        vpi_ctx->ext_buffers[i] = mem_ext;
                    }
                    vpi_ctx->frame_buf_list[i] = malloc(sizeof(BufLink));
                    if (NULL == vpi_ctx->frame_buf_list[i]) {
                        VPILOGE("UNABLE TO ALLOCATE FRAME BUFFER LIST\n");
                        return VPI_ERR_MALLOC;
                    }
                    vpi_ctx->frame_buf_list[i]->item    = (void *)malloc(sizeof(VpeFrame));
                    vpi_ctx->frame_buf_list[i]->mem_idx = 0xFFFFFFFF;
                    vpi_ctx->frame_buf_list[i]->next    = NULL;
                }
                vpi_ctx->frame_buf_head     = NULL;
                /* Extra buffers are allowed
                     * when minimum required buffers have been added.*/
                vpi_ctx->num_buffers    = alloc_buffer_num;
                vpi_ctx->add_extra_flag = 1;
            }
            break;
        case DEC_OK:
            /* nothing to do, just call again */
            break;
        case DEC_HW_TIMEOUT:
            VPILOGE("HW Timeout\n");
            return -1;
        case DEC_ABORTED:
            VPILOGE("H264 decoder is aborted: %d\n", ret);
            H264DecAbortAfter(vpi_ctx->dec_inst);
            break;
        case DEC_SYSTEM_ERROR:
            VPILOGE("Fatal system error\n");
            return -1;
        default:
            VPILOGE("FATAL ERROR: %d\n", ret);
            return -1;
        }
    } while (vpi_ctx->h264_dec_output.data_left);

    return 0;
}

int vpi_decode_h264_dec_process(VpiDecCtx *vpi_ctx)
{
    int ret;
    int i;
    VpeFrame *vpi_frame        = NULL;
    VpePacket vpi_packet       = {0};

    pthread_mutex_lock(&vpi_ctx->dec_thread_mutex);
    if (NULL == vpi_ctx->strm_buf_head && 0 == vpi_ctx->eos_received) {
        VPILOGE("stream buffer empty\n");
        pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
        return 0;
    }
    if (vpi_ctx->eos_received == 1) {
        if (vpi_ctx->strm_buf_head && vpi_ctx->strm_buf_head->item_size == 0) {
            // The last frame packet
            H264DecEndOfStream(vpi_ctx->dec_inst, 1);
            vpi_ctx->stream_mem_used[vpi_ctx->strm_buf_head->mem_idx] = 0;
            for (i = 0; i < vpi_ctx->allocated_buffers; i++) {
                if (vpi_ctx->strm_buf_list[i]->mem_idx ==
                    vpi_ctx->strm_buf_head->mem_idx) {
                    vpi_ctx->strm_buf_list[i]->mem_idx = 0xFFFFFFFF;
                    break;
                }
            }
            vpi_ctx->strm_buf_head =
                vpi_dec_buf_list_delete(vpi_ctx->strm_buf_head);
            vpi_ctx->eos_handled = 1;
        }
        if (vpi_ctx->eos_handled == 1) {
            ret = vpi_dec_h264_next_picture(vpi_ctx->dec_inst, &vpi_ctx->pic);
            VPILOGD("vpi_dec_h264_next_picture ret %d\n", ret);
            if (ret == DEC_PIC_RDY) {
                for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
                    VPILOGD("DEC_PIC_RDY pic %d -> %d x %d\n", i,
                        vpi_ctx->pic.pictures[i].pic_width,
                        vpi_ctx->pic.pictures[i].pic_height);
                }
                for (i = 0; i < vpi_ctx->num_buffers; i++) {
                    if (vpi_ctx->frame_buf_list[i]->mem_idx == 0xFFFFFFFF) {
                        vpi_ctx->frame_buf_list[i]->mem_idx = i;
                        break;
                    }
                }
                if (i == vpi_ctx->num_buffers) {
                    // This case should not happen
                    VPILOGE("All frame buffer used out\n");
                    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
                    return -1;
                }
                vpi_frame = (VpeFrame *)vpi_ctx->frame_buf_list[i]->item;
                memset(vpi_frame, 0, sizeof(VpeFrame));
                vpi_ctx->pic_rdy = 1;
                vpi_dec_output_frame(vpi_ctx, vpi_frame, &vpi_ctx->pic);
                vpi_dec_buf_list_add(&vpi_ctx->frame_buf_head,
                    vpi_ctx->frame_buf_list[i]);
                vpi_ctx->pic_display_number++;
                VPILOGD("******** %d got frame :data[0]=%p,data[1]=%p\n",
                    vpi_ctx->pic_display_number, vpi_frame->data[0],
                    vpi_frame->data[1]);
                vpi_ctx->pic_rdy = 0;
            } else if (ret == DEC_END_OF_STREAM) {
                vpi_ctx->last_pic_flag = 1;
                VPILOGD("END-OF-STREAM received in output thread\n");
                vpi_ctx->add_buffer_thread_run = 0;
                vpi_ctx->dec_thread_finish     = 1;

            }
            pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
            return 0;
        }
    }

    vpi_packet.data = (uint8_t *)vpi_ctx->strm_buf_head->item;
    vpi_packet.size = vpi_ctx->strm_buf_head->item_size;
    vpi_send_packet_to_decode_buffer(vpi_ctx, &vpi_packet,
                        vpi_ctx->stream_mem[vpi_ctx->strm_buf_head->mem_idx]);
    vpi_ctx->stream_mem[vpi_ctx->strm_buf_head->mem_idx].virtual_address =
        (uint32_t *)vpi_packet.data;
    vpi_ctx->h264_dec_input.stream =
        (uint8_t *)vpi_ctx->stream_mem[vpi_ctx->strm_buf_head->mem_idx]
            .virtual_address;
    vpi_ctx->h264_dec_input.stream_bus_address =
        vpi_ctx->stream_mem[vpi_ctx->strm_buf_head->mem_idx].bus_address;
    vpi_ctx->h264_dec_input.data_len = vpi_ctx->strm_buf_head->item_size;
    VPILOGD("set to stream_mem_index %d\n", vpi_ctx->strm_buf_head->mem_idx);

    do {
        ret = vpi_decode_h264_frame_decoding(vpi_ctx);

        if (ret == 1) {
            // waiting for release dpb buffer
            vpi_ctx->waiting_for_dpb = 1;
            pthread_cond_wait(&vpi_ctx->dec_thread_cond,
                              &vpi_ctx->dec_thread_mutex);
        } else {
            break;
        }
    } while(vpi_ctx->h264_dec_output.data_left);
    VPILOGD("decoding stream size %d\n", vpi_ctx->h264_dec_input.data_len);

    if (vpi_ctx->h264_dec_output.data_left == 0) {
        int idx = vpi_ctx->rls_mem_index;

        vpi_ctx->stream_mem_used[vpi_ctx->strm_buf_head->mem_idx] = 0;
        VPILOGD("release stream_mem_index %d\n", vpi_ctx->strm_buf_head->mem_idx);
        vpi_ctx->rls_strm_buf_list[idx]->mem_idx = vpi_ctx->rls_mem_index;
        vpi_ctx->rls_strm_buf_list[idx]->item    = vpi_ctx->strm_buf_head->item;
        vpi_ctx->rls_strm_buf_list[idx]->opaque  = vpi_ctx->strm_buf_head->opaque;
        vpi_dec_buf_list_add(&vpi_ctx->rls_strm_buf_head,
                             vpi_ctx->rls_strm_buf_list[idx]);
        vpi_ctx->rls_mem_index++;
        if (vpi_ctx->rls_mem_index == 32) {
            vpi_ctx->rls_mem_index = 0;
        }
        for (i = 0; i < vpi_ctx->allocated_buffers; i++) {
            if (vpi_ctx->strm_buf_list[i]->mem_idx ==
                vpi_ctx->strm_buf_head->mem_idx) {
                vpi_ctx->strm_buf_list[i]->mem_idx = 0xFFFFFFFF;
                break;
            }
        }
        vpi_ctx->strm_buf_head =
                vpi_dec_buf_list_delete(vpi_ctx->strm_buf_head);
        ret = 0;
    } else {
        ret = -1;
    }

    ret = vpi_dec_h264_next_picture(vpi_ctx->dec_inst, &vpi_ctx->pic);
    VPILOGD("h264_next_picture ret = %d\n", ret);
    if(ret) {
        VPILOGD("h264_next_picture: %d\n", ret);
        print_decode_return(ret);
    }

    if(ret == DEC_PIC_RDY) {
        for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
            VPILOGD("DEC_PIC_RDY pic %d -> %d x %d\n",
                    i, vpi_ctx->pic.pictures[i].pic_width,
                    vpi_ctx->pic.pictures[i].pic_height);
        }

        for (i = 0; i < vpi_ctx->num_buffers; i++) {
            if (vpi_ctx->frame_buf_list[i]->mem_idx == 0xFFFFFFFF) {
                vpi_ctx->frame_buf_list[i]->mem_idx = i;
                break;
            }
        }
        if (i == vpi_ctx->num_buffers) {
            // This case should not happen
            VPILOGE("All frame buffer used out\n");
            pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
            return -1;
        }
        vpi_frame = (VpeFrame *)vpi_ctx->frame_buf_list[i]->item;
        vpi_ctx->pic_rdy = 1;
        memset(vpi_frame, 0, sizeof(VpeFrame));
        vpi_dec_output_frame(vpi_ctx, vpi_frame, &vpi_ctx->pic);
        vpi_dec_buf_list_add(&vpi_ctx->frame_buf_head,
            vpi_ctx->frame_buf_list[i]);
        vpi_ctx->pic_display_number++;
        VPILOGD("******** %d got frame :data[0]=%p,data[1]=%p\n",
            vpi_ctx->pic_display_number,
            vpi_frame->data[0], vpi_frame->data[1]);
    } else if(ret == DEC_END_OF_STREAM) {
        vpi_ctx->last_pic_flag = 1;
        VPILOGD("END-OF-STREAM received\n");
        vpi_ctx->add_buffer_thread_run = 0;
        vpi_ctx->dec_thread_finish     = 1;
    }
    pthread_mutex_unlock(&vpi_ctx->dec_thread_mutex);
    return 1;
}

int vpi_decode_h264_dec_frame(VpiDecCtx *vpi_ctx, void *indata, void *outdata)
{
    VpePacket *vpi_packet = (VpePacket *)indata;
    VpeFrame *vpi_frame   = (VpeFrame *)outdata;
    int i;
    enum DecRet ret;
    VpiRet vpi_ret;

    vpi_ctx->pic_rdy = 0;

    if (vpi_packet->size == 0) {
        VPILOGD("received EOS\n");
        H264DecEndOfStream(vpi_ctx->dec_inst, 1);

        ret = vpi_dec_h264_next_picture(vpi_ctx->dec_inst, &vpi_ctx->pic);
        VPILOGD("vpi_dec_h264_next_picture ret %d\n", ret);
        if (ret == DEC_PIC_RDY) {
            for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
                VPILOGD("DEC_PIC_RDY pic %d -> %d x %d\n", i,
                        vpi_ctx->pic.pictures[i].pic_width,
                        vpi_ctx->pic.pictures[i].pic_height);
            }
            vpi_ctx->pic_rdy = 1;
            vpi_dec_output_frame(vpi_ctx, vpi_frame, &vpi_ctx->pic);

            vpi_ctx->pic_display_number++;
            VPILOGD("******** %d got frame :data[0]=%p,data[1]=%p\n",
                    vpi_ctx->pic_display_number, vpi_frame->data[0],
                    vpi_frame->data[1]);
            vpi_ctx->pic_rdy = 0;
            return 1;
        } else if (ret == DEC_END_OF_STREAM) {
            vpi_ctx->last_pic_flag = 1;
            VPILOGD("END-OF-STREAM received in output thread\n");
            vpi_ctx->add_buffer_thread_run = 0;
            return 0;
        }
        return 0;
    }

    vpi_ctx->got_package_number++;
    vpi_send_packet_to_decode_buffer(vpi_ctx, vpi_packet,
                                     vpi_ctx
                                         ->stream_mem[vpi_ctx->stream_mem_index]);
    vpi_ctx->h264_dec_input.stream =
        (uint8_t *)vpi_ctx->stream_mem[vpi_ctx->stream_mem_index]
            .virtual_address;
    vpi_ctx->h264_dec_input.stream_bus_address =
        vpi_ctx->stream_mem[vpi_ctx->stream_mem_index].bus_address;
    vpi_ctx->h264_dec_input.data_len = vpi_packet->size;

    do {
        vpi_ctx->h264_dec_input.pic_id = vpi_ctx->pic_decode_number;
        VPILOGD("input.data_len = %d\n", vpi_ctx->h264_dec_input.data_len);
        ret = H264DecDecode(vpi_ctx->dec_inst, &vpi_ctx->h264_dec_input,
                            &vpi_ctx->h264_dec_output);
        print_decode_return(ret);
        switch (ret) {
        case DEC_STREAM_NOT_SUPPORTED:
            VPILOGE("ERROR: UNSUPPORTED STREAM!\n");
            return -1;
        case DEC_HDRS_RDY:
#ifdef DPB_REALLOC_DISABLE
            if (vpi_ctx->hdrs_rdy) {
                VPILOGD("Decoding ended, flush the DPB\n");
                /* the err_exit of stream is not reached yet */
                H264DecEndOfStream(vpi_ctx->dec_inst, 0);
            }
#endif
            vpi_ctx->hdrs_rdy = 1;

            START_SW_PERFORMANCE;
            vpi_ret =
                vpi_ctx->vpi_dec_wrapper.get_info(vpi_ctx->dec_inst,
                                                  &vpi_ctx->sequence_info);
            END_SW_PERFORMANCE;
            if (vpi_ret != VPI_SUCCESS) {
                VPILOGE("ERROR in getting stream info!\n");
                return -1;
            }
#if 1
            VPILOGD("Width %d Height %d\n", vpi_ctx->sequence_info.pic_width,
                    vpi_ctx->sequence_info.pic_height);

            VPILOGD("Cropping params: (%d, %d) %dx%d\n",
                    vpi_ctx->sequence_info.crop_params.crop_left_offset,
                    vpi_ctx->sequence_info.crop_params.crop_top_offset,
                    vpi_ctx->sequence_info.crop_params.crop_out_width,
                    vpi_ctx->sequence_info.crop_params.crop_out_height);

            VPILOGD("MonoChrome = %d\n", vpi_ctx->sequence_info.is_mono_chrome);
            VPILOGD("Interlaced = %d\n", vpi_ctx->sequence_info.is_interlaced);
            VPILOGD("num_of_ref_frames = %d\n",
                    vpi_ctx->sequence_info.num_of_ref_frames);

#endif
#ifdef ALWAYS_OUTPUT_REF
            H264DecUseExtraFrmBuffers(vpi_ctx->dec_inst,
                                      vpi_ctx->use_extra_buffers_num);
#endif
            vpi_dec_cfg_by_seqeuence_info(vpi_ctx);
            vpi_ret =
                vpi_ctx->vpi_dec_wrapper.set_info(vpi_ctx->dec_inst,
                                                  vpi_ctx->vpi_dec_config,
                                                  &vpi_ctx->sequence_info);

            if (vpi_ret != VPI_SUCCESS) {
                VPILOGE("Invalid pp parameters\n");
                return -1;
            }
            break;
        case DEC_ADVANCED_TOOLS:
            if (vpi_ctx->enable_mc) {
                /* ASO/FMO detected and not supported in multicore mode */
                VPILOGD("ASO/FMO detected in multicore, dec will stop\n");
                return -1;
            }

            /* ASO/STREAM ERROR was noticed in the stream.
                 * The decoder has to reallocate resources */
            /* we should have some data left Used to indicate that
                 * decoding needs to finalized prior to corrupting next pic */
            assert(vpi_ctx->h264_dec_output.data_left);
#ifdef LOW_LATENCY_FRAME_MODE
            if (vpi_ctx->low_latency) {
                vpi_ctx->process_end_flag = 1;
                //pic_decoded = 1;
                // sem_post(&next_frame_start_sem);
                // wait_for_task_completion(task);
                vpi_ctx->h264_dec_input.data_len = 0;
                //task_has_freed = 1;
            }
#endif
            break;
        case DEC_PENDING_FLUSH:
        case DEC_PIC_DECODED:
            /*lint -esym(644,tmp_image,pic_size) variable initialized at
                 * DEC_HDRS_RDY_BUFF_NOT_EMPTY case */

            if (ret == DEC_PIC_DECODED) {
                /* If enough pictures decoded -> force decoding to err_exit
                     * by setting that no more stream is available */
                if (vpi_ctx->pic_decode_number == vpi_ctx->max_num_pics) {
                    vpi_ctx->process_end_flag        = 1;
                    vpi_ctx->h264_dec_input.data_len = 0;
                }

                VPILOGD("DECODED PIC %d\n", vpi_ctx->pic_decode_number);
                /* Increment decoding number for every decoded picture */
                vpi_ctx->pic_decode_number++;
            }
            break;
        case DEC_STRM_PROCESSED:
        case DEC_BUF_EMPTY:
        case DEC_NONREF_PIC_SKIPPED:
        case DEC_NO_DECODING_BUFFER:
        case DEC_STRM_ERROR:
            /* Used to indicate that picture decoding
                 * needs to finalized prior to corrupting next picture
                 * pic_rdy = 0; */
            break;
        case DEC_WAITING_FOR_BUFFER:
            VPILOGD("Waiting for frame buffers\n");
            struct DWLLinearMem mem, mem_ext;
            int alloc_buffer_num;
            enum DecRet dec_ret;

            dec_ret =
                H264DecGetBufferInfo(vpi_ctx->dec_inst, &vpi_ctx->h264_hbuf);
            VPILOGD("H264DecGetBufferInfo ret %d\n", dec_ret);
            VPILOGD("buf_to_free %p, next_buf_size %d, buf_num %d\n",
                    (void *)vpi_ctx->h264_hbuf.buf_to_free.virtual_address,
                    vpi_ctx->h264_hbuf.next_buf_size,
                    vpi_ctx->h264_hbuf.buf_num);
            if (vpi_ctx->h264_hbuf.buf_to_free.virtual_address != NULL &&
                vpi_ctx->res_changed) {
                vpi_ctx->add_extra_flag = 0;
                vpi_h264_release_ext_buffers(vpi_ctx);
                vpi_ctx->buffer_release_flag = 1;
                vpi_ctx->num_buffers         = 0;
                vpi_ctx->res_changed         = 0;
            }
            vpi_ctx->buffer_size = vpi_ctx->h264_hbuf.next_buf_size;
            if (vpi_ctx->buffer_release_flag &&
                vpi_ctx->h264_hbuf.next_buf_size) {
                /* Only add minimum required buffers at first. */
#ifdef ALWAYS_OUTPUT_REF
                alloc_buffer_num = vpi_ctx->h264_hbuf.buf_num;
#else
                alloc_buffer_num =
                    vpi_ctx->h264_hbuf.buf_num + vpi_ctx->use_extra_buffers_num;
#endif
                for (i = 0; i < alloc_buffer_num; i++) {
                    enum DecRet dwl_ret;
                    mem.mem_type = DWL_MEM_TYPE_DPB;
                    if (vpi_ctx->pp_enabled) {
                        dwl_ret =
                            DWLMallocLinear(vpi_ctx->dwl_inst,
                                            vpi_ctx->h264_hbuf.next_buf_size,
                                            &mem);
                    } else {
                        dwl_ret =
                            DWLMallocRefFrm(vpi_ctx->dwl_inst,
                                            vpi_ctx->h264_hbuf.next_buf_size,
                                            &mem);
                    }
                    if (dwl_ret) {
                        vpi_ctx->num_buffers = i;
                        return -1;
                    }
                    mem_ext = mem;
                    //memset(mem.virtual_address, 0x00,mem.size);
                    dec_ret = H264DecAddBuffer(vpi_ctx->dec_inst, &mem);
                    if (dec_ret != DEC_OK &&
                        dec_ret != DEC_WAITING_FOR_BUFFER) {
                        if (vpi_ctx->pp_enabled) {
                            DWLFreeLinear(vpi_ctx->dwl_inst, &mem);
                        } else {
                            DWLFreeRefFrm(vpi_ctx->dwl_inst, &mem);
                        }
                    } else {
                        vpi_ctx->ext_buffers[i] = mem_ext;
                    }
                }
                /* Extra buffers are allowed
                     * when minimum required buffers have been added.*/
                vpi_ctx->num_buffers    = alloc_buffer_num;
                vpi_ctx->add_extra_flag = 1;
            }
            break;
        case DEC_OK:
            /* nothing to do, just call again */
            break;
        case DEC_HW_TIMEOUT:
            VPILOGE("HW Timeout\n");
            return -1;
        case DEC_ABORTED:
            VPILOGE("H264 decoder is aborted: %d\n", ret);
            H264DecAbortAfter(vpi_ctx->dec_inst);
            break;
        case DEC_SYSTEM_ERROR:
            VPILOGE("Fatal system error\n");
            return -1;
        default:
            VPILOGE("FATAL ERROR: %d\n", ret);
            return -1;
        }
    } while (vpi_ctx->h264_dec_output.data_left);

    ret = vpi_dec_h264_next_picture(vpi_ctx->dec_inst, &vpi_ctx->pic);
    VPILOGD("h264_next_picture ret = %d\n", ret);
    if (ret) {
        VPILOGD("h264_next_picture: %d\n", ret);
        print_decode_return(ret);
    }

    vpi_ctx->stream_mem_index++;
    if (vpi_ctx->stream_mem_index == vpi_ctx->allocated_buffers) {
        vpi_ctx->stream_mem_index = 0;
    }

    VPILOGD("in h264_decode_frame return:vpi_packet->size = %d.........\n",
            vpi_packet->size);

    if (ret == DEC_PIC_RDY) {
        vpi_ctx->pic_rdy = 1;
        vpi_ctx->pts     = vpi_packet->pts;
        vpi_ctx->pkt_dts = vpi_packet->pkt_dts;
        vpi_dec_output_frame(vpi_ctx, vpi_frame, &vpi_ctx->pic);
        vpi_ctx->pic_display_number++;
        VPILOGD("******** %d got frame :data[0]=%p,data[1]=%p\n",
                vpi_ctx->pic_display_number, vpi_frame->data[0],
                vpi_frame->data[1]);
        return 1;
    } else if (ret == DEC_END_OF_STREAM) {
        vpi_ctx->last_pic_flag = 1;
        VPILOGD("END-OF-STREAM received\n");
        vpi_ctx->add_buffer_thread_run = 0;
        return 0;
    }
    return vpi_packet->size;
}

VpiRet vpi_decode_h264_control(VpiDecCtx *vpi_ctx, void *indata, void *outdata)
{
    VpeCtrlCmdParam *in_param = (VpeCtrlCmdParam *)indata;
    int *out_num = NULL;

    switch (in_param->cmd) {
    case VPE_CMD_DEC_PIC_CONSUME:
        vpi_decode_h264_picture_consume(vpi_ctx, in_param->data);
        break;
    case VPE_CMD_DEC_STRM_BUF_COUNT:
        out_num = (int*)outdata;
        *out_num = vpi_dec_get_stream_buffer_index(vpi_ctx, 0);
        break;
    case VPE_CMD_DEC_GET_USED_STRM_MEM:
        vpi_decode_h264_get_used_strm_mem(vpi_ctx, outdata);
        break;
    default:
        break;
    }

    return VPI_SUCCESS;
}

int vpi_decode_h264_close(VpiDecCtx *vpi_ctx)
{
    int i;
    int idx;

    while (vpi_ctx->strm_buf_head) {
        idx = vpi_ctx->rls_mem_index;
        vpi_ctx->rls_strm_buf_list[idx]->mem_idx = vpi_ctx->rls_mem_index;
        vpi_ctx->rls_strm_buf_list[idx]->item    = vpi_ctx->strm_buf_head->item;
        vpi_ctx->rls_strm_buf_list[idx]->opaque  =
            vpi_ctx->strm_buf_head->opaque;
        vpi_dec_buf_list_add(&vpi_ctx->rls_strm_buf_head,
                             vpi_ctx->rls_strm_buf_list[idx]);

        vpi_ctx->rls_mem_index++;
        if (vpi_ctx->rls_mem_index == 32) {
            vpi_ctx->rls_mem_index = 0;
        }

        vpi_ctx->strm_buf_head =
            vpi_dec_buf_list_delete(vpi_ctx->strm_buf_head);
    }

    for (i = 0; i < vpi_ctx->allocated_buffers; i++) {
        if (vpi_ctx->stream_mem[i].mem_type == DWL_MEM_TYPE_DPB) {
            vpi_ctx->stream_mem[i].virtual_address = NULL;
        }
        if (vpi_ctx->dec_inst) {
            DWLFreeLinear(vpi_ctx->dwl_inst, &vpi_ctx->stream_mem[i]);
        }
        free(vpi_ctx->strm_buf_list[i]);
    }

    for (i = 0; i < vpi_ctx->num_buffers; i++) {
        if (vpi_ctx->frame_buf_list[i]->item) {
            free(vpi_ctx->frame_buf_list[i]->item);
        }
        free(vpi_ctx->frame_buf_list[i]);
    }

    if (vpi_ctx->pic_display_number > 0) {
        vpi_dec_performance_report(vpi_ctx);
    }
    if (vpi_ctx->dec_inst) {
        H264DecRelease(vpi_ctx->dec_inst);
    }
    vpi_dec_release_ext_buffers(vpi_ctx);
    if (vpi_ctx->dwl_inst) {
        DWLRelease(vpi_ctx->dwl_inst);
    }
    VPILOGD("close finish\n");
    return 0;
}
