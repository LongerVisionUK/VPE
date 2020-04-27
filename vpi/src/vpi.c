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
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "vpe_vpi.h"
#include "vpi.h"
#include "vpi_log_manager.h"
#include "vpi_video_dec.h"
#include "vpi_video_prc.h"
#include "vpi_video_h26xenc.h"
#include "vpi_video_vp9enc.h"

#include "transcoder.h"

#ifdef FB_SYSLOG_ENABLE
#include "syslog_sink.h"
#endif

static VpiCtx *vpi_ctx      = NULL;
static VpiHwCtx *vpi_hw_ctx = NULL;
static int log_enabled      = 0;

static int vpi_init(VpeCtx vpe_ctx, void *cfg)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx;
    VpiEncVp9Ctx *vp9enc_ctx;
    VpiH26xEncCtx *h26xenc_ctx;
    VpiPrcCtx *prc_ctx;
    VpiRet ret = VPI_SUCCESS;
    DecOption *dec_option = (DecOption *)cfg;

    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
        dec_ctx = (VpiDecCtx *)vpe_vpi_ctx->ctx;
        memset(dec_ctx, 0, sizeof(VpiDecCtx));
        dec_ctx->dec_fmt      = Dec_H264_H10P;
        dec_option->task_id   = vpi_hw_ctx->task_id;
        dec_option->priority  = vpi_hw_ctx->priority;
        ret                   = vpi_vdec_init(dec_ctx, dec_option);
        break;

    case HEVCDEC_VPE:
        dec_ctx = (VpiDecCtx *)vpe_vpi_ctx->ctx;
        memset(dec_ctx, 0, sizeof(VpiDecCtx));
        dec_ctx->dec_fmt      = Dec_HEVC;
        dec_option->task_id   = vpi_hw_ctx->task_id;
        dec_option->priority  = vpi_hw_ctx->priority;
        ret                   = vpi_vdec_init(dec_ctx, cfg);
        break;

    case VP9DEC_VPE:
        dec_ctx = (VpiDecCtx *)vpe_vpi_ctx->ctx;
        memset(dec_ctx, 0, sizeof(VpiDecCtx));
        dec_ctx->dec_fmt      = Dec_VP9;
        dec_option->task_id   = vpi_hw_ctx->task_id;
        dec_option->priority  = vpi_hw_ctx->priority;
        ret                   = vpi_vdec_init(dec_ctx, cfg);
        break;

    case H26XENC_VPE:
        h26xenc_ctx = (VpiH26xEncCtx *)vpe_vpi_ctx->ctx;
        //memset(h26xenc_ctx, 0, sizeof(VpiH26xEncCtx));
        //h26xenc_ctx->enc_fmt = Enc_H26x;
        H26xEncCfg *h26x_enc_cfg = (H26xEncCfg *)cfg;
        h26x_enc_cfg->priority   = vpi_hw_ctx->priority;
        h26x_enc_cfg->device     = vpi_hw_ctx->device_name;
        ret                      = vpi_h26xe_init(h26xenc_ctx, h26x_enc_cfg);
        break;

    case VP9ENC_VPE:
        vp9enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
        ret        = vpi_venc_vp9_init(vp9enc_ctx, cfg);
        break;

    case PP_VPE:
        prc_ctx = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        prc_ctx->filter_type       = Filter_PP;
        if (vpi_hw_ctx) {
            vpi_vprc_init(prc_ctx, &prc_ctx->ppfilter.params);
        }
        break;

    case SPLITER_VPE:
        prc_ctx = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        prc_ctx->filter_type = Filter_Spliter;
        break;

    case HWDOWNLOAD_VPE:
        prc_ctx = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        prc_ctx->filter_type = Filter_HwDownload;
        if (vpi_hw_ctx) {
            vpi_vprc_init(prc_ctx, vpi_hw_ctx->device_name);
        }
        break;

    default:
        break;
    }

    VPILOGD("plugin %d init finished\n", vpe_vpi_ctx->plugin);

    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

static int vpi_decode_put_packet(VpeCtx vpe_ctx, void *indata)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx     = (VpiDecCtx *)vpe_vpi_ctx->ctx;
    VpiRet ret             = VPI_SUCCESS;

    VPILOGD("plugin %d put packet start\n", vpe_vpi_ctx->plugin);
    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        ret = vpi_vdec_put_packet(dec_ctx, indata);
        VPILOGD("plugin %d put packet finish\n", vpe_vpi_ctx->plugin);
        return ret;
    case H26XENC_VPE:
    case VP9ENC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("decode_put_packet function is not in current pluging %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    default:
        break;
    }

    return ret;
}

