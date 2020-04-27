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

#ifndef __VPE_TYPES_H__
#define __VPE_TYPES_H__

#include <stdint.h>

#define PIC_INDEX_MAX_NUMBER 5

#define VPE_TASK_LIVE        0
#define VPE_TASK_VOD         1

typedef void *VpeCtx;

typedef enum {
    VPE_CMD_BASE,

    VPE_CMD_DEC_PIC_CONSUME,
    VPE_CMD_DEC_STRM_BUF_COUNT,
    VPE_CMD_DEC_GET_USED_STRM_MEM,

    /*hw downloader command*/
    VPE_CMD_HWDW_FREE_BUF,
    VPE_CMD_HWDW_GET_INDEX,

    /*vp9 encoder command*/
    VPE_CMD_VP9ENC_SET_PENDDING_FRAMES_COUNT,
    VPE_CMD_VP9ENC_GET_NEXT_PIC,
    VPE_CMD_VP9ENC_GET_PIC_TOBE_FREE,

    /*pp command*/
    VPE_CMD_PP_CONFIG,
    VPE_CMD_PP_CONSUME,
} VpeCmd;

typedef enum VpePixFmt {
    VPE_FMT_YUV420P,
    VPE_FMT_NV12,
    VPE_FMT_NV21,
    VPE_FMT_YUV420P10LE,
    VPE_FMT_YUV420P10BE,
    VPE_FMT_P010LE,
    VPE_FMT_P010BE,
    VPE_FMT_YUV422P,
    VPE_FMT_YUV422P10LE,
    VPE_FMT_YUV422P10BE,
    VPE_FMT_YUV444P,
    VPE_FMT_RGB24,
    VPE_FMT_BGR24,
    VPE_FMT_ARGB,
    VPE_FMT_RGBA,
    VPE_FMT_ABGR,
    VPE_FMT_BGRA,
} VpePixFmt;

typedef struct DecOption {
    char *pp_setting;
    char *dev_name;
    int buffer_depth;
    int transcode;
    int task_id;
    int priority;
} DecOption;

/**
 * VP9 encoder opition data which got from application like ffmpeg
 */
typedef struct VpeEncVp9Opition {
    int force_8bit;
    /*low level hardware frame context, point to one VpeFrame pointer*/
    void *framectx;

    /*Parameters which copied from ffmpeg AVCodecContext*/
    uint32_t bit_rate;
    int bit_rate_tolerance;
    int frame_rate_numer;
    int frame_rate_denom;

    /*VPE VP9 encoder public parameters*/
    char *preset;
    int effort;
    int lag_in_frames;
    int passes;

    /*VPE VP9 encoder public parameters with ---vpevp9-params*/
    char *enc_params;
} VpeEncVp9Opition;

/*
 * PP configuration
 */
typedef struct VpePPOpition {
    int nb_outputs;
    int force_10bit;
    char *low_res;
    int w;
    int h;
    VpePixFmt format;
    /*low level hardware frame context, point to one VpeFrame pointer*/
    void *frame;
} VpePPOpition;

typedef struct VpePacket {
    uint32_t size;
    uint8_t *data;
    int64_t pts;
    int64_t pkt_dts;
    int32_t index_encoded;
    void *opaque;
} VpePacket;

typedef struct VpeCrop{
    int enabled;
    int x;
    int y;
    int w;
    int h;
} VpeCrop;

typedef struct VpeScale{
    int enabled;
    int w;
    int h;
} VpeScale;

typedef struct VpePicData{
    unsigned int is_interlaced;
    unsigned int pic_stride;
    unsigned int crop_out_width;
    unsigned int crop_out_height;
    unsigned int pic_format;
    unsigned int pic_pixformat;
    unsigned int bit_depth_luma;
    unsigned int bit_depth_chroma;
    unsigned int pic_compressed_status;
} VpePicData;

