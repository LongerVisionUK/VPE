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

#ifndef __VPI_VIDEO_H26XENC_UTILS_H__
#define __VPI_VIDEO_H26XENC_UTILS_H__

#include "vpi_video_h26xenc.h"

FILE *open_file(char *name, char *mode);
void h26x_enc_write_strm(FILE *fout, u32 *strmbuf, u32 size, u32 endian);
i32 setup_roi_map_buffer(struct VPIH26xEncCfg *tb, VPEH26xEncOptions *options,
                         VCEncIn *p_enc_in, VCEncInst encoder);
/*input YUV format convertion for specific format*/
float get_pixel_width_inbyte(VCEncPictureType type);
FILE *format_customized_yuv(struct VPIH26xEncCfg *tb,
                            VPEH26xEncOptions *options, i32 *ret);
void change_cml_customized_format(VPEH26xEncOptions *options);
void change_to_customized_format(VPEH26xEncOptions *options,
                                 VCEncPreProcessingCfg *pre_proc_cfg);
/*read&parse input cfg files for different features*/
i32 read_config_files(struct VPIH26xEncCfg *tb, VPEH26xEncOptions *options);
i32 read_gmv(struct VPIH26xEncCfg *tb, VCEncIn *p_enc_in,
             VPEH26xEncOptions *options);
i32 parsing_smart_config(char *fname, VPEH26xEncOptions *options);
/*input YUV read*/
i32 read_picture(struct VPIH26xEncCfg *tb, u32 input_format, u32 src_img_size,
                 u32 src_width, u32 src_height);
u64 h26x_enc_next_picture(struct VPIH26xEncCfg *tb, int picture_cnt);
void get_aligned_pic_size_byformat(i32 type, u32 width, u32 height,
                                   u32 alignment, u32 *luma_size,
                                   u32 *chroma_size, u32 *picture_size);
u32 get_resolution(char *filename, i32 *p_width, i32 *p_height);
/*GOP pattern file parse*/
int init_gop_configs(int gop_size, VPEH26xEncOptions *options,
                     VCEncGopConfig *gop_cfg, u8 *gop_cfg_offset, bool b_pass2);
/*SEI information from cfg file*/
u8 *read_userdata(VCEncInst encoder, char *name);
/*adaptive gop decision*/
i32 adaptive_gop_decision(struct VPIH26xEncCfg *tb, VCEncIn *p_enc_in,
                          VCEncInst encoder, i32 *p_next_gop_size,
                          AdapGopCtr *agop);
i32 get_next_gop_size(struct VPIH26xEncCfg *tb, VCEncIn *p_enc_in,
                      VCEncInst encoder, i32 *p_next_gop_size,
                      AdapGopCtr *agop);
u32 setup_input_buffer(struct VPIH26xEncCfg *tb, VPEH26xEncOptions *options,
                       VCEncIn *p_enc_in);
void setup_output_buffer(struct VPIH26xEncCfg *tb, VCEncIn *p_enc_in);
void get_free_iobuffer(struct VPIH26xEncCfg *tb);
void init_slice_ctl(struct VPIH26xEncCfg *tb,
                    struct VPEH26xEncOptions *options);
void setup_slice_ctl(struct VPIH26xEncCfg *tb);
i32 change_format_for_FB(struct VPIH26xEncCfg *tb, VPEH26xEncOptions *options,
                         VCEncPreProcessingCfg *pre_proc_cfg);
#if defined(SUPPORT_DEC400) || defined(SUPPORT_TCACHE)
i32 read_table(struct VPIH26xEncCfg *tb, u32 lum_tbl_size, u32 ch_tbl_size);
#endif
void init_stream_segment_crl(struct VPIH26xEncCfg *tb,
                             struct VPEH26xEncOptions *options);
void write_strm_bufs(FILE *fout, VCEncStrmBufs *bufs, u32 offset, u32 size,
                     u32 endian);
void write_nals_bufs(FILE *fout, VCEncStrmBufs *bufs,
                     const u32 *p_nalu_size_buf, u32 num_nalus, u32 offset,
                     u32 hdr_size, u32 endian);
void get_stream_bufs(VCEncStrmBufs *bufs, struct VPIH26xEncCfg *tb,
                     VPEH26xEncOptions *options, bool encoding);
/* timer help*/
unsigned int utime_diff(struct timeval end, struct timeval start);

#endif /* __VPI_VIDEO_H26XENC_UTILS_H__ */
