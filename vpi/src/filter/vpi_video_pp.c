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
#include <assert.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "vpe_types.h"
#include "vpi_log.h"
#include "vpi_video_pp.h"
#include "vpi_video_prc.h"
#include "vpi_error.h"

#include "ppinternal.h"
#include "dectypes.h"
#include "hugepage_api.h"
#include "trans_fd_api.h"
#include "vpi_video_dec_tb_defs.h"

#define ALIGN(a) (1 << (a))
#define XALIGN(x, n) (((x) + ((n)-1)) & (~((n)-1)))
#define WIDTH_ALIGN 16
#define HEIGHT_ALIGN 8
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define NEXT_MULTIPLE(value, n) (((value) + (n)-1) & ~((n)-1))
#define PP_IN_HEIGHT_ALIGN 8
#define PP_OUT_HEIGHT_ALIGN 4
#define PP_FETCH_ALIGN 32
#define HUGE_PGAE_SIZE (2 * 1024 * 1024)
#define NUM_OUT 1

extern u32 dec_pp_in_blk_size;
static struct TBCfg tb_cfg;

typedef struct VpiMwlInitParam {
    u32 client_type;
    char *device;
    int mem_id;
} VpiMwlInitParam;

typedef struct VpiMwl {
    u32 client_type;
    int fd_memalloc;
    int mem_id;
    char *device;
    EDMA_HANDLE edma_handle;
} VpiMwl;

#ifdef PP_MEM_ERR_TEST
static int pp_memory_err_cnt    = 0;
static int pp_memory_err_shadow = 0;

static int pp_memory_check()
{
    int ret = 0;
    if (pp_memory_err_cnt++ == pp_memory_err_shadow) {
        VPILOGE("[%s,%d],pp_memory_err_cnt %d, pp_mem_err_test %d, force malloc "
                "memory error!!!\n",
                __FUNCTION__, __LINE__, pp_memory_err_cnt,
                pp_memory_err_shadow);
        ret = -1;
    }
    return ret;
}
#endif

#ifdef PP_EDMA_ERR_TEST
static int pp_edma_err_cnt    = 0;
static int pp_edma_err_shadow = 0;

static int pp_edma_eerror_check()
{
    int ret = 0;
    if (pp_edma_err_cnt++ == pp_edma_err_shadow) {
        VPILOGE("[%s,%d],edma_err_cnt %d, pp_edma_err_test %d, force edma "
                "transfer error!!!\n",
                __FUNCTION__, __LINE__, pp_edma_err_cnt, pp_edma_err_shadow);
        ret = -1;
    }
    return ret;
}
#endif

static i32 pp_mwl_release(const void *instance)
{
    VpiMwl *mwl = (VpiMwl *)instance;

    if (mwl) {
        if (mwl->edma_handle) {
            TRANS_EDMA_release(mwl->edma_handle);
        }

        if (mwl->fd_memalloc > 0) {
            TranscodeCloseFD(mwl->fd_memalloc);
        }

        free(mwl);
    }
    return 0;
}

static void *pp_mwl_init(VpiMwlInitParam *param)
{
    VpiMwl *mwl;
    mwl = (VpiMwl *)calloc(1, sizeof(VpiMwl));
    if (mwl == NULL) {
        VPILOGE("%s", "failed to alloc struct VpiMwl\n");
        return NULL;
    }

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                __LINE__);
        return NULL;
    }
#endif

    mwl->client_type = param->client_type;
    mwl->fd_memalloc = -1;
    if (param->device == NULL) {
        VPILOGE("device name error\n");
        goto err;
    }
    mwl->device      = param->device;
    mwl->mem_id      = param->mem_id;
    mwl->fd_memalloc = TranscodeOpenFD(param->device, O_RDWR | O_SYNC);
    if (mwl->fd_memalloc == -1) {
        VPILOGD("failed to open: %s\n", param->device);
        goto err;
    }

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                __LINE__);
        goto err;
    }
#endif
    return mwl;

err:
    pp_mwl_release(mwl);
    return NULL;
}

static i32 pp_mwl_malloc_linear(const void *instance, u32 size,
                                struct DWLLinearMem *info)
{
    VpiMwl *mwl = (VpiMwl *)instance;

#define getpagesize() (1)
    u32 pgsize             = getpagesize();
    int ret                = 0;
    struct mem_info params = { 0 };
    u32 alloc_flag         = 0;

#define EP_SIDE_EN (1 << 1)
#define RC_SIDE_EN (1 << 2)

    assert(mwl != NULL);
    assert(info != NULL);

    info->logical_size    = size;
    size                  = NEXT_MULTIPLE(size, pgsize);
    info->size            = size;
    info->virtual_address = NULL;
    info->bus_address     = 0;
    params.size           = info->size;

    info->bus_address_rc     = 0;
    info->virtual_address_ep = NULL;

    if (info->mem_type == DWL_MEM_TYPE_CPU_FILE_SINK)
        alloc_flag = RC_SIDE_EN;
    else if (info->mem_type == DWL_MEM_TYPE_DPB)
        alloc_flag = EP_SIDE_EN;
    else
        alloc_flag = EP_SIDE_EN | RC_SIDE_EN;

    if (alloc_flag & EP_SIDE_EN) {
        params.task_id      = mwl->mem_id;
        params.mem_location = EP_SIDE;
        params.size         = info->size;
        VPILOGD("EP: size(%d) task_id(%d)\n", params.size, params.task_id);
        ret = ioctl(mwl->fd_memalloc, CB_TRANX_MEM_ALLOC, &params);
        if (ret) {
            VPILOGD("%s", "ERROR! No linear buffer available\n");
            return DWL_ERROR;
        }

#ifdef PP_MEM_ERR_TEST
        if (pp_memory_check() != 0) {
            VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                    __LINE__);
            return DWL_ERROR;
        }
#endif

        info->bus_address = params.phy_addr;
        info->size        = params.size;
    }
    if (alloc_flag & RC_SIDE_EN) {
        info->virtual_address = fbtrans_get_huge_pages(info->size);
        info->bus_address_rc  = (addr_t)info->virtual_address;
        if (info->virtual_address == NULL)
            return DWL_ERROR;

#ifdef PP_MEM_ERR_TEST
        if (pp_memory_check() != 0) {
            VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                    __LINE__);
            return DWL_ERROR;
        }
#endif
    }

    return DWL_OK;
}

static void pp_mwl_free_linear(const void *instance, struct DWLLinearMem *info)
{
    VpiMwl *mwl = (VpiMwl *)instance;
    int i;
    int ret;
    struct mem_info params;

    assert(info != NULL);

    params.task_id      = mwl->mem_id;
    params.mem_location = EP_SIDE;
    params.phy_addr     = info->bus_address;
    params.size         = info->size;
    params.rc_kvirt     = info->rc_kvirt;
    if (info->bus_address != 0) {
        ret = ioctl(mwl->fd_memalloc, CB_TRANX_MEM_FREE, &params);
        VPILOGD("ret = %d\n", ret);
    }

    if (info->virtual_address) {
        fbtrans_free_huge_pages(info->virtual_address, info->size);
        info->virtual_address = NULL;
    }
}

static int pp_split_string(char **tgt, int max, char *src, char *split)
{
    int count   = 0;
    char *currp = src;
    char *p;
    char c;
    int i    = 0;
    int last = 0;

    while ((c = *currp++) != '\0') {
        if ((p = strchr(split, c)) == NULL) {
            if (count < max) {
                tgt[count][i++] = c;
            } else {
                VPILOGE("the split count exceeds max num\n");
                return -1;
            }
            last = 1;
        } else {
            if (last == 1) {
                tgt[count][i] = '\0';
                count++;
                i = 0;
            }
            last = 0;
        }
    }
    if (last == 1) {
        tgt[count][i] = '\0';
        count++;
    }

    return count;
}