static int vpi_decode_get_frame(VpeCtx vpe_ctx, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx     = (VpiDecCtx *)vpe_vpi_ctx->ctx;
    VpiRet ret             = VPI_SUCCESS;

    VPILOGD("plugin %d get frame start\n", vpe_vpi_ctx->plugin);
    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        ret = vpi_vdec_get_frame(dec_ctx, outdata);
        VPILOGD("plugin %d get frame finish\n", vpe_vpi_ctx->plugin);
        return ret;
    case H26XENC_VPE:
    case VP9ENC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("decode_get_frame function is not in current pluging %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    default:
        break;
    }

    return ret;
}

static int vpi_decode(VpeCtx vpe_ctx, void *indata, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx     = (VpiDecCtx *)vpe_vpi_ctx->ctx;
    VpiRet ret             = VPI_SUCCESS;

    VPILOGD("plugin %d decode\n", vpe_vpi_ctx->plugin);
    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        ret = vpi_vdec_decode(dec_ctx, indata, outdata);
        return ret;
    case H26XENC_VPE:
    case VP9ENC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("decode funtion is not in current plugin %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    default:
        break;
    }

    return 0;
}

static int vpi_encode_put_frame(VpeCtx vpe_ctx, void *indata)
{
    VpeVpiCtx *vpe_vpi_ctx    = (VpeVpiCtx *)vpe_ctx;
    VpiEncVp9Ctx *vp9_enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
    VpiRet ret                = VPI_SUCCESS;

    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("encode_put_frame function is not in current pluging %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    case H26XENC_VPE:
        //ret = vpi_venc_put_frame(enc_ctx, indata);
        return ret;
    case VP9ENC_VPE:
        //ret = vpi_venc_vp9_put_frame(enc_ctx, indata);
        return ret;
    default:
        break;
    }

    return ret;
}

static int vpi_encode_get_packet(VpeCtx vpe_ctx, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx    = (VpeVpiCtx *)vpe_ctx;
    VpiEncVp9Ctx *vp9_enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
    VpiRet ret                = VPI_SUCCESS;

    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("encode_get_packet function is not in current pluging %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    case H26XENC_VPE:
        //ret = vpi_venc_get_packet(enc_ctx, outdata);
        return ret;
    case VP9ENC_VPE:
        //ret = vpi_venc_vp9_get_packet(enc_ctx, outdata);
        return ret;
    default:
        break;
    }

    return ret;
}

static int vpi_encode(VpeCtx vpe_ctx, void *indata, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiH26xEncCtx *h26xenc_ctx;
    VpiEncVp9Ctx *vp9_enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
    VpiRet ret                = VPI_SUCCESS;

    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        VPILOGE("encode funtion is not in current plugin %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    case H26XENC_VPE:
        h26xenc_ctx = (VpiH26xEncCtx *)vpe_vpi_ctx->ctx;
        ret         = vpi_h26xe_encode(h26xenc_ctx, indata, outdata);
        break;
    case VP9ENC_VPE:
        ret = vpi_venc_vp9_encode(vp9_enc_ctx, indata, outdata);
        break;
    default:
        break;
    }

    if (H26XENC_VPE == vpe_vpi_ctx->plugin)
        return ret;
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

static int vpi_process(VpeCtx vpe_ctx, void *indata, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)vpe_ctx;
    VpiPrcCtx *prc_ctx     = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
    VpiRet ret             = VPI_SUCCESS;

    VPILOGD("plugin %d process\n", vpe_vpi_ctx->plugin);
    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
    case H26XENC_VPE:
    case VP9ENC_VPE:
        VPILOGE("process funtion is not in current plugin %d",
                vpe_vpi_ctx->plugin);
        ret = VPI_ERR_WRONG_PLUGIN;
        break;
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        ret = vpi_vprc_process(prc_ctx, indata, outdata);
        break;
    default:
        break;
    }

    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

static int vpi_control(VpeCtx vpe_ctx, void *indata, void *outdata)
{
    VpeVpiCtx *vpe_vpi_ctx    = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx        = (VpiDecCtx *)vpe_vpi_ctx->ctx;
    VpiEncVp9Ctx *vp9_enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
    VpiPrcCtx *prc_ctx        = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
    VpiH26xEncCtx *h26xenc_ctx;
    VpiRet ret = VPI_SUCCESS;

    VPILOGD("plugin %d control start\n", vpe_vpi_ctx->plugin);
    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        vpi_vdec_control(dec_ctx, indata, outdata);
        break;

    case H26XENC_VPE:
        h26xenc_ctx = (VpiH26xEncCtx *)vpe_vpi_ctx->ctx;
        vpi_h26xe_ctrl(h26xenc_ctx, indata, outdata);
        break;

    case VP9ENC_VPE:
        vpi_venc_vp9_control(vp9_enc_ctx, indata, outdata);
        break;

    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        vpi_vprc_control(prc_ctx, indata, outdata);
        break;
    default:
        break;
    }

    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