typedef struct VpePicInfo{
    int enabled;
    int flag;
    int format;
    int width;
    int height;
    int pic_width;
    int pic_height;
    VpeCrop crop;
    VpeScale scale;
    VpePicData picdata; /* add for get init param*/
} VpePicInfo;

typedef struct VpeFrame {
    int task_id;
    int src_width;
    int src_height;
    int width;
    int height;

    /* For video the linesizes should be multiples of the CPUs alignment
     * preference, this is 16 or 32 for modern desktop CPUs.
     * Some code requires such alignment other code can be slower without
     * correct alignment, for yet other it makes no difference.*/
    int linesize[3];
    int key_frame;
    int64_t pts;
    int64_t pkt_dts;
    uint8_t *data[3];
    uint32_t pic_struct_size;
    VpePicInfo pic_info[PIC_INDEX_MAX_NUMBER];
    void *opaque;
    void (*vpe_frame_free)(void *opaque, uint8_t *data);
} VpeFrame;

typedef struct VpeSysInfo {
    int device;
    char *device_name;
    int sys_log_level;
    int task_id;
    int priority;
} VpeSysInfo;

typedef struct VpeCtrlCmdParam {
    VpeCmd cmd;
    void *data;
} VpeCtrlCmdParam;

typedef enum VpePlugin {
    H264DEC_VPE,
    HEVCDEC_VPE,
    VP9DEC_VPE,
    H26XENC_VPE,
    VP9ENC_VPE,
    PP_VPE,
    SPLITER_VPE,
    HWDOWNLOAD_VPE,
    HWCONTEXT_VPE,
} VpePlugin;

/*
 * below definition is for H26xEnc
 */
typedef enum VpiH26xCodecID {
    CODEC_ID_HEVC,
    CODEC_ID_H264,
} VpiH26xCodecID;

typedef enum VpeH26xPreset {
    VPE_PRESET_NONE,
    VPE_PRESET_SUPERFAST,
    VPE_PRESET_FAST,
    VPE_PRESET_MEDIUM,
    VPE_PRESET_SLOW,
    VPE_PRESET_SUPERSLOW,
    VPE_PRESET_NUM
} VpeH26xPreset;

typedef enum VpeFlushState {
    VPIH26X_FLUSH_IDLE,
    VPIH26X_FLUSH_PREPARE,
    VPIH26X_FLUSH_TRANSPIC,
    VPIH26X_FLUSH_ENCDATA,
    VPIH26X_FLUSH_FINISH,
} VpeFlushState;

typedef enum VpiPixFmt{
    VPI_YUV420_PLANAR,
    VPI_YUV420_SEMIPLANAR,
    VPI_YUV420_SEMIPLANAR_VU,
    VPI_YUV420_PLANAR_10BIT_P010,
} VpiPixFmt;

typedef enum VpiH26xEncCtrl {
    VPI_H26x_ENC_CTRL_GET_NEXT_PIC = 50,
    VPI_H26x_ENC_CTRL_SET_FINDPIC_PPINDEX,
    VPI_H26x_ENC_CTRL_GET_FLUSHSTATE,
    VPI_H26x_ENC_CTRL_UPDATE_STATISTIC,
    VPI_H26x_ENC_CTRL_NO_INFRM,
} VpiH26xEncCtrl;