static int pp_parse_low_res(VpiPPFilter *filter)
{
    int i, j;
#define MAX_SEG_NUM 4
    char strs[MAX_SEG_NUM][256];
    char *pstrs[MAX_SEG_NUM];
    int seg_num = 0;
    char *p, *ep, *cp;

    for (i = 0; i < MAX_SEG_NUM; i++)
        pstrs[i] = &strs[i][0];

    filter->low_res_num = 0;

    if (!filter->low_res)
        return -1;

    seg_num =
        pp_split_string((char **)&pstrs, MAX_SEG_NUM, filter->low_res, "()");
    if (seg_num <= 0) {
        VPILOGE("can't find low_res info!\n");
        return -1;
    }

    while (filter->low_res_num < seg_num) {
        cp = strs[filter->low_res_num];
        if ((p = strchr(cp, ',')) != NULL) {
            filter->resizes[filter->low_res_num].x = atoi(cp);
            cp                                     = p + 1;
            if ((p = strchr(cp, ',')) != NULL) {
                filter->resizes[filter->low_res_num].y = atoi(cp);
                cp                                     = p + 1;
                if ((p = strchr(cp, ',')) != NULL) {
                    filter->resizes[filter->low_res_num].cw = atoi(cp);
                    filter->resizes[filter->low_res_num].ch = atoi(p + 1);
                    cp                                      = p + 1;
                    if ((p = strchr(cp, ',')) != NULL) {
                        cp = p + 1;
                    } else if (filter->low_res_num != 0) {
                        return -1;
                    }
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        }
        if ((p = strchr(cp, 'x')) == NULL) {
            if (cp[0] == 'd') {
                int n = atoi(cp + 1);
                if (n != 2 && n != 4 && n != 8) {
                    return -1;
                }
                filter->resizes[filter->low_res_num].sw = -n;
                filter->resizes[filter->low_res_num].sh = -n;
            } else if (filter->low_res_num != 0) {
                return -1;
            }
        } else {
            filter->resizes[filter->low_res_num].sw = atoi(cp);
            filter->resizes[filter->low_res_num].sh = atoi(p + 1);
        }
        if (filter->resizes[filter->low_res_num].sw == -1 &&
            filter->resizes[filter->low_res_num].sh == -1) {
            return -1;
        }
        filter->low_res_num++;
    }

    return 0;
}

static void pp_dump_config(PPClient *pp)
{
    VpiPPConfig *test_cfg = &pp->pp_config;
    PPConfig *dec_cfg     = &pp->dec_cfg;
    int i;

    VPILOGD("dump VpiPPConfig\n");

    VPILOGD("in_width = %d\n", test_cfg->in_width);
    VPILOGD("in_height = %d\n", test_cfg->in_height);
    VPILOGD("in_width_align = %d\n", test_cfg->in_width_align);
    VPILOGD("in_height_align = %d\n", test_cfg->in_height_align);
    VPILOGD("in_stride = %d\n", test_cfg->in_stride);
    VPILOGD("align = %d\n", test_cfg->align);
    VPILOGD("max_num_pics = %d\n", test_cfg->max_num_pics);
    VPILOGD("frame_size = %d\n", test_cfg->frame_size);
    VPILOGD("in_p010 = %d\n", test_cfg->in_p010);
    VPILOGD("compress_bypass = %d\n", test_cfg->compress_bypass);
    VPILOGD("cache_enable = %d\n", test_cfg->cache_enable);
    VPILOGD("shaper_enable = %d\n", test_cfg->shaper_enable);
    VPILOGD("pp_enabled = %d\n", test_cfg->pp_enabled);
    VPILOGD("out_p010 = %d\n", test_cfg->out_p010);

#ifdef SUPPORT_TCACHE
    TcacheContext *tcg = &pp->tcache_config;

    VPILOGD("dump TcacheContext\n");

    VPILOGD("t_in_fmt = %d\n", tcg->t_in_fmt);
    VPILOGD("t_out_fmt = %d\n", tcg->t_out_fmt);
    VPILOGD("rgb_cov_bd = %d\n", tcg->rgb_cov_bd);

    VPILOGD("t_in_width = %d\n", tcg->t_in_width);
    VPILOGD("t_in_height = %d\n", tcg->t_in_height);
    VPILOGD("t_wplanes = %d\n", tcg->t_wplanes);
    for (i = 0; i < tcg->t_wplanes; i++) {
        VPILOGD("t_in_stride[%d] = %d\n", i, tcg->t_in_stride[i]);
        VPILOGD("t_in_align_stride[%d] = %d\n", i, tcg->t_in_align_stride[i]);
        VPILOGD("t_in_plane_height[%d] = %d\n", i, tcg->t_in_plane_height[i]);
        VPILOGD("t_in_plane_size[%d] = %d\n", i, tcg->t_in_plane_size[i]);
    }
    VPILOGD("t_out_width = %d\n", tcg->t_out_width);
    VPILOGD("t_out_height = %d\n", tcg->t_out_height);
    VPILOGD("t_rplanes = %d\n", tcg->t_rplanes);
    for (i = 0; i < tcg->t_rplanes; i++) {
        VPILOGD("t_out_stride[%d] = %d\n", i, tcg->t_out_stride[i]);
        VPILOGD("t_out_align_stride[%d] = %d\n", i,
                tcg->t_out_align_stride[i]);
        VPILOGD("t_out_plane_height[%d] = %d\n", i,
                tcg->t_out_plane_height[i]);
        VPILOGD("t_out_plane_size[%d] = %d\n", i, tcg->t_out_plane_size[i]);
    }
#endif

    VPILOGD("dump PPConfig\n");
    VPILOGD("in_format = %d\n", dec_cfg->in_format);
    VPILOGD("in_stride = %d\n", dec_cfg->in_stride);
    VPILOGD("in_height = %d\n", dec_cfg->in_height);
    VPILOGD("in_width = %d\n", dec_cfg->in_width);

    dump_ppuconfig(pp, &dec_cfg->ppu_config[0]);
}

static int pp_parse_params(PPClient *pp, VpiPPParams *params)
{
#ifdef SUPPORT_TCACHE
    TcacheContext *tcache_cfg = &pp->tcache_config;
    int i;
#endif
    VpiPPConfig *test_cfg = &pp->pp_config;

    test_cfg->max_num_pics    = params->num_of_decoded_pics;
    test_cfg->in_width        = params->width;
    test_cfg->in_height       = params->height;
    test_cfg->in_p010         = params->in_10bit;
    test_cfg->align           = params->align;
    test_cfg->compress_bypass = params->compress_bypass;

#ifdef SUPPORT_CACHE
    test_cfg->cache_enable  = params->cache_enable;
    test_cfg->shaper_enable = params->shaper_enable;
#endif

#ifdef SUPPORT_TCACHE
    tcache_cfg->t_in_width  = params->width;
    tcache_cfg->t_in_height = NEXT_MULTIPLE(params->height, PP_IN_HEIGHT_ALIGN);
#endif

    if (!strcmp(params->in_format, "nv12")) {
        test_cfg->in_p010 = 0;
#ifdef SUPPORT_TCACHE
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_NV12;
#endif
    } else if (!strcmp(params->in_format, "p010le")) {
        test_cfg->in_p010 = 1;
#ifdef SUPPORT_TCACHE
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_P010LE;
#endif
#ifdef SUPPORT_TCACHE
    } else if (!strcmp(params->in_format, "yuv420p")) {
        test_cfg->in_p010    = 0;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV420P;
    } else if (!strcmp(params->in_format, "iyuv")) {
        test_cfg->in_p010    = 0;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV420P;
    } else if (!strcmp(params->in_format, "yuv422p")) {
        test_cfg->in_p010    = 0;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV422P;
    } else if (!strcmp(params->in_format, "nv21")) {
        test_cfg->in_p010    = 0;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_NV21;
    } else if (!strcmp(params->in_format, "yuv420p10le")) {
        test_cfg->in_p010    = 1;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV420P10LE;
    } else if (!strcmp(params->in_format, "yuv420p10be")) {
        test_cfg->in_p010    = 1;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV420P10BE;
    } else if (!strcmp(params->in_format, "yuv422p10le")) {
        test_cfg->in_p010    = 1;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV422P10LE;
    } else if (!strcmp(params->in_format, "yuv422p10be")) {
        test_cfg->in_p010    = 1;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV422P10BE;
    } else if (!strcmp(params->in_format, "p010be")) {
        test_cfg->in_p010    = 1;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_P010BE;
    } else if (!strcmp(params->in_format, "yuv444p")) {
        test_cfg->in_p010    = 0;
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_YUV444P;
    } else if (!strcmp(params->in_format, "rgb24")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_RGB24;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
    } else if (!strcmp(params->in_format, "bgr24")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_BGR24;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
    } else if (!strcmp(params->in_format, "argb")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_ARGB;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
    } else if (!strcmp(params->in_format, "rgba")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_RGBA;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
    } else if (!strcmp(params->in_format, "abgr")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_ABGR;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
    } else if (!strcmp(params->in_format, "bgra")) {
        tcache_cfg->t_in_fmt = TCACHE_PIX_FMT_BGRA;
        if (params->in_10bit) {
            test_cfg->in_p010      = 1;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_10;
        } else {
            test_cfg->in_p010      = 0;
            tcache_cfg->rgb_cov_bd = TCACHE_COV_BD_8;
        }
#endif
    } else {
        VPILOGE("unsupport input format %s!!!\n", params->in_format);
        return -1;
    }

#ifdef SUPPORT_TCACHE
    tcache_cfg->t_out_fmt =
        tcache_get_output_format(tcache_cfg->t_in_fmt,
                                 test_cfg->in_p010 ? 10 : 8);
    if (tcache_cfg->t_out_fmt == -1) {
        VPILOGE("tcache get output format failed!\n");
        return -1;
    }
    VPILOGD("set tcache intput format %d -> output format %d\n",
            tcache_cfg->t_in_fmt, tcache_cfg->t_out_fmt);

    u32 maxbottom = 0;
    u32 maxright  = 0;
    for (i = 0; i < 4; i++) {
        if (!params->ppu_cfg[i].crop.enabled)
            continue;
        u32 thisbottom = NEXT_MULTIPLE(params->ppu_cfg[i].crop.y +
                                           params->ppu_cfg[i].crop.height,
                                       2);
        if (thisbottom > maxbottom) {
            maxbottom = thisbottom;
        }
        u32 thisright =
            params->ppu_cfg[i].crop.x + params->ppu_cfg[i].crop.width;
        if (thisright > maxright) {
            maxright = thisright;
        }
    }
    if ((maxbottom || maxright) &&
        (maxbottom < params->height || maxright < params->width)) {
        test_cfg->crop_enabled = 1;
        test_cfg->crop_w       = maxright;
        test_cfg->crop_h       = maxbottom;
    }
#endif

    return 0;
}

#ifdef SUPPORT_TCACHE
static void pp_tcache_calc_size(PPClient *pp)
{
    u32 i;
    VpiPPConfig *test_cfg;
    TcacheContext *tcg = &pp->tcache_config;

    tcg->t_wplanes = tcache_get_planes(tcg->t_in_fmt);

    for (i = 0; i < tcg->t_wplanes; i++) {
        tcg->t_in_stride[i] =
            tcache_get_stride(tcg->t_in_width, tcg->t_in_fmt, i, 1);
        tcg->t_in_align_stride[i] = tcache_get_stride_align(
            NEXT_MULTIPLE(tcg->t_in_stride[i], TCACHE_RAW_INPUT_ALIGNMENT));
        tcg->t_in_plane_height[i] =
            tcache_get_height(NEXT_MULTIPLE(tcg->t_in_height, 4), tcg->t_in_fmt,
                              i);
        tcg->t_in_plane_size[i] =
            tcg->t_in_align_stride[i] * tcg->t_in_plane_height[i];
        VPILOGD("tcache read plane %d, stride %d align to %d, height "
                "%d\n",
                i, tcg->t_in_stride[i], tcg->t_in_align_stride[i],
                tcg->t_in_plane_height[i]);
    }

    tcg->t_out_width  = tcg->t_in_width;
    tcg->t_out_height = tcg->t_in_height;

    tcg->t_rplanes = tcache_get_planes(tcg->t_out_fmt);

    for (i = 0; i < tcg->t_rplanes; i++) {
        tcg->t_out_stride[i] =
            tcache_get_stride(tcg->t_out_width, tcg->t_out_fmt, i, 1);
        tcg->t_out_align_stride[i] = tcache_get_stride_align(
            NEXT_MULTIPLE(tcg->t_out_stride[i], PP_FETCH_ALIGN));
        tcg->t_out_plane_height[i] =
            tcache_get_height(NEXT_MULTIPLE(tcg->t_out_height,
                                            PP_OUT_HEIGHT_ALIGN),
                              tcg->t_out_fmt, i);
        tcg->t_out_plane_size[i] =
            tcg->t_out_align_stride[i] * tcg->t_out_plane_height[i];
        VPILOGD("tcache write plane %d, stride %d align to %d, height "
                "%d\n",
                i, tcg->t_out_stride[i], tcg->t_out_align_stride[i],
                tcg->t_out_plane_height[i]);
    }
}

extern int TRANS_EDMA_RC2EP_link_finish(EDMA_HANDLE ehd);

static int pp_tcache_finish(PPClient *pp)
{
    int ret;

    ret = TRANS_EDMA_RC2EP_link_finish(pp->edma_handle);
    if (ret < 0) {
        VPILOGE("tcache wait for EDMA read link done failed\n");
    }

#ifdef PP_EDMA_ERR_TEST
    if (pp_edma_eerror_check() != 0) {
        VPILOGE("[%s,%d]PP force edma error in function\n", __FUNCTION__,
                __LINE__);
        return -1;
    }
#endif

    return ret;
}

extern int TCACHE_config(TCACHE_HANDLE thd, TCACHE_PARAM *pParam);
extern int TRANS_EDMA_RC2EP_link_config(EDMA_HANDLE ehd, u64 link_table_rc_base,
                                        void *link_table_rc_vbase,
                                        u32 element_size);

static int pp_tcache_config(PPClient *pp)
{
    u32 edma_link_ep_base[MAX_INPUT_PLANE + MAX_OUTPUT_PLANE] = {
        0x00000000, 0x02000000, 0x03000000, 0x02000000, 0x01000000
    };

    int ret;
    u32 i, plane, wplanes;
    u32 output_width;
    u32 link_size = 0;
    u32 offset[MAX_INPUT_PLANE];
    u32 offset_ep[MAX_INPUT_PLANE];
    u32 block_size[MAX_INPUT_PLANE];
    struct dma_link_table *edma_link;
    u64 edma_link_rc_base[3];
    TcacheContext *tcg = &pp->tcache_config;
    TCACHE_PARAM tcache_param;
    TCACHE_PARAM *pParam  = &tcache_param;
    VpiPPConfig *test_cfg = &pp->pp_config;
    u32 valid_in_plane_size[MAX_INPUT_PLANE];
    u32 valid_out_plane_size[MAX_OUTPUT_PLANE];

    memset(pParam, 0, sizeof(*pParam));

    pParam->RGB_COV_A = 0x4c85;
    pParam->RGB_COV_B = 0x962b;
    pParam->RGB_COV_C = 0x1d50;
    pParam->RGB_COV_E = 0x9090;
    pParam->RGB_COV_F = 0xb694;

    pParam->write_format = tcg->t_in_fmt;
    pParam->write_bd     = tcg->rgb_cov_bd;

    pParam->dtrc_read_enable   = 0;
    pParam->hs_enable          = 1;
    pParam->hs_dma_ch          = 0;
    pParam->hs_go_toggle_count = 0xf4;
    pParam->hs_ce              = tcg->t_wplanes;

    edma_link_rc_base[0] = pp->pp_in_buffer->buffer.bus_address_rc;
    edma_link_rc_base[1] =
        edma_link_rc_base[0] +
        tcache_get_block_size(tcg->t_in_align_stride[0], tcg->t_in_fmt, 0);
    edma_link_rc_base[2] =
        edma_link_rc_base[1] +
        tcache_get_block_size(tcg->t_in_align_stride[1], tcg->t_in_fmt, 1);

    for (plane = 0; plane < tcg->t_wplanes; plane++) {
        pParam->writeStride[plane]    = tcg->t_in_align_stride[plane];
        pParam->writeStartAddr[plane] = edma_link_ep_base[plane] & 0x03FFFFFF;

        if (test_cfg->crop_enabled && test_cfg->crop_h < tcg->t_in_height) {
            valid_in_plane_size[plane] =
                tcg->t_in_align_stride[plane] *
                tcache_get_height(test_cfg->crop_h, tcg->t_in_fmt, plane);
        } else {
            valid_in_plane_size[plane] = tcg->t_in_plane_size[plane];
        }

        pParam->writeEndAddr[plane] =
            pParam->writeStartAddr[plane] + valid_in_plane_size[plane] - 1;

        block_size[plane] = tcache_get_block_size(tcg->t_in_align_stride[plane],
                                                  tcg->t_in_fmt, plane);
        link_size += (valid_in_plane_size[plane] + block_size[plane] - 1) /
                     block_size[plane];
    }

    edma_link = (struct dma_link_table *)pp->edma_link.virtual_address;
    memset(edma_link, 0, (link_size + 1) * sizeof(struct dma_link_table));
    wplanes = tcg->t_wplanes;

    offset[0] = offset[1] = offset[2] = 0;
    offset_ep[0] = offset_ep[1] = offset_ep[2] = 0;
    for (i = 0; i < link_size; i++) {
        if (i == link_size - 1)
            edma_link[i].control = 0x00000009;
        else
            edma_link[i].control = 0x00000001;

        edma_link[i].size =
            MIN((u32)block_size[i % wplanes],
                valid_in_plane_size[i % wplanes] - offset_ep[i % wplanes]);
        edma_link[i].sar_low =
            (edma_link_rc_base[i % wplanes] + offset[i % wplanes]) & 0xffffffff;
        edma_link[i].sar_high =
            (edma_link_rc_base[i % wplanes] + offset[i % wplanes]) >> 32;
        edma_link[i].dst_low =
            (edma_link_ep_base[i % wplanes] + offset_ep[i % wplanes]) &
            0xffffffff;
        edma_link[i].dst_high = 0;
        offset[i % wplanes] += HUGE_PGAE_SIZE;
        offset_ep[i % wplanes] += edma_link[i].size;
    }

    pParam->read_format = tcg->t_out_fmt;
    pParam->read_count  = 0;
    pParam->read_client = 0;

    for (plane = 0; plane < tcg->t_rplanes; plane++) {
        pParam->readStride[plane]      = tcg->t_out_align_stride[plane];
        pParam->readValidStride[plane] = tcg->t_out_align_stride[plane];

        if (test_cfg->crop_enabled) {
            valid_out_plane_size[plane] =
                tcg->t_out_align_stride[plane] *
                tcache_get_height(test_cfg->crop_h, tcg->t_out_fmt, plane);
            if (tcg->t_out_fmt == TCACHE_PIX_FMT_NV12 ||
                test_cfg->crop_w < tcg->t_out_width) {
                u32 crop_align =
                    (tcg->t_out_fmt == TCACHE_PIX_FMT_NV12) ? 1 : 32;
                pParam->readValidStride[plane] =
                    tcache_get_stride(test_cfg->crop_w, tcg->t_out_fmt, plane,
                                      crop_align);
            }
        } else {
            valid_out_plane_size[plane] = tcg->t_out_plane_size[plane];
        }

        pParam->readStartAddr[plane] =
            edma_link_ep_base[MAX_INPUT_PLANE + plane] & 0x03FFFFFF;
        pParam->readEndAddr[plane] =
            pParam->readStartAddr[plane] + valid_out_plane_size[plane] -
            (pParam->readStride[plane] - pParam->readValidStride[plane]) - 1;

        VPILOGD("start addr: 0x%08x, end addr: 0x%08x, read stride: "
                "%d, read valid stride: %d\n",
                pParam->readStartAddr[plane], pParam->readEndAddr[plane],
                pParam->readStride[plane], pParam->readValidStride[plane]);
    }

    if (test_cfg->crop_enabled && test_cfg->crop_h < tcg->t_in_height) {
        pParam->hs_image_height =
            tcache_get_height(test_cfg->crop_h, tcg->t_in_fmt, 0);
    } else {
        pParam->hs_image_height = tcg->t_in_height;
    }
    pParam->hs_enable = 1;

    extern int TRANS_EDMA_RC2EP_link_config(EDMA_HANDLE ehd,
                                            u64 link_table_rc_base,
                                            void *link_table_rc_vbase,
                                            u32 element_size);

    TCACHE_config((TCACHE_HANDLE)pp->tcache_handle, (TCACHE_PARAM *)pParam);
    ret = TRANS_EDMA_RC2EP_link_config(pp->edma_handle,
                                       pp->edma_link.bus_address_rc,
                                       pp->edma_link.virtual_address,
                                       link_size + 1);
    if (ret < 0)
        return -1;

#ifdef PP_EDMA_ERR_TEST
    if (pp_edma_eerror_check() != 0) {
        VPILOGE("[%s,%d]PP force edma error in function\n", __FUNCTION__,
                __LINE__);
        return -1;
    }
#endif
    pp->dec_cfg.pp_in_buffer.bus_address = pParam->readStartAddr[0];
    return 0;
}

#endif

static int pp_calc_pic_size(PPClient *pp)
{
    VpiPPConfig *test_cfg = &pp->pp_config;
#ifdef SUPPORT_TCACHE
    TcacheContext *tcache_cfg = &pp->tcache_config;
#endif
    int i;

#ifdef SUPPORT_TCACHE
    test_cfg->in_stride =
        NEXT_MULTIPLE(test_cfg->in_width * (test_cfg->in_p010 ? 2 : 1),
                      PP_FETCH_ALIGN);
#else
    test_cfg->in_stride =
        NEXT_MULTIPLE(test_cfg->in_width * (test_cfg->in_p010 ? 2 : 1), 16);
#endif
    test_cfg->in_width_align =
        test_cfg->in_stride / (test_cfg->in_p010 ? 2 : 1);
    test_cfg->in_height_align =
        NEXT_MULTIPLE(test_cfg->in_height, PP_IN_HEIGHT_ALIGN);
    test_cfg->frame_size =
        (test_cfg->in_stride * test_cfg->in_height_align * 3) / 2;

#ifdef SUPPORT_TCACHE
    pp_tcache_calc_size(pp);
    test_cfg->frame_size = 0;
    for (i = 0; i < tcache_cfg->t_rplanes; i++) {
        test_cfg->frame_size += tcache_cfg->t_in_plane_size[i];
    }
#endif
    return 0;
}

static int pp_buf_init(PPClient *pp)
{
    u32 i;
    PPInst pp_inst        = pp->pp_inst;
    VpiPPConfig *test_cfg = &pp->pp_config;
    PPConfig *dec_cfg     = &pp->dec_cfg;
    u32 pp_out_byte_per_pixel;
    u32 pp_width, pp_height, pp_stride, pp_stride_2;
    u32 pp_buff_size;

    /* calc output buffer size */
    pp->out_buf_size = 0;
    for (i = 0; i < 4; i++) {
        if (!dec_cfg->ppu_config[i].enabled)
            continue;

        pp_out_byte_per_pixel =
            (dec_cfg->ppu_config[i].out_cut_8bits || test_cfg->in_p010 == 0) ?
                1 :
                ((dec_cfg->ppu_config[i].out_p010 ||
                  (dec_cfg->ppu_config[i].tiled_e && test_cfg->in_p010)) ?
                     2 :
                     1);

        if (dec_cfg->ppu_config[i].tiled_e) {
            pp_width  = NEXT_MULTIPLE(dec_cfg->ppu_config[i].scale.width, 4);
            pp_height = NEXT_MULTIPLE(dec_cfg->ppu_config[i].scale.height,
                                      PP_OUT_HEIGHT_ALIGN) /
                        4;
            pp_stride = NEXT_MULTIPLE(4 * pp_width * pp_out_byte_per_pixel,
                                      ALIGN(test_cfg->align));
            dec_cfg->ppu_config[i].ystride = pp_stride;
            dec_cfg->ppu_config[i].cstride = pp_stride;
            dec_cfg->ppu_config[i].align   = test_cfg->align;
            VPILOGV("pp[%d] luma, pp_width=%d, pp_height=%d, "
                    "pp_stride=%d(0x%x), pp_out_byte_per_pixel=%d, "
                    "align=%d\n",
                    i, pp_width, pp_height, pp_stride, pp_stride,
                    pp_out_byte_per_pixel, test_cfg->align);
            pp_buff_size = pp_stride * pp_height;
            pp_buff_size += PP_LUMA_BUF_RES;
            /* chroma */
            if (!dec_cfg->ppu_config[i].monochrome) {
                pp_height =
                    NEXT_MULTIPLE(dec_cfg->ppu_config[i].scale.height / 2,
                                  PP_OUT_HEIGHT_ALIGN) /
                    4; /*fix height align 2*/
                VPILOGV("pp[%d] chroma, pp_height=%d\n", i, pp_height);
                pp_buff_size += pp_stride * pp_height;
                pp_buff_size += PP_CHROMA_BUF_RES;
            }
            VPILOGV("pp[%d] pp_buff_size=%d(0x%x)\n", i, pp_buff_size,
                    pp_buff_size);
        } else {
            pp_width  = dec_cfg->ppu_config[i].scale.width;
            pp_height = dec_cfg->ppu_config[i].scale.height;
            pp_stride = NEXT_MULTIPLE(pp_width * pp_out_byte_per_pixel, 4);
            pp_stride_2 =
                NEXT_MULTIPLE(pp_width / 2 * pp_out_byte_per_pixel, 4);
            pp_buff_size = pp_stride * pp_height;
            pp_buff_size += PP_LUMA_BUF_RES;
            if (!dec_cfg->ppu_config[i].monochrome) {
                if (!dec_cfg->ppu_config[i].planar)
                    pp_buff_size += pp_stride * pp_height / 2;
                else
                    pp_buff_size += pp_stride_2 * pp_height;
                pp_buff_size += PP_LUMA_BUF_RES;
            }
            VPILOGV("pp[%d], tile_e=0 pp_width=%d , pp_height=%d , "
                    "pp_stride=%d(0x%x),pp_out_byte_per_pixel=%d\n",
                    i, pp_width, pp_height, pp_stride, pp_stride,
                    pp_out_byte_per_pixel);
            VPILOGV("pp[%d], tile_e=0 pp_buff_size=%d(0x%x)\n", i, pp_buff_size,
                    pp_buff_size);
        }
        pp->out_buf_size += NEXT_MULTIPLE(pp_buff_size, 16);
    }

#ifdef SUPPORT_DEC400
    pp->dec_table_offset = pp->out_buf_size;
    pp->out_buf_size += DEC400_PPn_TABLE_OFFSET(4);
#endif

#ifdef SUPPORT_TCACHE
    pp->edma_link.mem_type = DWL_MEM_TYPE_CPU_FILE_SINK;
    if (DWLMallocLinear(((PPContainer *)pp_inst)->dwl, 0x4000,
                        &pp->edma_link) != DWL_OK) {
        VPILOGE("UNABLE TO ALLOCATE EDMA LINK BUFFER MEMORY\n");
        goto end;
    }
    VPILOGV("[%s,%d]edma_link: bus_address=0x%llx, bus_address_rc=0x%llx, "
            "virtual_address %p, virtual_address_ep %p, size %d \n",
            __FUNCTION__, __LINE__, pp->edma_link.bus_address,
            pp->edma_link.bus_address_rc, pp->edma_link.virtual_address,
            pp->edma_link.virtual_address_ep, pp->edma_link.size);
#endif

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                __LINE__);
        return -1;
    }
#endif

    /*used for buffer management*/
    FifoInit(OUTPUT_BUF_DEPTH, &pp->pp_out_Fifo);
    if (pp->param.ext_buffers_need > OUTPUT_BUF_DEPTH) {
        VPILOGE("TOO MANY BUFFERS REQUEST, ext_buffers_need=%d\n",
                pp->param.ext_buffers_need);
        goto end;
    }
    pp->out_buf_nums = pp->param.ext_buffers_need;
    VPILOGV("pp->out_buf_nums = %d\n", pp->out_buf_nums);
    for (i = 0; i < pp->out_buf_nums; i++) {
        pp->pp_out_buffer[i].mem_type = DWL_MEM_TYPE_DPB;
        VPILOGE("Malloc No.%d pp out buffrer,(size=%d)\n", i, pp->out_buf_size);
        if (DWLMallocLinear(((PPContainer *)pp_inst)->dwl, pp->out_buf_size,
                            &pp->pp_out_buffer[i]) != DWL_OK) {
            VPILOGE("UNABLE TO ALLOCATE STREAM BUFFER MEMORY\n");
            goto end;
        }

        VPILOGV("[%s,%d]out_buffer i %d, bus_address=0x%llx, "
                "bus_address_rc=0x%llx, virtual_address %p, virtual_address_ep "
                "%p, size %d \n",
                __FUNCTION__, __LINE__, i, pp->pp_out_buffer[i].bus_address,
                pp->pp_out_buffer[i].bus_address_rc,
                pp->pp_out_buffer[i].virtual_address,
                pp->pp_out_buffer[i].virtual_address_ep,
                pp->pp_out_buffer[i].size);

#ifdef PP_MEM_ERR_TEST
        if (pp_memory_check() != 0) {
            VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                    __LINE__);
            return -1;
        }
#endif

        FifoPush(pp->pp_out_Fifo, &pp->pp_out_buffer[i],
                 FIFO_EXCEPTION_DISABLE);
    }

    VPILOGV("pp buffer fifo count=%d\n", FifoCount(pp->pp_out_Fifo));
    return 0;