static int vpi_close(VpeCtx vpe_ctx)
{
    VpeVpiCtx *vpe_vpi_ctx    = (VpeVpiCtx *)vpe_ctx;
    VpiDecCtx *dec_ctx        = (VpiDecCtx *)vpe_vpi_ctx->ctx;
    VpiEncVp9Ctx *vp9_enc_ctx = (VpiEncVp9Ctx *)vpe_vpi_ctx->ctx;
    VpiPrcCtx *prc_ctx        = (VpiPrcCtx *)vpe_vpi_ctx->ctx;
    VpiH26xEncCtx *h26xenc_ctx;
    VpiRet ret = VPI_SUCCESS;

    switch (vpe_vpi_ctx->plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        ret = vpi_vdec_close(dec_ctx);
        break;
    case H26XENC_VPE:
        h26xenc_ctx = (VpiH26xEncCtx *)vpe_vpi_ctx->ctx;
        vpi_h26xe_close(h26xenc_ctx);
        break;
    case VP9ENC_VPE:
        ret = vpi_venc_vp9_close(vp9_enc_ctx);
        break;
    case PP_VPE:
    case SPLITER_VPE:
    case HWDOWNLOAD_VPE:
        if (vpi_hw_ctx) {
            vpi_vprc_close(prc_ctx);
        }
        break;
    default:
        break;
    }

    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

static VpeApi vpe_api = {
    vpi_init,
    vpi_decode,
    vpi_encode,
    vpi_decode_put_packet,
    vpi_decode_get_frame,
    vpi_encode_put_frame,
    vpi_encode_get_packet,
    vpi_control,
    vpi_process,
    vpi_close,
};

static int log_init()
{
    char filename[512];
    time_t now;
    struct tm *tm;

    time(&now);
    tm = localtime(&now);

    sprintf(filename, "vpi_%04d%02d%02d_%02d%02d%02d.log", tm->tm_year + 1900,
            tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("log_filename %s\n", filename);

    log_open(filename);
    return 0;
}

int vpe_create(VpeCtx *ctx, VpeApi **vpi, VpePlugin plugin)
{
    VpiDecCtx *dec_ctx;
    VpiEncVp9Ctx *vp9_enc_ctx;
    VpiH26xEncCtx *h26xenc_ctx;
    VpiPrcCtx *prc_ctx;

    if (!log_enabled) {
        if (log_init()) {
            return -1;
        }
        log_enabled = 1;
    }

    if (HWCONTEXT_VPE == plugin) {
        VpeSysInfo *vpi_dev_info = (VpeSysInfo *)(*ctx);
        if (!vpi_dev_info) {
            VPILOGE("vpi dev info NULL\n");
            return -1;
        }
        if (!vpi_hw_ctx && (vpi_dev_info->device >= 0)) {
            vpi_hw_ctx             = malloc(sizeof(VpiHwCtx));
            vpi_hw_ctx->hw_context = vpi_dev_info->device;
            if (ioctl(vpi_hw_ctx->hw_context, CB_TRANX_MEM_GET_TASKID,
                      &vpi_hw_ctx->task_id) < 0) {
                VPILOGE("get task id failed!\n");
                return -1;
            }
            strcpy(vpi_hw_ctx->device_name, vpi_dev_info->device_name);
            vpi_dev_info->task_id = vpi_hw_ctx->task_id;
            vpi_hw_ctx->priority  = vpi_dev_info->priority;

#ifdef FB_SYSLOG_ENABLE
            if (!vpi_dev_info->sys_log_level) {
                vpi_dev_info->sys_log_level = SYSLOG_SINK_LEV_STAT;
            }
            VPILOGD("sys log level %d\n", vpi_dev_info->sys_log_level);
            init_syslog_module("system", vpi_dev_info->sys_log_level);
#endif
        }

        return 0;
    }

    if (NULL == ctx || NULL == vpi) {
        VPILOGE("invalid input ctx %p vpi %p\n", ctx, vpi);
        return -1;
    }

    *ctx = NULL;
    *vpi = NULL;

    VPILOGD("enter ctx %p vpi %p, plugin %d\n", ctx, vpi, plugin);
    VpeVpiCtx *vpe_vpi_ctx = malloc(sizeof(VpeVpiCtx));
    if (NULL == vpe_vpi_ctx) {
        VPILOGE("failed to allocate vpe_vpi context\n");
        return -1;
    }
    memset(vpe_vpi_ctx, 0, sizeof(VpeVpiCtx));
    vpe_vpi_ctx->plugin = plugin;

    if (NULL == vpi_ctx) {
        vpi_ctx = malloc(sizeof(VpiCtx));
        if (NULL == vpi_ctx) {
            VPILOGE("failed to allocate vpi context\n");
            return -1;
        }
        memset(vpi_ctx, 0, sizeof(VpiCtx));
    }
    switch (plugin) {
    case H264DEC_VPE:
    case HEVCDEC_VPE:
    case VP9DEC_VPE:
        dec_ctx = (VpiDecCtx *)malloc(sizeof(VpiDecCtx));
        memset(dec_ctx, 0, sizeof(VpiDecCtx));
        vpe_vpi_ctx->ctx     = dec_ctx;
        vpi_ctx->vpi_dec_ctx = vpe_vpi_ctx;
        vpi_ctx->ref_cnt++;
        break;
    case H26XENC_VPE:
        h26xenc_ctx = (VpiH26xEncCtx *)malloc(sizeof(VpiH26xEncCtx));
        if (h26xenc_ctx == NULL) {
            return -1;
        }
        memset(h26xenc_ctx, 0, sizeof(VpiH26xEncCtx));
        vpe_vpi_ctx->ctx = h26xenc_ctx;
        vpi_ctx->ref_cnt++;
        break;
    case VP9ENC_VPE:
        vp9_enc_ctx = (VpiEncVp9Ctx *)malloc(sizeof(VpiEncVp9Ctx));
        memset(vp9_enc_ctx, 0, sizeof(VpiEncVp9Ctx));
        vp9_enc_ctx->vp9_enc_cfg.device   = (char *)&vpi_hw_ctx->device_name;
        vp9_enc_ctx->vp9_enc_cfg.priority = vpi_hw_ctx->priority;
        vp9_enc_ctx->vp9_enc_cfg.mem_id   = vpi_hw_ctx->task_id;
        vpe_vpi_ctx->ctx                  = vp9_enc_ctx;
        vpi_ctx->ref_cnt++;
        break;
    case PP_VPE:
        prc_ctx = (VpiPrcCtx *)malloc(sizeof(VpiPrcCtx));
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        vpe_vpi_ctx->ctx        = prc_ctx;
        vpi_ctx->vpi_prc_pp_ctx = vpe_vpi_ctx;
        vpi_ctx->ref_cnt++;
        break;
    case SPLITER_VPE:
        prc_ctx = (VpiPrcCtx *)malloc(sizeof(VpiPrcCtx));
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        vpe_vpi_ctx->ctx             = prc_ctx;
        vpi_ctx->vpi_prc_spliter_ctx = vpe_vpi_ctx;
        vpi_ctx->ref_cnt++;
        break;
    case HWDOWNLOAD_VPE:
        prc_ctx = (VpiPrcCtx *)malloc(sizeof(VpiPrcCtx));
        memset(prc_ctx, 0, sizeof(VpiPrcCtx));
        vpe_vpi_ctx->ctx          = prc_ctx;
        vpi_ctx->vpi_prc_hwdw_ctx = vpe_vpi_ctx;
        vpi_ctx->ref_cnt++;
        break;
    default:
        break;
    }

    *ctx = vpe_vpi_ctx;
    *vpi = &vpe_api;
    return 0;
}

int vpe_destroy(VpeCtx ctx)
{
    if (ctx == NULL) {
        if (vpi_hw_ctx && vpi_hw_ctx->hw_context) {
            if (ioctl(vpi_hw_ctx->hw_context, CB_TRANX_MEM_FREE_TASKID,
                      &vpi_hw_ctx->task_id) < 0) {
                VPILOGE("free hw context task id failed!\n");
            }

            if (vpi_hw_ctx) {
                free(vpi_hw_ctx);
                vpi_hw_ctx = NULL;
            }
#ifdef FB_SYSLOG_ENABLE
            close_syslog_module();
#endif
            log_close();
            log_enabled = 0;
        }
        return 0;
    }

    VpeVpiCtx *vpe_vpi_ctx = (VpeVpiCtx *)ctx;
    free(vpe_vpi_ctx->ctx);
    free(ctx);

    if (vpi_ctx) {
        vpi_ctx->ref_cnt--;
        if (vpi_ctx->ref_cnt == 0) {
            free(vpi_ctx);
            vpi_ctx = NULL;
        }
    }

    return 0;
}