typedef enum VpiEncRet {
    VPI_ENC_START_OK = 100, /*OK returned at encoder's start stage*/
    VPI_ENC_START_OUT_ES, /*OUT_ES returned at encoder's start stage*/
    VPI_ENC_START_ERROR, /*ERROR returned at encoder's stage stage*/
    VPI_ENC_ENC_OK, /*OK returned at encoder's encoding stage*/
    VPI_ENC_ENC_READY, /*READY returned at encoder's encoding stage*/
    VPI_ENC_ENC_FRM_ENQUEUE, /*FRM_ENQUEUE returned at encoder's encoding
                                stage*/
    VPI_ENC_ENC_ERROR, /*ERROR returned at encoder's encoding stage*/
    VPI_ENC_FLUSH_IDLE_OK, /*OK returned at encoder's FLUSH_IDLE stage */
    VPI_ENC_FLUSH_IDLE_READY, /*READY returned at encoder's FLUSH_IDLE stage*/
    VPI_ENC_FLUSH_IDLE_FRM_ENQUEUE, /*RFM_ENQUEUE returned at encoder's
                                       FLUSH_IDLE stage*/
    VPI_ENC_FLUSH_IDLE_ERROR, /*ERROR returned at encoder's FLUSH_IDLE stage*/
    VPI_ENC_FLUSH_PREPARE, /*indicate the encoder at FLUSH_PREPARE stage*/
    VPI_ENC_FLUSH_TRANSPIC_OK, /*OK returned at encoder's FLUSH_TRANSPIC stage*/
    VPI_ENC_FLUSH_TRANSPIC_READY, /*READY returned at encoder's FLUSH_TRANSPIC
                                     stage*/
    VPI_ENC_FLUSH_TRANSPIC_ERROR, /*READY returned at encoder's FLUSH_TRANSPIC
                                     stage*/
    VPI_ENC_FLUSH_ENCDATA_OK, /*OK returned at encoder's FLUSH_ENCDATA stage*/
    VPI_ENC_FLUSH_ENCDATA_READY, /*READY returned at encoder's FLUSH_ENCDATA
                                    stage*/
    VPI_ENC_FLUSH_ENCDATA_FRM_ENQUEUE, /*FRM_ENQUEUE returned at encoder's
                                          FLUSH_ENCDATA stage*/
    VPI_ENC_FLUSH_ENCDATA_ERROR, /*ERROR returned at encoder's FLUSH_ENCDATA
                                    stage*/
    VPI_ENC_FLUSH_FINISH_OK, /*OK returned at encoder's FLUSH_FINISH stage*/
    VPI_ENC_FLUSH_FINISH_ERROR, /*ERROR returned at encoder's FLUSH_FINISH
                                   stage*/
    VPI_ENC_FLUSH_FINISH_END /*END returned at encoder's FLUSH_FINISH stage*/
} VpiEncRet;

typedef struct H26xEncCfg {
    char module_name[20];
    int pp_index;
    int priority;
    char *device;
    // int task_id;
    int crf;
    char *preset;
    VpiH26xCodecID codec_id;
    const char *codec_name;
    char *profile;
    char *level;
    int bit_per_second;
    int input_rate_numer; /* Input frame rate numerator */
    int input_rate_denom; /* Input frame rate denominator */
    int lum_width_src;
    int lum_height_src;

    /*VCENC_YUV420_PLANAR,VCENC_YUV420_SEMIPLANAR,VCENC_YUV420_SEMIPLANAR_VU,
       VCENC_YUV420_PLANAR_10BIT_P010,VCENC_YUV420_PLANAR*/
    int input_format;
    char *enc_params;
    VpeFrame *frame_ctx;
} H26xEncCfg;

typedef struct VpeApi {
    int (*init)(VpeCtx, void *);

    int (*decode)(VpeCtx, void *indata, void *outdata);

    int (*encode)(VpeCtx, void *indata, void *outdata);

    int (*decode_put_packet)(VpeCtx, void *indata);

    int (*decode_get_frame)(VpeCtx, void *outdata);

    int (*encode_put_frame)(VpeCtx, void *indata);

    int (*encode_get_packet)(VpeCtx, void *outdata);

    int (*control)(VpeCtx, void *indata, void *outdata);

    int (*process)(VpeCtx, void *indata, void *outdata);

    int (*close)(VpeCtx);
} VpeApi;

typedef struct VpeMediaProc {
    VpeSysInfo *info;
    VpeCtx ctx;
    VpeApi *vpi;
} VpeMediaProc;

#endif