end:
    return -1;
}

extern enum FifoRet FifoPop(FifoInst inst, FifoObject *object,
                            enum FifoException e);

static void pp_request_buf(PPClient *pp)
{
    struct DWLLinearMem *pp_out_buffer;

    FifoPop(pp->pp_out_Fifo, (FifoObject *)&pp_out_buffer,
            FIFO_EXCEPTION_DISABLE);
    pp->dec_cfg.pp_out_buffer = *pp_out_buffer;
    VPILOGV("[pp] get buffer bus address=%p, virtual address=%p\n",
            (void *)pp_out_buffer->bus_address, pp_out_buffer->virtual_address);
}

static int pp_buf_release(PPClient *pp)
{
    u32 i;
    PPInst pp_inst = pp->pp_inst;

    if (pp->pp_inst != NULL) {
        for (i = 0; i < pp->out_buf_nums; i++) {
            if (pp->pp_out_buffer[i].bus_address ||
                pp->pp_out_buffer[i].bus_address_rc)
                DWLFreeLinear(((PPContainer *)pp_inst)->dwl,
                              &pp->pp_out_buffer[i]);
        }
#ifdef SUPPORT_TCACHE
        if (pp->edma_link.bus_address || pp->edma_link.bus_address_rc)
            DWLFreeLinear(((PPContainer *)pp_inst)->dwl, &pp->edma_link);
#endif
    }
    return 0;
}

