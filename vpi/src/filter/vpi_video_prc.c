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

#include "vpi_log.h"
#include "vpi_video_prc.h"
#include "vpi_video_hwdwprc.h"
#include "vpi_video_pp.h"

VpiRet vpi_vprc_init(VpiPrcCtx *vpi_ctx, void *prc_cfg)
{
    VpiRet ret = VPI_SUCCESS;

    switch (vpi_ctx->filter_type) {
    case Filter_PP:
        ret = vpi_prc_pp_init(vpi_ctx, prc_cfg);
        break;
    case Filter_Spliter:
        break;
    case Filter_HwDownload:
        ret = vpi_prc_hwdw_init(vpi_ctx, prc_cfg);
        break;
    default:
        VPILOGW("Unknown/Not supported format %d", vpi_ctx->filter_type);
        ret = VPI_ERR_VALUE;
    }

    return ret;
}

VpiRet vpi_vprc_process(VpiPrcCtx *vpi_ctx, void *indata, void *outdata)
{
    VpiRet ret = VPI_SUCCESS;

    switch (vpi_ctx->filter_type) {
    case Filter_PP:
        ret = vpi_prc_pp_process(vpi_ctx, indata, outdata);
        break;
    case Filter_Spliter:
        break;
    case Filter_HwDownload:
        ret = vpi_prc_hwdw_process(vpi_ctx, indata, outdata);
        break;
    default:
        VPILOGW("Unknown/Not supported format %d", vpi_ctx->filter_type);
        break;
    }
    return ret;
}

VpiRet vpi_vprc_control(VpiPrcCtx *vpi_ctx, void *indata, void *outdata)
{
    VpiRet ret = VPI_SUCCESS;

    switch (vpi_ctx->filter_type) {
    case Filter_PP:
        vpi_prc_pp_control(vpi_ctx, indata, outdata);
        break;
    case Filter_Spliter:
        break;
    case Filter_HwDownload:
        vpi_prc_hwdw_control(vpi_ctx, indata, outdata);
        break;
    default:
        VPILOGW("Unknown/Not supported format %d", vpi_ctx->filter_type);
        ret = VPI_ERR_VALUE;
    }

    return ret;
}

int vpi_vprc_close(VpiPrcCtx *vpi_ctx)
{
    int ret = VPI_SUCCESS;

    switch (vpi_ctx->filter_type) {
    case Filter_PP:
        ret = vpi_prc_pp_close(vpi_ctx);
        break;
    case Filter_Spliter:
        break;
    case Filter_HwDownload:
        ret = vpi_prc_hwdw_close(vpi_ctx);
        break;
    default:
        VPILOGW("Unknown/Not supported format %d", vpi_ctx->filter_type);
        ret = VPI_ERR_VALUE;
    }

    return ret;
}