static void pp_get_next_pic(PPClient *pp, PPDecPicture *hpic)
{
    u32 i, j;
    int ret        = 0;
    u32 index      = 0;
    PPInst pp_inst = pp->pp_inst;
    PPContainer *pp_c;
    struct DecPicturePpu *pic = &hpic->pp_pic;

    pp_c = (PPContainer *)pp_inst;

    for (i = 0; i < DEC_MAX_OUT_COUNT; i++) {
        if (!hpic->pictures[i].pp_enabled) {
            /* current mem alloc, the dec table is added after pic mem for each ppN. */
            index += 1;
            continue;
        }

        /*pp0->vce is not the real scenario, just use to test performance.*/
        if (i == 0) {
            j = 0;
        } else {
            j = i + 1;
        }

        pic->pictures[j].sequence_info.bit_depth_luma =
            hpic->pictures[i].bit_depth_luma;
        pic->pictures[j].sequence_info.bit_depth_chroma =
            hpic->pictures[i].bit_depth_chroma;

        pic->pictures[j].sequence_info.pic_width = hpic->pictures[i].pic_width;
        pic->pictures[j].sequence_info.pic_height =
            hpic->pictures[i].pic_height;

        pic->pictures[j].picture_info.format = hpic->pictures[i].output_format;
        pic->pictures[j].picture_info.pixel_format =
            hpic->pictures[i].pixel_format;

        pic->pictures[j].pic_width     = hpic->pictures[i].pic_width;
        pic->pictures[j].pic_height    = hpic->pictures[i].pic_height;
        pic->pictures[j].pic_stride    = hpic->pictures[i].pic_stride;
        pic->pictures[j].pic_stride_ch = hpic->pictures[i].pic_stride_ch;
        pic->pictures[j].pp_enabled    = hpic->pictures[i].pp_enabled;

#ifdef SUPPORT_TCACHE
        pic->pictures[j].luma.virtual_address   = NULL;
        pic->pictures[j].chroma.virtual_address = NULL;
#else
        pic->pictures[j].luma.virtual_address =
            (u32 *)hpic->pictures[i].output_picture;
        pic->pictures[j].chroma.virtual_address =
            (u32 *)hpic->pictures[i].output_picture_chroma;
#endif
        pic->pictures[j].luma.bus_address =
            hpic->pictures[i].output_picture_bus_address;
        pic->pictures[j].chroma.bus_address =
            hpic->pictures[i].output_picture_chroma_bus_address;

        if ((hpic->pictures[i].output_format == DEC_OUT_FRM_TILED_4X4) &&
            hpic->pictures[i].pp_enabled) {
            pic->pictures[j].luma.size =
                pic->pictures[j].pic_stride * hpic->pictures[i].pic_height / 4;
            if (((hpic->pictures[i].pic_height / 4) & 1) == 1)
                pic->pictures[j].chroma.size =
                    pic->pictures[j].pic_stride_ch *
                    (hpic->pictures[i].pic_height / 4 + 1) / 2;
            else
                pic->pictures[j].chroma.size = pic->pictures[j].pic_stride_ch *
                                               hpic->pictures[i].pic_height / 8;
        } else {
            pic->pictures[j].luma.size =
                pic->pictures[j].pic_stride * hpic->pictures[i].pic_height;
            pic->pictures[j].chroma.size = pic->pictures[j].pic_stride_ch *
                                           hpic->pictures[i].pic_height / 2;
        }

#ifdef SUPPORT_DEC400
        pic->pictures[j].pic_compressed_status = 2;
        pic->pictures[j].luma_table.virtual_address =
            NULL;
        pic->pictures[j].luma_table.bus_address =
            pp_c->pp_out_buffer.bus_address + pp->dec_table_offset +
            DEC400_PPn_Y_TABLE_OFFSET(index);
        pic->pictures[j].chroma_table.virtual_address =
            NULL;
        pic->pictures[j].chroma_table.bus_address =
            pp_c->pp_out_buffer.bus_address + pp->dec_table_offset +
            DEC400_PPn_UV_TABLE_OFFSET(index);
        index++;
#else
        pic->pictures[j].pic_compressed_status = 0;
#endif

        VPILOGV("[pp] dump pic info: \n \
           pic->pictures[%d].sequence_info.bit_depth_luma = %d \n \
           pic->pictures[%d].sequence_info.bit_depth_chroma = %d \n \
           pic->pictures[%d].picture_info.format = %d \n \
           pic->pictures[%d].picture_info.pixel_format = %d \n \
           pic->pictures[%d].pic_width = %d \n \
           pic->pictures[%d].pic_height = %d \n \
           pic->pictures[%d].pic_stride = %d \n \
           pic->pictures[%d].pic_stride_ch = %d \n \
           pic->pictures[%d].pp_enabled = %d \n \
           pic->pictures[%d].luma.virtual_address = %p \n \
           pic->pictures[%d].luma.bus_address = 0x%08lx \n \
           pic->pictures[%d].chroma.virtual_address =%p \n \
           pic->pictures[%d].chroma.bus_address = 0x%08lx \n \
           pic->pictures[%d].luma.size = %d \n \
           pic->pictures[%d].chroma.size = %d \n \
           pic->pictures[%d].luma_table.virtual_address  = %p \n \
           pic->pictures[%d].luma_table.bus_address = 0x%08lx \n \
           pic->pictures[%d].chroma_table.virtual_address  = %p \n \
           pic->pictures[%d].chroma_table.bus_address = 0x%08lx \n",
                j, pic->pictures[j].sequence_info.bit_depth_luma, j,
                pic->pictures[j].sequence_info.bit_depth_chroma, j,
                pic->pictures[j].picture_info.format, j,
                pic->pictures[j].picture_info.pixel_format, j,
                pic->pictures[j].pic_width, j, pic->pictures[j].pic_height, j,
                pic->pictures[j].pic_stride, j, pic->pictures[j].pic_stride_ch,
                j, pic->pictures[j].pp_enabled, j,
                pic->pictures[j].luma.virtual_address, j,
                pic->pictures[j].luma.bus_address, j,
                pic->pictures[j].chroma.virtual_address, j,
                pic->pictures[j].chroma.bus_address, j,
                pic->pictures[j].luma.size, j, pic->pictures[j].chroma.size, j,
                pic->pictures[j].luma_table.virtual_address, j,
                pic->pictures[j].luma_table.bus_address, j,
                pic->pictures[j].chroma_table.virtual_address, j,
                pic->pictures[j].chroma_table.bus_address);
    }

    return;
}

void *pp_trans_demuxer_init(VpiPPParams *params)
{
    void *mwl                = NULL;
    VpiMwlInitParam mwl_para = { DWL_CLIENT_TYPE_ST_PP, params->device,
                                 params->mem_id };

    mwl_para.mem_id = params->mem_id;
    mwl_para.device = params->device;
    mwl             = pp_mwl_init(&mwl_para);
    if (mwl == NULL) {
        VPILOGE("Transcode demuxer mwl init failed!\n");
        return NULL;
    }
    return mwl;
}

static int vpi_pp_input_buf_init(VpiPPFilter *filter)
{
    PPClient *pp = filter->pp_client;

    /* FIXME Maybe need malloc more buffer!!!! check fbtrans demux.c:L554 */
    int height   = filter->params.height;
    int width    = filter->params.width;
    int mwl_size = ((height + 63) / 64) * (3 * 1024 * 1024);
    const void *mwl;

    mwl = filter->mwl;
    if (mwl == NULL) {
        VPILOGE("Transcode demuxer mwl error!\n");
        return -1;
    }

    filter->buffers.buffer.mem_type = DWL_MEM_TYPE_CPU_FILE_SINK;
    if (pp_mwl_malloc_linear(mwl, mwl_size, &filter->buffers.buffer)) {
        VPILOGE("No memory available for the stream buffer\n");
        return -1;
    }

    VPILOGV("[%s,%d]input_buf: bus_address=0x%llx, bus_address_rc=0x%llx, "
            "virtual_address %p, virtual_address_ep %p, size %d \n",
            __FUNCTION__, __LINE__, filter->buffers.buffer.bus_address,
            filter->buffers.buffer.bus_address_rc,
            filter->buffers.buffer.virtual_address,
            filter->buffers.buffer.virtual_address_ep,
            filter->buffers.buffer.size);

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                __LINE__);
        return -1;
    }
#endif

    /* Initialize stream to buffer start */
    filter->buffers.stream[0] = (u8 *)filter->buffers.buffer.virtual_address;
    filter->buffers.stream[1] = (u8 *)filter->buffers.buffer.virtual_address;

    return 0;
}

pp_raw_parser_inst pp_raw_parser_open(VpePixFmt format, int width, int height)
{
    VpiRawParser *inst;
    int i;
    u32 height_align;
    u32 entry_num;

    inst = calloc(1, sizeof(VpiRawParser));
    if (inst == NULL) {
        VPILOGE("alloc failed!\n");
        goto err_exit;
    }

    inst->img_width  = width;
    inst->img_height = height;
    inst->format     = format;

    VPILOGV("img_width = %d, img_height = %d, format = %d\n", inst->img_width,
            inst->img_height, inst->format);

    switch (inst->format) {
    case VPE_FMT_YUV420P:
        inst->planes          = 3;
        inst->byte_width[0]   = inst->img_width;
        inst->height[0]       = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        inst->byte_width[1] = inst->byte_width[2] = (inst->img_width + 1) / 2;
        inst->height[1] = inst->height[2] = (inst->img_height + 1) / 2;
        inst->height_align[1]             = inst->height_align[2] =
            inst->height_align[0] / 2;
        break;
    case VPE_FMT_NV12:
    case VPE_FMT_NV21:
        inst->planes        = 2;
        inst->byte_width[0] = inst->byte_width[1] = inst->img_width;
        inst->height[0]                           = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        inst->height[1]       = (inst->img_height + 1) / 2;
        inst->height_align[1] = inst->height_align[0] / 2;
        break;
    case VPE_FMT_YUV420P10LE:
    case VPE_FMT_YUV420P10BE:
        inst->planes        = 3;
        inst->byte_width[0] = inst->img_width * 2;
        inst->byte_width[1] = inst->byte_width[2] =
            ((inst->img_width + 1) / 2) * 2;
        inst->height[0]       = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        inst->height[1] = inst->height[2] = (inst->img_height + 1) / 2;
        inst->height_align[1]             = inst->height_align[2] =
            inst->height_align[0] / 2;
        break;
    case VPE_FMT_P010LE:
    case VPE_FMT_P010BE:
        inst->planes        = 2;
        inst->byte_width[0] = inst->byte_width[1] = inst->img_width * 2;
        inst->height[0]                           = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        inst->height[1]       = (inst->img_height + 1) / 2;
        inst->height_align[1] = inst->height_align[0] / 2;
        break;
    case VPE_FMT_YUV422P:
        inst->planes        = 3;
        inst->byte_width[0] = inst->img_width;
        inst->byte_width[1] = inst->byte_width[2] = (inst->img_width + 1) / 2;
        inst->height[0] = inst->height[1] = inst->height[2] = inst->img_height;
        inst->height_align[0] = inst->height_align[1] = inst->height_align[2] =
            XALIGN(inst->img_height, HEIGHT_ALIGN);
        break;
    case VPE_FMT_YUV422P10LE:
    case VPE_FMT_YUV422P10BE:
        inst->planes        = 3;
        inst->byte_width[0] = inst->img_width * 2;
        inst->byte_width[1] = inst->byte_width[2] =
            ((inst->img_width + 1) / 2) * 2;
        inst->height[0] = inst->height[1] = inst->height[2] = inst->img_height;
        inst->height_align[0] = inst->height_align[1] = inst->height_align[2] =
            XALIGN(inst->img_height, HEIGHT_ALIGN);
        break;
    case VPE_FMT_YUV444P:
        inst->planes        = 3;
        inst->byte_width[0] = inst->byte_width[1] = inst->byte_width[2] =
            inst->img_width;
        inst->height[0] = inst->height[1] = inst->height[2] = inst->img_height;
        inst->height_align[0] = inst->height_align[1] = inst->height_align[2] =
            XALIGN(inst->img_height, HEIGHT_ALIGN);
        break;
    case VPE_FMT_RGB24:
    case VPE_FMT_BGR24:
        inst->planes          = 1;
        inst->byte_width[0]   = inst->img_width * 3;
        inst->height[0]       = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        break;
    case VPE_FMT_ARGB:
    case VPE_FMT_RGBA:
    case VPE_FMT_ABGR:
    case VPE_FMT_BGRA:
        inst->planes          = 1;
        inst->byte_width[0]   = inst->img_width * 4;
        inst->height[0]       = inst->img_height;
        inst->height_align[0] = XALIGN(inst->img_height, HEIGHT_ALIGN);
        break;
    default:
        VPILOGE("error format!\n");
        goto err_exit;
    }

    for (i = 0; i < inst->planes; i++) {
        inst->stride[i]     = XALIGN(inst->byte_width[i], WIDTH_ALIGN);
        inst->plane_size[i] = inst->stride[i] * inst->height_align[i];
        inst->frame_size += inst->byte_width[i] * inst->height[i];
    }

    inst->entry_num           = (inst->img_height + 63) / 64;
    inst->hugepage_frame_size = inst->entry_num * HUGE_PGAE_SIZE;
    VPILOGV("inst->entry_num = %d, inst->hugepage_frame_size = %d\n",
            inst->entry_num, inst->hugepage_frame_size);

    return (pp_raw_parser_inst)inst;

err_exit:
    if (inst)
        free(inst);

    return NULL;
}

static int pp_raw_parsert_read_frame(pp_raw_parser_inst instance, u8 *buffer,
                                     u8 *stream[2], i32 *size, u8 rb,
                                     VpeFrame *frame)
{
    VpiRawParser *inst = (VpiRawParser *)instance;
    int read_len, total_len = 0;
    int i, h;
    u8 *plane_buf, *pBuf;
    u8 *src_buf = NULL;
    u32 rest_height, cur_height;
    int e;
    u8 *pEntryBuf;

    if (inst == NULL || buffer == NULL)
        return -1;

    if (*size < inst->hugepage_frame_size) {
        *size = inst->hugepage_frame_size;
        return -1;
    }
    plane_buf = buffer;
    for (i = 0; i < inst->planes; i++) {
        pEntryBuf   = plane_buf;
        rest_height = inst->height[i];

        src_buf = frame->data[i];

        for (e = 0; e < inst->entry_num; e++) {
            pBuf = pEntryBuf;
            cur_height =
                MIN(tcache_get_block_height(inst->format, i), rest_height);

            for (h = 0; h < cur_height; h++) {
                read_len = inst->byte_width[i];
                memcpy(pBuf, src_buf, inst->byte_width[i]);
                src_buf += inst->byte_width[i];
                if (read_len <= 0)
                    return read_len;

                total_len += read_len;
                pBuf += inst->stride[i];
            }
            pEntryBuf += HUGE_PGAE_SIZE;
            rest_height -= cur_height;
        }
        plane_buf += tcache_get_block_size(inst->stride[i], inst->format, i);
    }

    VPILOGV("total_len%d, inst->frame_size %d\n", total_len, inst->frame_size);
    if (total_len < inst->frame_size)
        return -1;

    return total_len;
}

static int pp_send_packet(VpiPPFilter *filter, struct DecInput **input,
                          VpeFrame *frame)
{
    PPClient *pp = filter->pp_client;
    i32 size, len;
    struct DecInput *buffer;

    buffer = &filter->buffers;
    size   = buffer->buffer.size;

    len = pp_raw_parsert_read_frame(filter->inst,
                                    (u8 *)buffer->buffer.virtual_address,
                                    buffer->stream, &size, 0, frame);
    if (len <= 0) {
        VPILOGE("pp_raw_parsert_read_frame failed!\n");
        return -1;
    } else {
        buffer->data_len = len;
    }
    *input = &filter->buffers;
    return 0;
}

static void pp_resolve_params_overlap(VpiPPParams *params,
                                      struct TBPpUnitParams *pp_units_params,
                                      u32 standalone)
{
    /* Override PPU1-3 parameters with tb.cfg */
    u32 i, pp_enabled;

    /* transcoder need cml have higher priority */
    if (params->pp_enabled)
        return;

    for (i = params->ppu_cfg[0].enabled ? 1 : 0; i < 4; i++) {
        params->ppu_cfg[i].enabled      = pp_units_params[i].unit_enabled;
        params->ppu_cfg[i].cr_first     = pp_units_params[i].cr_first;
        params->ppu_cfg[i].tiled_e      = pp_units_params[i].tiled_e;
        params->ppu_cfg[i].crop.enabled = pp_units_params[i].unit_enabled;
        ;
        params->ppu_cfg[i].crop.x        = pp_units_params[i].crop_x;
        params->ppu_cfg[i].crop.y        = pp_units_params[i].crop_y;
        params->ppu_cfg[i].crop.width    = pp_units_params[i].crop_width;
        params->ppu_cfg[i].crop.height   = pp_units_params[i].crop_height;
        params->ppu_cfg[i].scale.enabled = pp_units_params[i].unit_enabled;
        ;
        params->ppu_cfg[i].scale.width    = pp_units_params[i].scale_width;
        params->ppu_cfg[i].scale.height   = pp_units_params[i].scale_height;
        params->ppu_cfg[i].shaper_enabled = pp_units_params[i].shaper_enabled;
        params->ppu_cfg[i].monochrome     = pp_units_params[i].monochrome;
        params->ppu_cfg[i].planar         = pp_units_params[i].planar;
        params->ppu_cfg[i].out_p010       = pp_units_params[i].out_p010;
        params->ppu_cfg[i].out_cut_8bits  = pp_units_params[i].out_cut_8bits;
        params->ppu_cfg[i].align          = params->align;
        params->ppu_cfg[i].ystride        = pp_units_params[i].ystride;
        params->ppu_cfg[i].cstride        = pp_units_params[i].cstride;
        params->ppu_cfg[i].align          = params->align;
    }
    if (params->ppu_cfg[0].enabled) {
        /* PPU0 */
        params->ppu_cfg[0].align = params->align;
        params->ppu_cfg[0].enabled |= pp_units_params[0].unit_enabled;
        params->ppu_cfg[0].cr_first |= pp_units_params[0].cr_first;
        if (params->hw_format != DEC_OUT_FRM_RASTER_SCAN)
            params->ppu_cfg[0].tiled_e |= pp_units_params[0].tiled_e;
        params->ppu_cfg[0].planar |= pp_units_params[0].planar;
        params->ppu_cfg[0].out_p010 |= pp_units_params[0].out_p010;
        params->ppu_cfg[0].out_cut_8bits |= pp_units_params[0].out_cut_8bits;
        if (!params->ppu_cfg[0].crop.enabled &&
            pp_units_params[0].unit_enabled) {
            params->ppu_cfg[0].crop.x      = pp_units_params[0].crop_x;
            params->ppu_cfg[0].crop.y      = pp_units_params[0].crop_y;
            params->ppu_cfg[0].crop.width  = pp_units_params[0].crop_width;
            params->ppu_cfg[0].crop.height = pp_units_params[0].crop_height;
        }
        if (params->ppu_cfg[0].crop.width || params->ppu_cfg[0].crop.height)
            params->ppu_cfg[0].crop.enabled = 1;
        if (!params->ppu_cfg[0].scale.enabled &&
            pp_units_params[0].unit_enabled) {
            params->ppu_cfg[0].scale.width  = pp_units_params[0].scale_width;
            params->ppu_cfg[0].scale.height = pp_units_params[0].scale_height;
        }
        if (params->ppu_cfg[0].scale.width || params->ppu_cfg[0].scale.height)
            params->ppu_cfg[0].scale.enabled = 1;
        params->ppu_cfg[0].shaper_enabled = pp_units_params[0].shaper_enabled;
        params->ppu_cfg[0].monochrome     = pp_units_params[0].monochrome;
        params->ppu_cfg[0].align          = params->align;
        if (!params->ppu_cfg[0].ystride)
            params->ppu_cfg[0].ystride = pp_units_params[0].ystride;
        if (!params->ppu_cfg[0].cstride)
            params->ppu_cfg[0].cstride = pp_units_params[0].cstride;
    }
    pp_enabled =
        pp_units_params[0].unit_enabled || pp_units_params[1].unit_enabled ||
        pp_units_params[2].unit_enabled || pp_units_params[3].unit_enabled;
    if (pp_enabled)
        params->pp_enabled = 1;
    if (standalone) { /* pp standalone mode */
        params->pp_standalone = 1;
        if (!params->pp_enabled && !params->fscale_cfg.fixed_scale_enabled) {
            /* No pp enabled explicitly, then enable fixed ratio pp (1:1) */
            params->fscale_cfg.down_scale_x        = 1;
            params->fscale_cfg.down_scale_y        = 1;
            params->fscale_cfg.fixed_scale_enabled = 1;
            params->pp_enabled                     = 1;
        }
    }
}

static int vpi_ppclient_release(PPClient *pp_client)
{
    if (pp_client != NULL) {
        pp_buf_release(pp_client);
        PPRelease(pp_client->pp_inst);
        if (pp_client->dwl != NULL)
            DWLRelease(pp_client->dwl);

        free(pp_client);
        pp_client = NULL;
    }
    return 0;
}

extern void vpi_tb_set_default_cfg(struct TBCfg *tb_cfg);
extern PPResult PPInit(PPInst *p_post_pinst, const void *dwl);
extern void dump_ppuconfig(const void *inst, PpUnitConfig *ppu_cfg);

static PPClient *pp_client_init(VpiPPParams *params)
{
    PPClient *pp_client   = NULL;
    PPClient *pp          = NULL;
    PPInst pp_inst        = NULL;
    VpiPPConfig *test_cfg = NULL;
    int ret;
    u32 i, j;
    PPConfig *dec_cfg = NULL;
    struct DWLInitParam dwl_init;
    u32 alignh = 2;
    u32 alignw = 2;

    pp_client = (PPClient *)calloc(1, sizeof(PPClient));
    if (pp_client == NULL) {
        VPILOGE("pp client malloc failed!!!\n");
        goto err_exit;
    }

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("[%s,%d]PP force memory error in function\n", __FUNCTION__,
                __LINE__);
        goto err_exit;
    }
#endif

    pp_client->param = *params;

    /* set test bench configuration */
    vpi_tb_set_default_cfg(&tb_cfg);

    if (tb_cfg.pp_params.pre_fetch_height == 16)
        dec_pp_in_blk_size = 0;
    else if (tb_cfg.pp_params.pre_fetch_height == 64)
        dec_pp_in_blk_size = 1;

    /* process pp size -1/-2/-4/-8. */
    for (i = 1; i < 4; i++) {
        if (pp_client->param.ppu_cfg[i].scale.width == -1 ||
            pp_client->param.ppu_cfg[i].scale.width == -2 ||
            pp_client->param.ppu_cfg[i].scale.width == -4 ||
            pp_client->param.ppu_cfg[i].scale.width == -8 ||
            pp_client->param.ppu_cfg[i].scale.height == -1 ||
            pp_client->param.ppu_cfg[i].scale.height == -2 ||
            pp_client->param.ppu_cfg[i].scale.height == -4 ||
            pp_client->param.ppu_cfg[i].scale.height == -8) {
            u32 original_width  = pp_client->param.width;
            u32 original_height = pp_client->param.height;
            if (pp_client->param.ppu_cfg[i].scale.width == -1 &&
                pp_client->param.ppu_cfg[i].scale.height == -1) {
                VPILOGE("pp %d scale setting error!!!\n", i);
                goto err_exit;
            }
            if (pp_client->param.ppu_cfg[i].crop.enabled) {
                if (pp_client->param.ppu_cfg[i].crop.width != original_width) {
                    original_width = pp_client->param.ppu_cfg[i].crop.width;
                }
                if (pp_client->param.ppu_cfg[i].crop.height !=
                    original_height) {
                    original_height = pp_client->param.ppu_cfg[i].crop.height;
                }
            }
            VPILOGV("original_width = %d, original_height = %d\n",
                    original_width, original_height);
            if (pp_client->param.ppu_cfg[i].scale.width == -1) {
                pp_client->param.ppu_cfg[i].scale.width =
                    NEXT_MULTIPLE((original_width *
                                   pp_client->param.ppu_cfg[i].scale.height) /
                                      original_height,
                                  alignw);
                pp_client->param.ppu_cfg[i].scale.height =
                    NEXT_MULTIPLE(pp_client->param.ppu_cfg[i].scale.height,
                                  alignh);
            } else if (pp_client->param.ppu_cfg[i].scale.height == -1) {
                pp_client->param.ppu_cfg[i].scale.width =
                    NEXT_MULTIPLE(pp_client->param.ppu_cfg[i].scale.width,
                                  alignw);
                pp_client->param.ppu_cfg[i].scale.height =
                    NEXT_MULTIPLE((original_height *
                                   pp_client->param.ppu_cfg[i].scale.width) /
                                      original_width,
                                  alignh);
            } else if (pp_client->param.ppu_cfg[i].scale.width == -2 &&
                       pp_client->param.ppu_cfg[i].scale.height == -2) {
                pp_client->param.ppu_cfg[i].scale.width =
                    NEXT_MULTIPLE(original_width / 2, alignw);
                pp_client->param.ppu_cfg[i].scale.height =
                    NEXT_MULTIPLE(original_height / 2, alignh);
            } else if (pp_client->param.ppu_cfg[i].scale.width == -4 &&
                       pp_client->param.ppu_cfg[i].scale.height == -4) {
                pp_client->param.ppu_cfg[i].scale.width =
                    NEXT_MULTIPLE(original_width / 4, alignw);
                pp_client->param.ppu_cfg[i].scale.height =
                    NEXT_MULTIPLE(original_height / 4, alignh);
            } else if (pp_client->param.ppu_cfg[i].scale.width == -8 &&
                       pp_client->param.ppu_cfg[i].scale.height == -8) {
                pp_client->param.ppu_cfg[i].scale.width =
                    NEXT_MULTIPLE(original_width / 8, alignw);
                pp_client->param.ppu_cfg[i].scale.height =
                    NEXT_MULTIPLE(original_height / 8, alignh);
            } else {
                VPILOGE("pp %d scale setting error!!!\n", i);
                goto err_exit;
            }
            VPILOGV("pp_client->param.ppu_cfg[%d].scale.width = %d, "
                    "pp_client->param.ppu_cfg[%d].scale.height = %d\n",
                    i, pp_client->param.ppu_cfg[i].scale.width, i,
                    pp_client->param.ppu_cfg[i].scale.height);
        }
    }

    ret      = pp_parse_params(pp_client, &pp_client->param);
    test_cfg = &pp_client->pp_config;
    if (ret < 0) {
        VPILOGE("pp_parse_params failed!\n");
        goto err_exit;
    }

    dwl_init.client_type  = DWL_CLIENT_TYPE_ST_PP;
    dwl_init.dec_dev_name = params->device;
    dwl_init.mem_id       = params->mem_id;
    dwl_init.priority     = params->priority;
    pp_client->dwl        = (void *)DWLInit(&dwl_init);
    if (pp_client->dwl == NULL) {
        VPILOGE("pp DWLInit failed!\n");
        goto err_exit;
    }

    /* initialize decoder. If unsuccessful -> exit */
    ret = PPInit((PPInst *)&pp_client->pp_inst, (const void *)pp_client->dwl);
    pp_inst = pp_client->pp_inst;

    if (ret != PP_OK) {
        VPILOGE("PP INITIALIZATION FAILED\n");
        goto err_exit;
    }

    if (pp_calc_pic_size(pp_client) < 0) {
        VPILOGE("pp_calc_pic_size failed!\n");
        goto err_exit;
    }

    VPILOGD("XALIGN TCACHE size in_width_align=%d, in_stride=%d\n",
            pp_client->pp_config.in_width_align,
            pp_client->pp_config.in_stride);

    /* Override command line options and tb.cfg for PPU0. */
    /* write pp params form tb_cfg.pp_units_params to pp_client->param.ppu_cfg
       or keep cml params - kwu */
    pp_resolve_params_overlap(&pp_client->param, tb_cfg.pp_units_params,
                              !tb_cfg.pp_params.pipeline_e);

#ifdef SUPPORT_CACHE
    /* resolve shaper setting */
    for (i = 0; i < 4; i++) {
        if (!pp_client->param.ppu_cfg[i].enabled)
            continue;
        pp_client->param.ppu_cfg[i].shaper_enabled = test_cfg->shaper_enable;
    }
#endif

    if (pp_client->param.fscale_cfg.fixed_scale_enabled) {
        u32 crop_w = pp_client->param.ppu_cfg[0].crop.width;
        u32 crop_h = pp_client->param.ppu_cfg[0].crop.height;
        if (!crop_w)
            crop_w = test_cfg->in_width;
        if (!crop_h)
            crop_h = test_cfg->in_height;
        pp_client->param.ppu_cfg[0].scale.width =
            (crop_w / pp_client->param.fscale_cfg.down_scale_x) & ~0x1;
        pp_client->param.ppu_cfg[0].scale.height =
            (crop_h / pp_client->param.fscale_cfg.down_scale_y) & ~0x1;
        pp_client->param.ppu_cfg[0].scale.enabled = 1;
        pp_client->param.ppu_cfg[0].enabled       = 1;
    }
    pp_client->pp_enabled = pp_client->param.pp_enabled;

    /* resolve pp align and set crop - kwu add */
    if (test_cfg->in_width_align != test_cfg->in_width ||
        test_cfg->in_height_align != test_cfg->in_height) {
        for (i = 1; i < 4; i++) {
            if (!pp_client->param.ppu_cfg[i].enabled)
                continue;
            if (!pp_client->param.ppu_cfg[i].crop.enabled) {
                pp_client->param.ppu_cfg[i].crop.enabled = 1;
                pp_client->param.ppu_cfg[i].crop.x       = 0;
                pp_client->param.ppu_cfg[i].crop.y       = 0;
                pp_client->param.ppu_cfg[i].crop.width   = test_cfg->in_width;
                pp_client->param.ppu_cfg[i].crop.height  = test_cfg->in_height;
            }
            VPILOGD("[pp] enable pp[%d] crop [%d, %d, %d, %d]\n", i,
                    pp_client->param.ppu_cfg[i].crop.x,
                    pp_client->param.ppu_cfg[i].crop.y,
                    pp_client->param.ppu_cfg[i].crop.width,
                    pp_client->param.ppu_cfg[i].crop.height);
        }
    }
    /* write pp config to dec_cfg->ppu_config - kwu */
    dec_cfg = &pp_client->dec_cfg;
    memcpy(dec_cfg->ppu_config, pp_client->param.ppu_cfg,
           sizeof(pp_client->param.ppu_cfg));

    for (i = 0; i < 4; i++) {
        if (dec_cfg->ppu_config[i].tiled_e && pp_client->param.in_10bit)
            dec_cfg->ppu_config[i].out_cut_8bits = 0;
        if (pp_client->param.in_10bit == 0) {
            dec_cfg->ppu_config[i].out_cut_8bits = 0;
            dec_cfg->ppu_config[i].out_p010      = 0;
        }

        VPILOGV("dec_cfg->ppu_config[%d].crop.width %d, "
                "dec_cfg->ppu_config[%d].crop.height %d \n",
                i, dec_cfg->ppu_config[i].crop.width, i,
                dec_cfg->ppu_config[i].crop.height);
        if (!dec_cfg->ppu_config[i].crop.width ||
            !dec_cfg->ppu_config[i].crop.height) {
            dec_cfg->ppu_config[i].crop.x = dec_cfg->ppu_config[i].crop.y = 0;
            dec_cfg->ppu_config[i].crop.width  = test_cfg->in_width_align;
            dec_cfg->ppu_config[i].crop.height = test_cfg->in_height_align;

            VPILOGV("dec_cfg->ppu_config[%d].crop.width %d, "
                    "dec_cfg->ppu_config[%d].crop.height %d \n",
                    i, dec_cfg->ppu_config[i].crop.width, i,
                    dec_cfg->ppu_config[i].crop.height);
        }

        VPILOGV("dec_cfg->ppu_config[%d].scale.width %d, "
                "dec_cfg->ppu_config[%d].scale.height %d \n",
                i, dec_cfg->ppu_config[i].scale.width, i,
                dec_cfg->ppu_config[i].scale.height);
        if (!dec_cfg->ppu_config[i].scale.width ||
            !dec_cfg->ppu_config[i].scale.height) {
            dec_cfg->ppu_config[i].scale.width =
                dec_cfg->ppu_config[i].crop.width;
            dec_cfg->ppu_config[i].scale.height =
                dec_cfg->ppu_config[i].crop.height;

            VPILOGV("dec_cfg->ppu_config[%d].scale.width %d, "
                    "dec_cfg->ppu_config[%d].scale.height %d \n",
                    i, dec_cfg->ppu_config[i].scale.width, i,
                    dec_cfg->ppu_config[i].scale.height);
        }
    }

    dec_cfg->in_format = test_cfg->in_p010;
    dec_cfg->in_stride = test_cfg->in_stride;
    dec_cfg->in_height = test_cfg->in_height_align;
    dec_cfg->in_width  = test_cfg->in_width_align;

    for (i = 0; i < 4; i++) {
        if (!dec_cfg->ppu_config[i].enabled)
            continue;
    }

    ret = pp_buf_init(pp_client);
    if (ret < 0) {
        VPILOGE("pp_buf_init failed!\n");
        goto err_exit;
    }
    pp_dump_config(pp_client);
    ret = PPSetInfo(pp_inst, dec_cfg);
    if (ret != PP_OK) {
        VPILOGE("encode fails for Invalid pp parameters\n");
        goto err_exit;
    }
    return pp_client;
err_exit:
    vpi_ppclient_release(pp_client);
    return NULL;
}

static void pp_report_pp_pic_info(struct DecPicturePpu *picture)
{
    char info_string[2048];
    static char *pic_types[] = { "IDR", "Non-IDR (P)", "Non-IDR (B)" };

    sprintf(&info_string[0], "pic_id %2d, type %s, ",
            picture->pictures[0].picture_info.pic_id,
            pic_types[picture->pictures[0].picture_info.pic_coding_type]);
    if (picture->pictures[0].picture_info.cycles_per_mb) {
        sprintf(&info_string[strlen(info_string)], " %4d cycles / mb,",
                picture->pictures[0].picture_info.cycles_per_mb);
    }

    sprintf(&info_string[strlen(info_string)],
            " %d x %d, Crop: (%d, %d), %d x %d %s",
            picture->pictures[0].sequence_info.pic_width,
            picture->pictures[0].sequence_info.pic_height,
            picture->pictures[0].sequence_info.crop_params.crop_left_offset,
            picture->pictures[0].sequence_info.crop_params.crop_top_offset,
            picture->pictures[0].sequence_info.crop_params.crop_out_width,
            picture->pictures[0].sequence_info.crop_params.crop_out_height,
            picture->pictures[0].picture_info.is_corrupted ? "CORRUPT" : "");

    VPILOGV("%s\n", info_string);
}

static int pp_output_frame(VpiPPFilter *filter, VpeFrame *out,
                           PPDecPicture *hipc)
{
    int ref_num_0             = 0;
    int ref_num_1             = 0;
    VpiPPParams *params       = &filter->pp_client->param;
    struct DecPicturePpu *pic = malloc(sizeof(struct DecPicturePpu));

    if (!pic)
        return -1;

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("PP force memory error in function\n");
        goto err_exit;
    }
#endif

    memcpy(pic, &hipc->pp_pic, sizeof(struct DecPicturePpu));
    pp_report_pp_pic_info(pic);

    out->width       = pic->pictures[0].pic_width;
    out->height      = pic->pictures[0].pic_height;
    out->linesize[0] = pic->pictures[0].pic_width;
    out->linesize[1] = pic->pictures[0].pic_width / 2;
    out->linesize[2] = pic->pictures[0].pic_width / 2;
    out->key_frame =
        (pic->pictures[0].picture_info.pic_coding_type == DEC_PIC_TYPE_I);

    /*always using pp0, never pp1*/
    pic->pictures[0].pp_enabled = 1;
    pic->pictures[1].pp_enabled = 0;

    for (int i = 2; i < 5; i++) {
        if (params->ppu_cfg[i - 1].enabled == 1) {
            pic->pictures[i].pp_enabled = 1;
        } else {
            pic->pictures[i].pp_enabled = 0;
        }
    }

    for (int i = 0; i < 5; i++)
        VPILOGV("pic.pictures[%d].pp_enabled = "
                "%d,comperss_status=%d,bit_depth_luma=%d\n",
                i, pic->pictures[i].pp_enabled,
                pic->pictures[i].pic_compressed_status,
                pic->pictures[i].sequence_info.bit_depth_luma);

    VPILOGV("fb_dec_ctx->picRdy = 1\n");

    out->pic_info[0].enabled = 1;
    if (params->ppu_cfg[0].scale.width != 0 &&
        params->ppu_cfg[0].scale.height != 0) {
        out->pic_info[0].width  = params->ppu_cfg[0].scale.width;
        out->pic_info[0].height = params->ppu_cfg[0].scale.height;
    } else {
        out->pic_info[0].width  = out->width;
        out->pic_info[0].height = out->height;
    }

    VPILOGV("out->pic_info[0].width %d, out->pic_info[0].height %d\n",
            out->pic_info[0].width, out->pic_info[0].height);

    out->pic_info[0].picdata.is_interlaced =
        pic->pictures[0].sequence_info.is_interlaced;
    out->pic_info[0].picdata.pic_stride = pic->pictures[0].pic_stride;

    out->pic_info[0].picdata.pic_format = pic->pictures[0].picture_info.format;
    out->pic_info[0].picdata.pic_pixformat =
        pic->pictures[0].picture_info.pixel_format;
    out->pic_info[0].picdata.bit_depth_luma =
        pic->pictures[0].sequence_info.bit_depth_luma;
    out->pic_info[0].picdata.bit_depth_chroma =
        pic->pictures[0].sequence_info.bit_depth_chroma;
    out->pic_info[0].picdata.pic_compressed_status =
        pic->pictures[0].pic_compressed_status;

    if (pic->pictures[1].pp_enabled == 1) {
        out->pic_info[1].enabled = 1;
        out->pic_info[1].width   = pic->pictures[0].pic_width;
        out->pic_info[1].height  = pic->pictures[0].pic_height;
    } else {
        out->pic_info[1].enabled = 0;
        out->pic_info[1].width   = pic->pictures[0].pic_width;
        out->pic_info[1].height  = pic->pictures[0].pic_height;
    }

    for (int i = 2; i < 5; i++) {
        if (params->ppu_cfg[i - 1].enabled == 1) {
            out->pic_info[i].enabled = 1;
            out->pic_info[i].width   = params->ppu_cfg[i - 1].scale.width;
            out->pic_info[i].height  = params->ppu_cfg[i - 1].scale.height;
            out->pic_info[i].picdata.is_interlaced =
                pic->pictures[i].sequence_info.is_interlaced;
            out->pic_info[i].picdata.pic_stride = pic->pictures[i].pic_stride;
            out->pic_info[i].picdata.crop_out_width =
                pic->pictures[i].sequence_info.crop_params.crop_out_width;
            out->pic_info[i].picdata.crop_out_height =
                pic->pictures[i].sequence_info.crop_params.crop_out_height;
            out->pic_info[i].picdata.pic_format =
                pic->pictures[i].picture_info.format;
            out->pic_info[i].picdata.pic_pixformat =
                pic->pictures[i].picture_info.pixel_format;
            out->pic_info[i].picdata.bit_depth_luma =
                pic->pictures[i].sequence_info.bit_depth_luma;
            out->pic_info[i].picdata.bit_depth_chroma =
                pic->pictures[i].sequence_info.bit_depth_chroma;
            out->pic_info[i].picdata.pic_compressed_status =
                pic->pictures[i].pic_compressed_status;
        }
    }
    VPILOGV("out->pic_info[0].picdata.pic_compressed_status = %d.\n",
            out->pic_info[0].picdata.pic_compressed_status);

    VPILOGV("output "
            "frames:[%d(%dx%d)][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)]\n",
            out->pic_info[0].enabled, out->pic_info[0].width,
            out->pic_info[0].height, out->pic_info[1].enabled,
            out->pic_info[1].width, out->pic_info[1].height,
            out->pic_info[2].enabled, out->pic_info[2].width,
            out->pic_info[2].height, out->pic_info[3].enabled,
            out->pic_info[3].width, out->pic_info[3].height,
            out->pic_info[4].enabled, out->pic_info[4].width,
            out->pic_info[4].height);

    if (filter->vce_ds_enable) {
        if (!out->pic_info[0].enabled || !out->pic_info[2].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, pp0 and pp1 should be "
                    "enabled!\n");
            goto err_exit;
        }
        if (out->pic_info[1].enabled || out->pic_info[3].enabled ||
            out->pic_info[4].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, except pp0 and pp1 should "
                    "not be enabled!\n");
            goto err_exit;
        }
        VPILOGV("set flag to 1\n");
        out->pic_info[2].flag = 1;
    }

    out->data[0]         = (uint8_t *)pic;
    out->pic_struct_size = sizeof(struct DecPicturePpu);

#ifdef PP_MEM_ERR_TEST
    if (pp_memory_check() != 0) {
        VPILOGE("PP force memory error in function\n");
        goto err_exit;
    }
#endif
    return 0;

err_exit:
    return -1;
}

static int pp_trans_formats(VpePixFmt format, char **pp_format, int *in_10bit,
                            int force_10bit)
{
    if (format == VPE_FMT_YUV420P) {
        *pp_format = "yuv420p";
        *in_10bit  = 0;
    } else if (format == VPE_FMT_YUV422P) {
        *pp_format = "yuv422p";
        *in_10bit  = 0;
    } else if (format == VPE_FMT_NV12) {
        *pp_format = "nv12";
        *in_10bit  = 0;
    } else if (format == VPE_FMT_NV21) {
        *pp_format = "nv21";
        *in_10bit  = 0;
    } else if (format == VPE_FMT_YUV420P10LE) {
        *pp_format = "yuv420p10le";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_YUV420P10BE) {
        *pp_format = "yuv420p10be";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_YUV422P10LE) {
        *pp_format = "yuv422p10le";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_YUV422P10BE) {
        *pp_format = "yuv422p10be";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_P010LE) {
        *pp_format = "p010le";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_P010BE) {
        *pp_format = "p010be";
        *in_10bit  = 1;
    } else if (format == VPE_FMT_YUV444P) {
        *pp_format = "yuv444p";
        *in_10bit  = 0;
    } else if (format == VPE_FMT_RGB24) {
        *pp_format = "rgb24";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else if (format == VPE_FMT_BGR24) {
        *pp_format = "bgr24";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else if (format == VPE_FMT_ARGB) {
        *pp_format = "argb";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else if (format == VPE_FMT_RGBA) {
        *pp_format = "rgba";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else if (format == VPE_FMT_ABGR) {
        *pp_format = "abgr";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else if (format == VPE_FMT_BGRA) {
        *pp_format = "bgra";
        *in_10bit  = 0;
        if (force_10bit)
            *in_10bit = 1;
    } else {
        VPILOGE("Transcoder unsupport input format %d!!!\n", format);
        return -1;
    }
    return 0;
}

static void pp_setup_defaul_params(VpiPPParams *params)
{
    memset(params, 0, sizeof(VpiPPParams));
    params->read_mode               = STREAMREADMODE_FRAME;
    params->fscale_cfg.down_scale_x = 1;
    params->fscale_cfg.down_scale_y = 1;
    params->is_ringbuffer           = 1;
    params->display_cropped         = 0;
    params->tile_by_tile            = 0;
    params->mc_enable               = 0;
    params->device                  = "/dev/transcoder0";
    params->mem_id                  = 1;
    params->priority                = 0;
    memset(params->ppu_cfg, 0, sizeof(params->ppu_cfg));
}

static int pp_set_params(VpiPPFilter *filter)
{
    int ret             = 0;
    int pp_index        = 0;
    VpiPPParams *params = &filter->params;

    /* check low_res and output numbers */
    if (filter->nb_outputs < 1 || filter->nb_outputs > 4) {
        VPILOGE("outputs number error\n");
        return -1;
    }

    if (filter->nb_outputs == 1 && filter->low_res_num == 1 &&
        (filter->resizes[0].x == 0 && filter->resizes[0].y == 0 &&
         filter->resizes[0].cw == 0 && filter->resizes[0].ch == 0 &&
         filter->resizes[0].sw == -2 && filter->resizes[0].sh == -2)) {
        VPILOGD("set vce_ds_enable to 1\n");

        filter->vce_ds_enable = 1;
        filter->resizes[1]    = filter->resizes[0];
        filter->resizes[0].x = filter->resizes[0].y = filter->resizes[0].cw =
            filter->resizes[0].ch                   = filter->resizes[0].sw =
                filter->resizes[0].sh               = 0;
        filter->low_res_num++;

    } else if (filter->nb_outputs == 1 && filter->low_res_num == 2) {
        if (!(filter->resizes[0].x == filter->resizes[1].x &&
              filter->resizes[0].y == filter->resizes[1].y &&
              filter->resizes[0].cw == filter->resizes[1].cw &&
              filter->resizes[0].ch == filter->resizes[1].ch &&
              filter->resizes[1].sw == -2 && filter->resizes[1].sh == -2)) {
            VPILOGE("low_res param error!\n");
            return -1;
        }

        filter->vce_ds_enable = 1;

    } else if (filter->nb_outputs == filter->low_res_num + 1) {
        if (filter->low_res_num) {
            for (int i = filter->nb_outputs - 1; i > 0; i--) {
                filter->resizes[i] = filter->resizes[i - 1];
            }
        }

        filter->resizes[0].x = filter->resizes[0].y = filter->resizes[0].cw =
            filter->resizes[0].ch                   = filter->resizes[0].sw =
                filter->resizes[0].sh               = 0;

        filter->low_res_num++;

    } else if (filter->nb_outputs != filter->low_res_num) {
        VPILOGE("low_res param error!\n");
        return -1;
    }

    VPILOGV("pp decoder resizes info summay:\n");
    for (int i = 0; i < filter->low_res_num; i++) {
        VPILOGV("low_res %d : (%d,%d, %d,%d, %dx%d)\n", i, filter->resizes[i].x,
                filter->resizes[i].y, filter->resizes[i].cw,
                filter->resizes[i].ch, filter->resizes[i].sw,
                filter->resizes[i].sh);
    }

    pp_setup_defaul_params(params);
    params->width  = filter->w;
    params->height = filter->h;
    ret            = pp_trans_formats(filter->format, &params->in_format,
                           &params->in_10bit, filter->force_10bit);
    if (ret < 0) {
        VPILOGE("pp_trans_formats failed ret=%d\n", ret);
        return -1;
    }
    params->align           = DEC_ALIGN_1024B;
    params->compress_bypass = 0;
    params->cache_enable = 1, params->shaper_enable = 1,

    /* 17 buffers as default value, same to decoder and the buffer_depth is inc
     * value.*/
        params->ext_buffers_need = 17 + filter->buffer_depth;

    /* set options to pp params */
    params->pp_enabled                = 1;
    params->ppu_cfg[0].enabled        = 1;
    params->ppu_cfg[0].tiled_e        = 1;
    params->ppu_cfg[0].out_p010       = params->in_10bit;
    params->ppu_cfg[0].align          = 10;
    params->ppu_cfg[0].shaper_enabled = 1;

    VPILOGV("[%s, %d], pp test Params summary,width %d, height %d, in_format "
            "%s, in_10bit %d, format %d\n",
            __FUNCTION__, __LINE__, params->width, params->height,
            params->in_format, params->in_10bit, filter->format);

    for (int i = 0; i < filter->low_res_num; i++) {
        if (filter->resizes[i].x == 0 && filter->resizes[i].y == 0 &&
            filter->resizes[i].cw == 0 && filter->resizes[i].ch == 0 &&
            filter->resizes[i].sw == 0 && filter->resizes[i].sh == 0) {
            continue;
        }
        VPILOGV("pp %d low_res %d get param\n", i, i);
        pp_index                          = i;
        params->ppu_cfg[pp_index].enabled = 1;
        params->ppu_cfg[pp_index].tiled_e = 1;

        /* crop params */
        if (!(filter->resizes[i].x == 0 && filter->resizes[i].y == 0 &&
              filter->resizes[i].cw == 0 && filter->resizes[i].ch == 0)) {
            if (!(params->width && params->height &&
                  filter->resizes[i].x == 0 && filter->resizes[i].y == 0 &&
                  filter->resizes[i].cw == params->width &&
                  filter->resizes[i].ch == params->height)) {
                params->ppu_cfg[pp_index].crop.enabled = 1;
                params->ppu_cfg[pp_index].crop.x       = filter->resizes[i].x;
                params->ppu_cfg[pp_index].crop.y       = filter->resizes[i].y;
                params->ppu_cfg[pp_index].crop.width   = filter->resizes[i].cw;
                params->ppu_cfg[pp_index].crop.height  = filter->resizes[i].ch;
            }
        }
        /* scale params*/
        if (!(filter->resizes[i].sw == 0 && filter->resizes[i].sh == 0)) {
            params->ppu_cfg[pp_index].scale.enabled = 1;
            params->ppu_cfg[pp_index].scale.width   = filter->resizes[i].sw;
            params->ppu_cfg[pp_index].scale.height  = filter->resizes[i].sh;
        }
        params->ppu_cfg[pp_index].out_p010       = params->in_10bit;
        params->ppu_cfg[pp_index].align          = 10;
        params->ppu_cfg[pp_index].shaper_enabled = 1;

        VPILOGV("ppu_cfg[%d]:enabled %d, tiled_e %d, out_p010 %d, align %d, "
                "shaper_enabled %d, crop(enabled %d, %d %d, %dx%d), "
                "scale(enabled %d, %dx%d)\n",
                pp_index, params->ppu_cfg[pp_index].enabled,
                params->ppu_cfg[pp_index].tiled_e,
                params->ppu_cfg[pp_index].out_p010,
                params->ppu_cfg[pp_index].align,
                params->ppu_cfg[pp_index].shaper_enabled,
                params->ppu_cfg[pp_index].crop.enabled,
                params->ppu_cfg[pp_index].crop.x,
                params->ppu_cfg[pp_index].crop.y,
                params->ppu_cfg[pp_index].crop.width,
                params->ppu_cfg[pp_index].crop.height,
                params->ppu_cfg[pp_index].scale.enabled,
                params->ppu_cfg[pp_index].scale.width,
                params->ppu_cfg[pp_index].scale.height);
    }
    return 0;
}

static int pp_set_hwframe_res(VpiPPFilter *filter)
{
    VpeFrame *frame       = filter->frame;
    PpUnitConfig *ppu_cfg = filter->pp_client->dec_cfg.ppu_config;
    VpiPPParams *params   = &filter->params;

    frame->pic_info[0].enabled = 1;
    dump_ppuconfig(NULL, ppu_cfg);
    if (ppu_cfg[0].scale.width < params->width) {
        frame->pic_info[0].width = ppu_cfg[0].scale.width;
    } else {
        frame->pic_info[0].width = params->width;
    }
    if (ppu_cfg[0].scale.height < params->height) {
        frame->pic_info[0].height = ppu_cfg[0].scale.height;
    } else {
        frame->pic_info[0].height = params->height;
    }

    frame->pic_info[0].pic_width             = ppu_cfg[0].scale.width;
    frame->pic_info[0].pic_height            = ppu_cfg[0].scale.height;
    frame->pic_info[0].picdata.is_interlaced = 0;
    frame->pic_info[0].picdata.pic_format    = 0;
    if (params->in_10bit) {
        frame->pic_info[0].picdata.pic_pixformat = 1;
        frame->pic_info[0].picdata.bit_depth_luma =
            frame->pic_info[0].picdata.bit_depth_chroma = 10;
    } else {
        frame->pic_info[0].picdata.pic_pixformat = 0;
        frame->pic_info[0].picdata.bit_depth_luma =
            frame->pic_info[0].picdata.bit_depth_chroma = 8;
    }
    frame->pic_info[0].picdata.pic_compressed_status = 2;
    frame->pic_info[1].enabled                       = 0;
    for (int i = 2; i < 5; i++) {
        if (params->ppu_cfg[i - 1].enabled == 1) {
            frame->pic_info[i].enabled = 1;
            frame->pic_info[i].width   = ppu_cfg[i - 1].scale.width;
            frame->pic_info[i].height  = ppu_cfg[i - 1].scale.height;

            frame->pic_info[i].picdata.is_interlaced = 0;
            frame->pic_info[i].picdata.pic_format    = 0;
            if (params->in_10bit) {
                frame->pic_info[i].picdata.pic_pixformat = 1;
                frame->pic_info[i].picdata.bit_depth_luma =
                    frame->pic_info[i].picdata.bit_depth_chroma = 10;
            } else {
                frame->pic_info[i].picdata.pic_pixformat = 0;
                frame->pic_info[i].picdata.bit_depth_luma =
                    frame->pic_info[i].picdata.bit_depth_chroma = 8;
            }

            frame->pic_info[i].picdata.pic_compressed_status = 2;
        }
    }

    VPILOGV("output "
            "frames:[%d(%dx%d)][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)][%d(%dx%d)]\n",
            frame->pic_info[0].enabled, frame->pic_info[0].width,
            frame->pic_info[0].height, frame->pic_info[1].enabled,
            frame->pic_info[1].width, frame->pic_info[1].height,
            frame->pic_info[2].enabled, frame->pic_info[2].width,
            frame->pic_info[2].height, frame->pic_info[3].enabled,
            frame->pic_info[3].width, frame->pic_info[3].height,
            frame->pic_info[4].enabled, frame->pic_info[4].width,
            frame->pic_info[4].height);

    if (filter->vce_ds_enable) {
        if (!frame->pic_info[0].enabled || !frame->pic_info[2].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, pp0 and pp1 should be "
                    "enabled!\n");
            return -1;
        }
        if (frame->pic_info[1].enabled || frame->pic_info[3].enabled ||
            frame->pic_info[4].enabled) {
            VPILOGE("When use 1/4 ds pass1 for vce, except pp0 and pp1 should "
                    "not be enabled!\n");
            return -1;
        }
        frame->pic_info[2].flag = 1;
    }

    return 0;
}

static int pp_config_props(VpiPPFilter *filter, VpePPOpition *cfg)
{
    int ret = 0;

    /* get iput from ffmpeg*/
    filter->nb_outputs  = cfg->nb_outputs;
    filter->low_res     = cfg->low_res;
    filter->force_10bit = cfg->force_10bit;
    filter->w           = cfg->w;
    filter->h           = cfg->h;
    filter->format      = cfg->format;
    filter->frame       = (VpeFrame *)cfg->frame;

    ret = pp_parse_low_res(filter);
    if (ret < 0) {
        return -1;
    }

    ret = pp_set_params(filter);
    if (ret < 0) {
        return -1;
    }
    filter->mwl = pp_trans_demuxer_init(&filter->params);
    if (!filter->mwl) {
        return -1;
    }

    filter->inst = pp_raw_parser_open(filter->format, filter->params.width,
                                      filter->params.height);
    if (!filter->inst) {
        return -1;
    }

    filter->pp_client = pp_client_init(&filter->params);
    if (!filter->pp_client) {
        return -1;
    }

    ret = pp_set_hwframe_res(filter);
    if (ret < 0) {
        return -1;
    }

    ret = vpi_pp_input_buf_init(filter);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

static int pp_picture_consumed(VpiPPFilter *filter,
                               struct DecPicturePpu *picture)
{
    PPClient *pp = filter->pp_client;
    int index = 0;
    u32 i;

#ifdef SUPPORT_TCACHE
    index = 1;
#endif

    if (pp == NULL) {
        VPILOGE("PP is NULL\n");
        return -1;
    }

    for (i = 0; i < pp->out_buf_nums; i++) {
        if (pp->pp_out_buffer[i].bus_address ==
            picture->pictures[index].luma.bus_address) {
            VPILOGD("push buff %d...\n", i);
            FifoPush(pp->pp_out_Fifo, &pp->pp_out_buffer[i],
                     FIFO_EXCEPTION_DISABLE);
        }
    }
    free(picture);
    return 0;
}

static void pp_print_dec_picture(PPDecPicture *dec_picture, int num)
{
    char info_string[2048];
    int i;

    sprintf(&info_string[0], "PIC %2d", num++);
    for (i = 0; i < 4; i++) {
        if (dec_picture->pictures[i].pp_enabled) {
            sprintf(&info_string[strlen(info_string)], ", %d : %d x %d", i,
                    dec_picture->pictures[i].pic_width,
                    dec_picture->pictures[i].pic_height);
        }
    }
    VPILOGD("%s\n", info_string);
}

VpiRet vpi_prc_pp_init(VpiPrcCtx *vpi_ctx, void *cfg)
{
    return VPI_SUCCESS;
}

VpiRet vpi_prc_pp_control(VpiPrcCtx *vpi_ctx, void *indata, void *outdata)
{
    VpeCtrlCmdParam *cmd          = (VpeCtrlCmdParam *)indata;
    VpiPPFilter *filter           = &vpi_ctx->ppfilter;
    VpePPOpition *cfg             = NULL;
    struct DecPicturePpu *picture = NULL;
    int ret                       = 0;

    switch (cmd->cmd) {
    case VPE_CMD_PP_CONFIG:
        cfg = (VpePPOpition *)cmd->data;
        ret = pp_config_props(filter, cfg);
        if (ret < 0) {
            VPILOGE("VPE_CMD_PP_CONFIG failed=%d\n", ret);
        }
        break;
    case VPE_CMD_PP_CONSUME:
        picture = cmd->data;
        ret     = pp_picture_consumed(filter, picture);
        if (ret < 0) {
            VPILOGE("VPE_CMD_PP_CONSUME failed=%d\n", ret);
        }
        break;
    default:
        VPILOGE("Not supported command %d for PP\n", cmd->cmd);
        break;
    }
    return ret;
}

VpiRet vpi_prc_pp_process(VpiPrcCtx *vpi_ctx, void *indata, void *outdata)
{
    VpiPPFilter *filter = &vpi_ctx->ppfilter;
    VpeFrame *input     = (VpeFrame *)indata;
    VpeFrame *output    = (VpeFrame *)outdata;
    PPClient *pp        = filter->pp_client;
    PPDecPicture dec_picture;
    VpiPPConfig *test_cfg     = &pp->pp_config;
    PPInst pp_inst            = NULL;
    PPConfig *dec_cfg         = &pp->dec_cfg;
    PPContainer *pp_c         = NULL;
    static u32 decode_pic_num = 0;
    int ret                   = 0;

    if (!pp || !pp->pp_inst) {
        VPILOGE("PP was not inited\n");
        return 0;
    }

    pp_inst = pp->pp_inst;
    pp_c    = (PPContainer *)pp_inst;

    /* read frame from VpeFrame to pp_in_buffer*/
    pp_send_packet(filter, &pp->pp_in_buffer, input);
    if (pp->pp_in_buffer == NULL) {
        VPILOGE("[%s@%d], pp_send_packet failed!", __FUNCTION__, __LINE__);
        goto err_exit;
    }
    dec_cfg->pp_in_buffer = pp->pp_in_buffer->buffer;

#ifdef SUPPORT_TCACHE
    if (pp->tcache_handle == NULL) {
        pp->tcache_handle =
            DWLGetIpHandleByOffset(pp_c->dwl, TCACHE_OFFSET_TO_VCEA);
        if (pp->tcache_handle == NULL) {
            VPILOGE("failed get tcache handle!\n");
            goto err_exit;
        }
    }
    if (pp->edma_handle == NULL) {
        pp->edma_handle = DWLGetIpHandleByOffset(pp_c->dwl, PCIE_EDMA_REG_BASE -
                                                                PCIE_REG_START);
        if (pp->edma_handle == NULL) {
            VPILOGE("failed get edma handle!\n");
            goto err_exit;
        }
    }

    ret = pp_tcache_config(pp);
    if (ret < 0)
        goto err_exit;
#endif

#ifdef PP_EDMA_ERR_TEST
    if (pp_edma_eerror_check() != 0) {
        VPILOGE("[%s,%d]PP force edma error in function\n", __FUNCTION__,
                __LINE__);
        goto err_exit;
    }
#endif

    ret = PPSetInput(pp_inst, pp->dec_cfg.pp_in_buffer);
    if (ret != PP_OK) {
        VPILOGE("encode fails for PPSetInput\n");
        goto err_exit;
    }
    pp_request_buf(pp);
    ret = PPSetOutput(pp_inst, pp->dec_cfg.pp_out_buffer);
    if (ret != PP_OK) {
        VPILOGE("encode fails for PPSetOutput\n");
        goto err_exit;
    }

    pp_dump_config(pp);
    ret = PPDecode(pp_inst);
    if (ret != PP_OK) {
        VPILOGE("encode fails for PPDecode failed, %d\n", ret);
        goto err_exit;
    }

    ret = PPNextPicture(pp_inst, &dec_picture);
    pp_get_next_pic(pp, &dec_picture);
    pp_print_dec_picture(&dec_picture, decode_pic_num++);
    ret = pp_output_frame(filter, output, &dec_picture);
    pp->num_of_output_pics++;
    return ret;

err_exit:
    return -1;
}

VpiRet vpi_prc_pp_close(VpiPrcCtx *ctx)
{
    VpiPPFilter *filter = &ctx->ppfilter;
    PPClient *pp_client = filter->pp_client;
    int ref_num_0       = 0;
    int ref_num_1       = 0;
    int i;

    if (pp_client != NULL) {
        pp_buf_release(pp_client);
        PPRelease(pp_client->pp_inst);

        if (pp_client->dwl)
            DWLRelease(pp_client->dwl);

        free(pp_client);
        filter->pp_client = NULL;
    }

    if (filter->mwl != NULL) {
        pp_mwl_free_linear(filter->mwl, &filter->buffers.buffer);
    }
    if (filter->mwl != NULL) {
        pp_mwl_release(filter->mwl);
        filter->mwl = NULL;
    }
    return VPI_SUCCESS;
}
