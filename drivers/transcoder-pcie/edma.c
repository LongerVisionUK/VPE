// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 Verisilicon Inc.
 *
 * This is edma transfer driver for Linux.
 */

#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>
#include <linux/pagemap.h>
#include <linux/firmware.h>
#include <linux/pci.h>

#include "common.h"
#include "edma.h"
#include "transcoder.h"

/* unit HZ */
#ifndef EMULATOR
#define EDMA_TIMEOUT	(1*HZ)
#else
#define EDMA_TIMEOUT	(120*HZ)
#endif

#define BAKUP_SIZE 0x4000

/*  DMA_CH_CONTROL_REG    */
/* Link and interrupt bit define */
#define DMA_CB				(1<<0)
#define DMA_TCB				(1<<1)
#define DMA_LLP				(1<<2)
#define DMA_LIE				(1<<3)
#define DMA_RIE				(1<<4)
#define DMA_CCS				(1<<8)
#define DMA_LLE				(1<<9)

/* dma TLP relative define */
#define DMA_FUNC_NUM(x)			((x)<<12)
#define DMA_NS_DST			(1<<23)
#define DMA_NS_SRC			(1<<24)
#define DMA_RO				(1<<25)
#define DMA_TD				(1<<26)
#define DMA_TC(x)			((x)<<27)
#define DMA_AT(x)			((x)<<30)

/* channel status CS */
#define DMA_CS_RUNNING			(0x01<<5)
#define DMA_CS_HALTED			(0x10<<5)
#define DMA_CS_STOPPED			(0x11<<5)

#define DMA_ENGINE_EN			0x1
#define DMA_HANDSHAKE			0x10000
#define MASK_ALL_INTE			0xFF00FF
#define CLEAR_ALL_INTE			0xFF00FF

#define DMA_CTRL_DATA_ARB_PRIOR_OFF		0x1000
#define DMA_CTRL_OFF				0x1008
#define DMA_WRITE_ENGINE_EN_OFF			0x100c
#define DMA_WRITE_DOORBELL_OFF			0x1010
#define DMA_WRITE_CHANNEL_ARB_WEIGHT_LOW_OFF	0x1018
#define DMA_WRITE_CHANNEL_ARB_WEIGHT_HIGH_OFF	0x101c

#define DMA_READ_ENGINE_EN_OFF			0x102c
#define DMA_READ_DOORBELL_OFF			0x1030
#define DMA_READ_CHANNEL_ARB_WEIGHT_LOW_OFF	0x1038
#define DMA_READ_CHANNEL_ARB_WEIGHT_HIGH_OFF	0x103c

#define DMA_WRITE_INT_STATUS_OFF		0x104c
#define DMA_WRITE_INT_MASK_OFF			0x1054
#define DMA_WRITE_INT_CLEAR_OFF			0x1058
#define DMA_WRITE_ERR_STATUS_OFF		0x105c
#define DMA_WRITE_DONE_IMWR_LOW_OFF		0x1060
#define DMA_WRITE_DONE_IMWR_HIGH_OFF		0x1064
#define DMA_WRITE_ABORT_IMWR_LOW_OFF		0x1068
#define DMA_WRITE_ABORT_IMWR_HIGH_OFF		0x106c
#define DMA_WRITE_CH01_IMWR_DATA_OFF		0x1070
#define DMA_WRITE_CH23_IMWR_DATA_OFF		0x1074
#define DMA_WRITE_CH45_IMWR_DATA_OFF		0x1078
#define DMA_WRITE_CH67_IMWR_DATA_OFF		0x107c
#define DMA_WRITE_LINKED_LIST_ERR_EN_OFF	0x1090

#define DMA_READ_INT_STATUS_OFF			0x10a0
#define DMA_READ_INT_MASK_OFF			0x10a8
#define DMA_READ_INT_CLEAR_OFF			0x10ac
#define DMA_READ_ERR_STATUS_LOW_OFF		0x10b4
#define DMA_READ_ERR_STATUS_HIGH_OFF		0x10b8
#define DMA_READ_LINKED_LIST_ERR_EN_OFF		0x10c4
#define DMA_READ_DONE_IMWR_LOW_OFF		0x10cc
#define DMA_READ_DONE_IMWR_HIGH_OFF		0x10d0
#define DMA_READ_ABORT_IMWR_LOW_OFF		0x10d4
#define DMA_READ_ABORT_IMWR_HIGH_OFF		0x10d8
#define DMA_READ_CH01_IMWR_DATA_OFF		0x10dc
#define DMA_READ_CH23_IMWR_DATA_OFF		0x10e0
#define DMA_READ_CH45_IMWR_DATA_OFF		0x10e4
#define DMA_READ_CH67_IMWR_DATA_OFF		0x10e8

#define DMA_WRITE_ENGINE_HSHAKE_CNT_LOW_OFF	0x1108
#define DMA_WRITE_ENGINE_HSHAKE_CNT_HIGH_OFF	0x110c

#define DMA_READ_ENGINE_HSHAKE_CNT_LOW_OFF	0x1118
#define DMA_READ_ENGINE_HSHAKE_CNT_HIGH_OFF	0x111c

#define DMA_CH_CTL1_WR_OFF			0x1200
#define DMA_XFER_SIZE_WR_OFF			0x1208
#define DMA_SAR_LOW_WRCH_OFF			0x120c
#define DMA_SAR_HIGH_WRCH_OFF			0x1210
#define DMA_DAR_LOW_WRCH_OFF			0x1214
#define DMA_DAR_HIGH_WRCH_OFF			0x1218
#define DMA_LLP_LOW_WR_OFF			0x121c
#define DMA_LLP_HIGH_WR_OFF			0x1220

#define DMA_CH_CTL1_RD_OFF			0x1300
#define DMA_XFER_SIZE_RD_OFF			0x1308
#define DMA_SAR_LOW_RD_OFF			0x130c
#define DMA_SAR_HIGH_RD_OFF			0x1310
#define DMA_DAR_LOW_RD_OFF			0x1314
#define DMA_DAR_HIGH_RD_OFF			0x1318
#define DMA_LLP_LOW_RD_OFF			0x131c
#define DMA_LLP_HIGH_RD_OFF			0x1320

#define DMA_CHN(c)			((c)*0x200)
#define DMA_DONE(c)			(1<<(c))
#define DMA_ABORT(c)			(1<<(16+(c)))
#define IMWR_OFF(i)			(((i)>>1)*0x4)

#define DMA_BASE_ADDR		0x1000
#define DMA_WR_BASE		0x1200
#define DMA_RD_BASE		0x1300

#define ABORT_INT_STATUS	0xFF0000

/* record edma transmission size over a period of time(PCIE_BW_TIMER)
 * then use this size to calculate the pcie bandwidth roughly.
 */
#define PCIE_BW_TIMER		(1*HZ)

/*
 * ELTA is edma link table address in slice0 ddr(ep side). There are 4 rc2ep
 * channels and 4 ep2rc channels,total is 8. every channel have EDMA_LT_SIZE
 * space to save link table.
 * sequence: rc2ep0,rc2ep1,rc2ep2,rc2ep3,ep2rc0,ep2rc1,ep2rc2,ep2rc3
 */
#define ELTA			0x4000000
#define EDMA_LT_SIZE		0x40000

/* mark edma chanels status, EDMA_BUSY: has been reserved, EDMA_BUSY: idle */
#define EDMA_BUSY		0x1
#define EDMA_FREE		0x0

/* link table offset */
#define LT_OFF(c)	((c)*EDMA_LT_SIZE)
/* new link table offset, it's used in transmission for tcache */
#define NT_OFF		0x20000

/* the max element count for tcache */
#define TC_MAX		192
/*
 * This struct record edma performance detail information.
 * Statistics edma performance by direction separation, two atomic variables
 * real-time size, two regular ints record last second of bandwidth.
 * @rc2ep_size: rc2ep transmission size, util is byte.
 * @ep2rc_size: ep2rc transmission size, util is byte.
 * @rc2ep_per: rc2ep edma transmission performance, util is MB/s.
 * @ep2rc_per: ep2rc edma transmission performance, util is MB/s.
 */
struct edma_perf {
	atomic64_t rc2ep_size;
	atomic64_t ep2rc_size;
	unsigned int rc2ep_per;
	unsigned int ep2rc_per;
};

struct backup_info {
	int tb_size;
	struct dma_link_table *tab_inf;
};

/*
 * The edma_t structure describes edma module.
 * @vedma_lt: edma link table virtual address.
 * @queue_wait: Waiting for edma interruption.
 * @tcache_lock[2]: protect getting tcache channel.
 * @wait_condition_r[4]: the event to wait for, using in rc2ep.
 * @wait_condition_w[4]: the event to wait for, using in ep2rc.
 * @readback_r[4]: only is a flag, ensure link table has been write to ddr.
 * @readback_w[4]: only is a flag, ensure link table has been write to ddr.
 * @rc2ep_cs[4]: record rc2ep channel status.
 * @ep2rc_cs[4]: record ep2rc channel status.
 * @ep2rc_sem: protect get/free edma channel, rc2ep direction.
 * @rc2ep_sem: protect get/free edma channel, ep2rc direction.
 * @rc2ep_cs_lock: protect get/free edma channel, rc2ep direction.
 * @ep2rc_cs_lock: protect get/free edma channel, ep2rc direction.
 * @edma_perf: record edma performance data.
 * @perf_timer: it is a timer, calculate performance periodically.
 * @rc2ep_cfg_lock: protect setting rc2ep edma registers.
 * @ep2rc_cfg_lock: protect setting ep2rc edma registers.
 * @edma_irq_lock: used in edma interrupt handle.
 * @tdev: record struct cb_tranx_t point.
 * @backup_rc2ep: for debug, save link table.
 * @backup_ep2rc: for debug, save link table.
 */
struct edma_t {
	void __iomem *vedma_lt;
	wait_queue_head_t queue_wait;
	struct mutex tcache_lock[2];
	u8 wait_condition_r[4];
	u8 wait_condition_w[4];
	u32 tcache_link_size[2];
	u32 readback_r[4];
	u32 readback_w[4];
	u8 rc2ep_cs[4];
	u8 ep2rc_cs[4];
	struct semaphore ep2rc_sem;
	struct semaphore rc2ep_sem;
	spinlock_t rc2ep_cs_lock;
	spinlock_t ep2rc_cs_lock;
	struct edma_perf edma_perf;
	struct timer_list perf_timer;
	spinlock_t rc2ep_cfg_lock;
	spinlock_t ep2rc_cfg_lock;
	spinlock_t edma_irq_lock;
	struct cb_tranx_t *tdev;
	struct backup_info backup_rc2ep[4];
	struct backup_info backup_ep2rc[4];
};

struct rc_addr_info {
	dma_addr_t paddr;
	unsigned int size;
};

struct dw_edma_llp {
	u32 control;
	u32 reserved;
	u32 llp_low;
	u32 llp_high;
};

static inline void edma_write(struct cb_tranx_t *tdev,
				 unsigned int addr, unsigned int val)
{
	writel((val), ((addr)+tdev->bar0_virt));
}

static inline unsigned int edma_read(struct cb_tranx_t *tdev,
					unsigned int addr)
{
	return readl((addr)+(tdev->bar0_virt));
}

/* when edma transfer time out, dump all edma registers. */
static void dump_edma_regs(struct cb_tranx_t *tdev, char *dir, int channel)
{
	unsigned int val;
	int i, j;

	for (i = 0; i <= 0x11c;) {
		val = edma_read(tdev, DMA_BASE_ADDR + i);
		trans_dbg(tdev, TR_ERR,
			"edma: dir:%s chn:%d i:0x%04x  val:0x%08x.\n",
			dir, channel, DMA_BASE_ADDR + i, val);
		i += 4;
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j <= 8; j++) {
			val = edma_read(tdev, DMA_WR_BASE + DMA_CHN(i) + j * 4);
			trans_dbg(tdev, TR_ERR,
				"edma: dir:%s chn:%d i:0x%x  val:0x%08x.\n",
				dir, channel, (DMA_WR_BASE + DMA_CHN(i) + j * 4), val);
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j <= 8; j++) {
			val = edma_read(tdev, DMA_RD_BASE + DMA_CHN(i) + j * 4);
			trans_dbg(tdev, TR_ERR,
				"edma: dir:%s chn:%d i:0x%x  val:0x%08x.\n",
				dir, channel, (DMA_RD_BASE + DMA_CHN(i) + j * 4), val);
		}
	}

}

/*
 * according transfer dircet, get a idle channel.
 *
 * @direct: the transfer direct of request channel.
 * @tedma: edma struct detail information.
 * return value: <0: failed; >=0 ok;
 */
static int get_edma_channel(u8 direct, struct edma_t *tedma)
{
	int i;

	if (direct == RC2EP) {
		while (1) {
			spin_lock(&tedma->rc2ep_cs_lock);
			/* rc2ep channel 0 and 3 only for tcache. */
			for (i = 1; i <= 2; i++) {
				if (tedma->rc2ep_cs[i] == EDMA_FREE) {
					tedma->rc2ep_cs[i] = EDMA_BUSY;
					break;
				}
			}
			spin_unlock(&tedma->rc2ep_cs_lock);

			if (i == 3) {
				if (down_interruptible(&tedma->rc2ep_sem))
					return -EFAULT;
			} else
				return i;
		}
	} else if (direct == EP2RC) {
		while (1) {
			spin_lock(&tedma->ep2rc_cs_lock);
			for (i = 0; i <= 3; i++) {
				if (tedma->ep2rc_cs[i] == EDMA_FREE) {
					tedma->ep2rc_cs[i] = EDMA_BUSY;
					break;
				}
			}
			spin_unlock(&tedma->ep2rc_cs_lock);

			if (i == 4) {
				if (down_interruptible(&tedma->ep2rc_sem))
					return -EFAULT;
			} else
				return i;
		}
	} else {
		trans_dbg(tedma->tdev, TR_ERR,
			"edma: %s, input direct:%d error.\n",
			__func__, direct);
	}

	return -EFAULT;
}

/*
 * free a channel; after using, need to change channel status to free.
 * then other task can get it from funcfion get_edma_channel.
 *
 * @channel: need to be freed channel.
 * @direct: the transfer direct of this channel.
 * @tedma: edma struct detail information.
 */
static void free_edma_channel(int channel, u8 direct,
				    struct edma_t *tedma)
{
	if (direct == RC2EP) {
		spin_lock(&tedma->rc2ep_cs_lock);
		tedma->rc2ep_cs[channel] = EDMA_FREE;
		up(&tedma->rc2ep_sem);
		spin_unlock(&tedma->rc2ep_cs_lock);
	} else {
		spin_lock(&tedma->ep2rc_cs_lock);
		tedma->ep2rc_cs[channel] = EDMA_FREE;
		up(&tedma->ep2rc_sem);
		spin_unlock(&tedma->ep2rc_cs_lock);
	}
}

/*
 * Because writing EP DDR has latency,
 * ensure the link table has been write to ddr completely,
 * add a flag at the end of the link table,check the falg,
 * untill flag is right. The latency is very short.
 */
static int check_link_table_done(struct cb_tranx_t *tdev,
					void __iomem *table_end, u32 flag)
{
	int loop = 100;
	int try_times = 100;

	writel(flag, table_end);
	while (try_times--) {
		while (loop--) {
			if (readl(table_end) == flag)
				return 0;
		}
		usleep_range(1, 2);
	}
	tdev->hw_err_flag = HW_ERR_FLAG;
	return -EFAULT;
}

/*
 * dump edma link table information.
 * @cnt: total element count in this link table.
 * @name: where dose the link table come from.
 * @en: whether to print dw_edma_llp.
 */
static void dump_link_table(struct dma_link_table *table_info, int cnt,
				 struct cb_tranx_t *tdev, char *name, char en,
				 char *dir, int channel)
{
	int i;
	struct dw_edma_llp __iomem *edma_llp;

	for (i = 0; i < cnt; i++) {
		trans_dbg(tdev, TR_ERR,
			"edma: sig dir:%s c:%d %s ctl:0x%02x size:0x%x sh:0x%x sl:0x%x dh:0x%x dl:0x%x\n",
			dir,
			channel,
			name,
			table_info[i].control,
			table_info[i].size,
			table_info[i].sar_high,
			table_info[i].sar_low,
			table_info[i].dst_high,
			table_info[i].dst_low);
	}

	if (en) {
		edma_llp = (struct dw_edma_llp __iomem *)(&table_info[cnt]);
		trans_dbg(tdev, TR_ERR,
			"edma: sig dir:%s c:%d %s ctrl:0x%02x rsv:0x%x llp_h:0x%x llp_l:0x%x\n",
			dir,
			channel,
			name,
			edma_llp->control,
			edma_llp->reserved,
			edma_llp->llp_high,
			edma_llp->llp_low);
	}
}

void dump_all_linktable(struct cb_tranx_t *tdev)
{
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];
	int i = 0, j = 0;
	struct dma_link_table *table_info;
	int cnt;

	for (j = 0; j < 4; j++) {
		table_info = tedma->backup_rc2ep[j].tab_inf;
		cnt = tedma->backup_rc2ep[j].tb_size;
		for (i = 0; i < cnt; i++) {
			trans_dbg(tdev, TR_ERR,
				"edma: all dir:%s c:%d ctl:0x%02x size:0x%x sh:0x%x sl:0x%x dh:0x%x dl:0x%x\n",
				"rc2ep",
				j,
				table_info[i].control,
				table_info[i].size,
				table_info[i].sar_high,
				table_info[i].sar_low,
				table_info[i].dst_high,
				table_info[i].dst_low);
		}
	}

	for (j = 0; j < 4; j++) {
		table_info = tedma->backup_ep2rc[j].tab_inf;
		cnt = tedma->backup_ep2rc[j].tb_size;
		for (i = 0; i < cnt; i++) {
			trans_dbg(tdev, TR_ERR,
				"edma: all dir:%s c:%d ctl:0x%02x size:0x%x sh:0x%x sl:0x%x dh:0x%x dl:0x%x\n",
				"ep2rc",
				j,
				table_info[i].control,
				table_info[i].size,
				table_info[i].sar_high,
				table_info[i].sar_low,
				table_info[i].dst_high,
				table_info[i].dst_low);
		}
	}
}

/*
 * transfer data from RC to EP by edma; support ddr to ddr and ddr to tcache.
 * to tcache transmission need enable hansshake, and the used channel is fixed.
 * so they have some different.
 *
 * @table_info: edma link table.
 * @edma_info: edma transfer info.
 * @tdev: core struct, record driver info.
 * @table_busaddr: for tcache, link table physical address base on BAR2.
 * @tcache: indicate that this transfer is from RC DDR to EP tcache.
 * @tcache_channel: only channel 0 and 3 support transfer data to tcache,
 *                  the channel is fixed, so when called this function,
 *                  channel has been selected by user application.
 * return value:
 *       0:success    non-zero:failed.
 */
static int edma_tranx_rc2ep(struct dma_link_table *table_info,
				struct trans_pcie_edma *edma_info,
				struct cb_tranx_t *tdev,
				u64 table_busaddr,
				u8 tcache,
				int tcache_channel)
{
	u32 val, ctl;
	int ret, c;
	unsigned char done = 0;

	/* edma link table which are saved in ep side ddr */
	struct dma_link_table __iomem *link_table;
	struct dw_edma_llp __iomem *edma_llp;
	void __iomem *latency_flag;

	/* link table physical address, ep pcie axi master view memory space */
	u64 table_paddr;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];
	u32 cnt = edma_info->element_size;

	/*
	 * for tcache, the link table has been ready,saved in ep ddr, so
	 * here only need the start address of link table.
	 */
	if (tcache) {
		link_table = table_info;
		table_paddr = table_busaddr;
		c = tcache_channel;
	} else {
		/* for ddr transfer, need to get a free channel and prepare
		 * the link table.
		 */
		c = get_edma_channel(RC2EP, tedma);
		if (c < 0) {
			trans_dbg(tdev, TR_ERR,
				"edma: get_edma_channel rc2ep failed.\n");
			return -EFAULT;
		}
		link_table = tedma->vedma_lt + LT_OFF(c);
		table_paddr = ELTA + LT_OFF(c);

		for (val = 0; val < cnt; val++) {
			link_table[val].control = table_info[val].control;
			link_table[val].size = table_info[val].size;
			link_table[val].sar_high = table_info[val].sar_high;
			link_table[val].sar_low = table_info[val].sar_low;
			link_table[val].dst_high = table_info[val].dst_high;
			link_table[val].dst_low = table_info[val].dst_low;

			tedma->backup_rc2ep[c].tab_inf[val].control = table_info[val].control;
			tedma->backup_rc2ep[c].tab_inf[val].size = table_info[val].size;
			tedma->backup_rc2ep[c].tab_inf[val].sar_high = table_info[val].sar_high;
			tedma->backup_rc2ep[c].tab_inf[val].sar_low = table_info[val].sar_low;
			tedma->backup_rc2ep[c].tab_inf[val].dst_high = table_info[val].dst_high;
			tedma->backup_rc2ep[c].tab_inf[val].dst_low = table_info[val].dst_low;
		}
		tedma->backup_rc2ep[c].tb_size = cnt;
	}
	edma_llp = (struct dw_edma_llp __iomem *)(&link_table[cnt]);
	edma_llp->control = DMA_LLP | DMA_TCB;
	edma_llp->reserved = 0;
	edma_llp->llp_high = QWORD_HI(table_paddr);
	edma_llp->llp_low = QWORD_LO(table_paddr);

	latency_flag = edma_llp + 1;
	if (check_link_table_done(tdev, latency_flag, tedma->readback_r[c])) {
		trans_dbg(tdev, TR_ERR,
			"edma: write link table failed,chn:%d direct:%s.\n",
			c, edma_info->direct ? "EP2RC" : "RC2EP");
		return -EFAULT;
	}
	tedma->readback_r[c]++;

	trans_dbg(tdev, TR_DBG,
		"edma: chn:%d element_size:%d direct:%d.\n",
		c, cnt, edma_info->direct);

	/*
	 * edma rc2ep have four channels, and they config registers are
	 * same, so use a spin lock to protect configuration procedure.
	 */
	spin_lock(&tedma->rc2ep_cfg_lock);
	if (tdev->hw_err_flag) {
		spin_unlock(&tedma->rc2ep_cfg_lock);
		return -EFAULT;
	}
	val = edma_read(tdev, DMA_READ_ENGINE_EN_OFF);
	if (!(val & DMA_ENGINE_EN))
		val |= DMA_ENGINE_EN;
	if (tcache) /* transfer to tcache need enable edma handshake.*/
		val |= (DMA_HANDSHAKE << c);
	edma_write(tdev, DMA_READ_ENGINE_EN_OFF, val|0x1);

	/*clear the interrupt*/
	val = (DMA_DONE(c)|DMA_ABORT(c));
	edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val);
	edma_write(tdev, DMA_CHN(c)+DMA_CH_CTL1_RD_OFF, DMA_TD|DMA_LLE|DMA_CCS);

	/* set link err enable */
	val = edma_read(tdev, DMA_READ_LINKED_LIST_ERR_EN_OFF);
	val |= (DMA_DONE(c)|DMA_ABORT(c));
	edma_write(tdev, DMA_READ_LINKED_LIST_ERR_EN_OFF, val);

	/* set LLP */
	edma_write(tdev, DMA_CHN(c)+DMA_LLP_LOW_RD_OFF, QWORD_LO(table_paddr));
	edma_write(tdev, DMA_CHN(c)+DMA_LLP_HIGH_RD_OFF, QWORD_HI(table_paddr));

	tedma->wait_condition_r[c] = 0;
	/* enable this channel */
	edma_write(tdev, DMA_READ_DOORBELL_OFF, c);
	spin_unlock(&tedma->rc2ep_cfg_lock);

	/* tcache don't need to wait interrupt, user app will check edma
	 * status, so return.
	 */
	if (tcache)
		return 0;

	ret = wait_event_interruptible_timeout(tedma->queue_wait,
				tedma->wait_condition_r[c], EDMA_TIMEOUT);
	if (ret == 0) {
		val = edma_read(tdev, DMA_READ_INT_STATUS_OFF);
		ctl = edma_read(tdev, DMA_CHN(c)+DMA_CH_CTL1_RD_OFF);
		trans_dbg(tdev, TR_ERR,
			"edma: RC2EP timeout: c:%d,status=0x%x,ctl=0x%x,condition=%d\n",
			c, val, ctl, tedma->wait_condition_r[c]);
		dump_all_linktable(tdev);

		/* double check edma status */
		if (((DMA_DONE(c)|DMA_ABORT(c)) & val) || tedma->wait_condition_r[c]) {
			trans_dbg(tdev, TR_ERR,
				"edma: double check,edma done,RC2EP c:%d, val=0x%x condition=%d\n",
				c, val, tedma->wait_condition_r[c]);
			done = 1;
			/* clear the interrupt */
			val = (DMA_DONE(c) | DMA_ABORT(c));
			edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val);
		} else {
			tdev->hw_err_flag = HW_ERR_FLAG;
			dump_link_table(link_table, cnt, tdev, "ddr", 1, "RC2EP", c);

			/* if edma transfer timeout, dump edma all registers */
			dump_edma_regs(tdev, "RC2EP", c);
		}
	} else if (ret < 0)
		trans_dbg(tdev, TR_ERR,
			"edma: RC2EP,wait transmission terminated, c:%d\n", c);
	else {
		done = 1;
		atomic64_add(edma_info->size, &tedma->edma_perf.rc2ep_size);
	}
	free_edma_channel(c, edma_info->direct, tedma);

	return (done == 1) ? 0 : -EFAULT;
}

/*
 * transfer data from EP to RC by edma; not support tcache transfer.
 *
 * @table_info: edma link table.
 * @edma_info: edma transfer info.
 * @tdev: core struct, record driver info.
 * return value:
 *       0:success    non-zero:failed.
 */
static int edma_tranx_ep2rc(struct dma_link_table *table_info,
				 struct trans_pcie_edma *edma_info,
				 struct cb_tranx_t *tdev)
{
	u32 val, ctl;
	int ret, c;
	unsigned char done = 0;

	/* edma link table which are saved in ep side ddr */
	struct dma_link_table __iomem *link_table;
	struct dw_edma_llp __iomem *edma_llp;
	void __iomem *latency_flag;

	/* link table physical address, ep pcie axi master view memory space */
	u64 table_paddr;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];
	u32 cnt = edma_info->element_size;

	c = get_edma_channel(EP2RC, tedma);
	if (c < 0) {
		trans_dbg(tdev, TR_ERR, "edma: get_edma_channel ep2rc failed\n");
		return -EFAULT;
	}

	link_table = tedma->vedma_lt + LT_OFF(c+4);
	table_paddr = ELTA + LT_OFF(c+4);

	for (val = 0; val < cnt; val++) {
		link_table[val].control = table_info[val].control;
		link_table[val].size = table_info[val].size;
		link_table[val].sar_high = table_info[val].sar_high;
		link_table[val].sar_low = table_info[val].sar_low;
		link_table[val].dst_high = table_info[val].dst_high;
		link_table[val].dst_low = table_info[val].dst_low;

		tedma->backup_ep2rc[c].tab_inf[val].control = table_info[val].control;
		tedma->backup_ep2rc[c].tab_inf[val].size = table_info[val].size;
		tedma->backup_ep2rc[c].tab_inf[val].sar_high = table_info[val].sar_high;
		tedma->backup_ep2rc[c].tab_inf[val].sar_low = table_info[val].sar_low;
		tedma->backup_ep2rc[c].tab_inf[val].dst_high = table_info[val].dst_high;
		tedma->backup_ep2rc[c].tab_inf[val].dst_low = table_info[val].dst_low;
	}
	tedma->backup_ep2rc[c].tb_size = cnt;

	edma_llp = (struct dw_edma_llp __iomem *)(&link_table[cnt]);
	edma_llp->control = DMA_LLP | DMA_TCB;
	edma_llp->reserved = 0;
	edma_llp->llp_high = QWORD_HI(table_paddr);
	edma_llp->llp_low = QWORD_LO(table_paddr);

	latency_flag = edma_llp + 1;
	if (check_link_table_done(tdev, latency_flag, tedma->readback_w[c])) {
		trans_dbg(tdev, TR_ERR,
			"edma: write link table failed,chn:%d direct:%s.\n",
			c, edma_info->direct ? "EP2RC" : "RC2EP");
		return -EFAULT;
	}
	tedma->readback_w[c]++;

	trans_dbg(tdev, TR_DBG, "edma: chn:%d element_size:%d dir:%d.\n",
		  c, cnt, edma_info->direct);
	/*
	 * edma ep2rc have four channels, and they config registers are
	 * same, so use a spin lock to protect configuration procedure.
	 */
	spin_lock(&tedma->ep2rc_cfg_lock);
	if (tdev->hw_err_flag) {
		spin_unlock(&tedma->ep2rc_cfg_lock);
		return -EFAULT;
	}
	val = edma_read(tdev, DMA_WRITE_ENGINE_EN_OFF);
	if ((val & 0x1) != 0x1)
		edma_write(tdev, DMA_WRITE_ENGINE_EN_OFF, val|0x1);

	/*clear the interrupt */
	val = (DMA_DONE(c)|DMA_ABORT(c));
	edma_write(tdev, DMA_WRITE_INT_CLEAR_OFF, val);
	edma_write(tdev, DMA_CHN(c)+DMA_CH_CTL1_WR_OFF, DMA_TD|DMA_LLE|DMA_CCS);

	/* set link err enable */
	val = edma_read(tdev, DMA_WRITE_LINKED_LIST_ERR_EN_OFF);
	val |= (DMA_DONE(c)|DMA_ABORT(c));
	edma_write(tdev, DMA_WRITE_LINKED_LIST_ERR_EN_OFF, val);

	/* set LLP */
	edma_write(tdev, DMA_CHN(c)+DMA_LLP_LOW_WR_OFF, QWORD_LO(table_paddr));
	edma_write(tdev, DMA_CHN(c)+DMA_LLP_HIGH_WR_OFF, QWORD_HI(table_paddr));

	tedma->wait_condition_w[c] = 0;
	/* enable this channel */
	edma_write(tdev, DMA_WRITE_DOORBELL_OFF, c);
	spin_unlock(&tedma->ep2rc_cfg_lock);

	 ret = wait_event_interruptible_timeout(tedma->queue_wait,
						tedma->wait_condition_w[c],
						EDMA_TIMEOUT);
	if (ret == 0) {
		val = edma_read(tdev, DMA_WRITE_INT_STATUS_OFF);
		ctl = edma_read(tdev, DMA_CHN(c)+DMA_CH_CTL1_WR_OFF);
		trans_dbg(tdev, TR_ERR,
			"edma: EP2RC timeout: c:%d,status=0x%x,ctl=0x%x,condition=%d\n",
			c, val, ctl, tedma->wait_condition_w[c]);
		dump_all_linktable(tdev);

		if (((DMA_DONE(c)|DMA_ABORT(c)) & val) || tedma->wait_condition_w[c]) {
			trans_dbg(tdev, TR_ERR,
				"edma: double check,edma done,EP2RC c:%d, val=0x%x condition=%d\n",
				c, val, tedma->wait_condition_w[c]);
			done = 1;
			/* clear the interrupt */
			val = (DMA_DONE(c)|DMA_ABORT(c));
			edma_write(tdev, DMA_WRITE_INT_CLEAR_OFF, val);
		} else {
			tdev->hw_err_flag = HW_ERR_FLAG;
			dump_link_table(link_table, cnt, tdev, "ddr", 1, "EP2RC", c);

			/* if edma transfer timeout, dump edma all registers */
			dump_edma_regs(tdev, "EP2RC", c);
		}
	} else if (ret < 0)
		trans_dbg(tdev, TR_ERR,
			"edma: EP2RC,wait transmission terminated, c:%d\n", c);
	else {
		done = 1;
		atomic64_add(edma_info->size, &tedma->edma_perf.ep2rc_size);
	}
	free_edma_channel(c, edma_info->direct, tedma);

	return (done == 1) ? 0 : -EFAULT;
}

/*
 * transfer data by edma link mode;
 * @table_info: edma link table.
 * @edma_info: edma transfer info.
 * @tdev: core struct, record driver info.
 * return value:
 *       0:success    non-zero:failed.
 */
static int edma_link_xfer(struct dma_link_table *table_info,
			       struct trans_pcie_edma *edma_info,
			       struct cb_tranx_t *tdev)
{
	int ret = 0;

	if (tdev->hw_err_flag)
		return tdev->hw_err_flag;

	if (edma_info->direct == RC2EP) {
		ret = edma_tranx_rc2ep(table_info, edma_info, tdev, 0, 0, 0);
	} else if (edma_info->direct == EP2RC) {
		ret = edma_tranx_ep2rc(table_info, edma_info, tdev);
	} else {
		trans_dbg(tdev, TR_ERR, "edma: edma %s error.\n",
			  edma_info->direct ? "EP2RC" : "RC2EP");
		ret = -EFAULT;
	}

	return ret;
}

/*
 * This function is only for tcache initialization before EP DDR initialization,
 * only support rc2ep, non-link mode.
 * @edma_info: edma config inforation
 * return value:
 *       0:success    -EFAULT:failed.
 */
int edma_normal_rc2ep_xfer(struct trans_pcie_edma *edma_info,
				  struct cb_tranx_t *tdev)
{
	u32 val, ctl;
	int c, ret;
	unsigned char done = 1;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	if (edma_info->direct != RC2EP) {
		trans_dbg(tdev, TR_ERR, "edma: %s, %s error.\n",
			__func__, edma_info->direct ? "EP2RC" : "RC2EP");
		return -EFAULT;
	}

	c = get_edma_channel(RC2EP, tedma);
	if (c < 0) {
		trans_dbg(tdev, TR_ERR, "edma: %s, get_edma_channel failed\n",
			__func__);
		return -EFAULT;
	}

	spin_lock(&tedma->rc2ep_cfg_lock);
	val = edma_read(tdev, DMA_READ_ENGINE_EN_OFF);
	if (!(val & DMA_ENGINE_EN))
		edma_write(tdev, DMA_READ_ENGINE_EN_OFF, val|DMA_ENGINE_EN);

	/* clear the interrupt */
	val = (DMA_DONE(c) | DMA_ABORT(c));
	edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val);
	val = DMA_RIE | DMA_TD | DMA_LIE;
	edma_write(tdev, DMA_CHN(c) + DMA_CH_CTL1_RD_OFF, val);

	/* transmit size */
	edma_write(tdev, DMA_CHN(c) + DMA_XFER_SIZE_RD_OFF, edma_info->size);
	/* rc ddr address */
	edma_write(tdev, DMA_CHN(c) + DMA_SAR_LOW_RD_OFF, edma_info->sar_low);
	edma_write(tdev, DMA_CHN(c) + DMA_SAR_HIGH_RD_OFF, edma_info->sar_high);
	/* ep ddr address */
	edma_write(tdev, DMA_CHN(c) + DMA_DAR_LOW_RD_OFF, edma_info->dar_low);
	edma_write(tdev, DMA_CHN(c) + DMA_DAR_HIGH_RD_OFF, edma_info->dar_high);
	edma_write(tdev, DMA_CHN(c) + DMA_LLP_LOW_RD_OFF, 0);
	edma_write(tdev, DMA_CHN(c) + DMA_LLP_HIGH_RD_OFF, 0);

	/* interrupt mode: wait transmit complite */
	tedma->wait_condition_r[c] = 0;
	/* enable doorbell */
	edma_write(tdev, DMA_READ_DOORBELL_OFF, c);
	spin_unlock(&tedma->rc2ep_cfg_lock);

	ret = wait_event_interruptible_timeout(tedma->queue_wait,
					       tedma->wait_condition_r[c],
					       EDMA_TIMEOUT);
	if (ret == 0) {
		done = 0;
		val = edma_read(tdev, DMA_READ_INT_STATUS_OFF);
		ctl = edma_read(tdev, DMA_CHN(c)+DMA_CH_CTL1_RD_OFF);
		trans_dbg(tdev, TR_ERR,
			"edma: RC2EP timeout: c:%d,status=0x%x,ctl=0x%x,condition=%d\n",
			c, val, ctl, tedma->wait_condition_r[c]);
		trans_dbg(tdev, TR_ERR,
			"edma: timeout: %s c:%d size:0x%x,sl:0x%x,sh:0x%x,dl:0x%x,dh:0x%x\n",
			__func__, c, edma_info->size,
			edma_info->sar_low, edma_info->sar_high,
			edma_info->dar_low, edma_info->dar_high);

		if ((DMA_DONE(c) | DMA_ABORT(c)) & val) {
			done = 1;
			val = (DMA_DONE(c) | DMA_ABORT(c));
			/* clear the interrupt */
			edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val);
		} else
			dump_edma_regs(tdev, "RC2EP", c);
	}
	if (ret < 0) {
		trans_dbg(tdev, TR_ERR,
			"edma: RC2EP,wait transmission terminated, c:%d\n", c);
	}
	free_edma_channel(c, edma_info->direct, tedma);

	return (done == 1) ? 0 : -EFAULT;
}

static inline
unsigned int count_pages(unsigned long iov_base, size_t iov_len)
{
	unsigned long first = (iov_base             & PAGE_MASK) >> PAGE_SHIFT;
	unsigned long last  = ((iov_base+iov_len-1) & PAGE_MASK) >> PAGE_SHIFT;

	return last - first + 1;
}

/* Calculate how many page count in the link table. */
static int calc_total_cnt(struct dma_link_table *link_table, int cnt)
{
	int i;
	int page_cnt = 0;
	unsigned long vaddr;

	for (i = 0; i < cnt; i++) {
		vaddr = link_table[i].sar_high;
		vaddr = (vaddr<<32)|link_table[i].sar_low;
		page_cnt += count_pages(vaddr, link_table[i].size);
	}

	return page_cnt;
}

/*
 * get the page information of a virtual address, then translet to physical
 * address array.
 */
static int cb_get_dma_addr(struct cb_tranx_t *tdev, unsigned long start,
				u32 len, struct rc_addr_info *paddr_array)
{
	u32 page_cnt;
	long rv;
	int i, sg_cnt;
	struct scatterlist *sg;
	struct sg_table sgt;
	struct page **user_pages;

	if (!len)
		return -EFAULT;

	page_cnt = count_pages(start, len);
	user_pages = kzalloc(sizeof(struct page *) * page_cnt, GFP_KERNEL);
	if (!user_pages) {
		trans_dbg(tdev, TR_ERR, "edma: allocate user page failed\n");
		goto out;
	}

	down_read(&current->mm->mmap_sem);
	rv = get_user_pages(start, page_cnt, FOLL_WRITE, user_pages, NULL);
	up_read(&current->mm->mmap_sem);
	if (rv != page_cnt) {
		trans_dbg(tdev, TR_ERR, "edma: get_user_pages failed:%ld\n", rv);
		rv = -EFAULT;
		goto out_free_page_mem;
	}

	rv = sg_alloc_table_from_pages(&sgt, user_pages, page_cnt,
				       start & (PAGE_SIZE-1), len, GFP_KERNEL);
	if (rv) {
		trans_dbg(tdev, TR_ERR, "edma: alloc sg_table failed:%ld\n", rv);
		rv = -EFAULT;
		goto out_put_user_pages;
	}

	sg_cnt = dma_map_sg(&tdev->pdev->dev, sgt.sgl, sgt.nents, 0);
	if (sg_cnt <= 0) {
		trans_dbg(tdev, TR_ERR, "edma: dma_map_sg failed:%d\n", sg_cnt);
		goto out_free_sg_table;
	}

	for_each_sg(sgt.sgl, sg, sg_cnt, i) {
		paddr_array[i].paddr = sg_dma_address(sg);
		paddr_array[i].size = sg_dma_len(sg);
	}
	dma_unmap_sg(&tdev->pdev->dev, sgt.sgl, sgt.nents, 0);
	rv = sg_cnt;

out_free_sg_table:
	sg_free_table(&sgt);
out_put_user_pages:
	for (i = 0 ; i < page_cnt ; i++)
		put_page(user_pages[i]);
out_free_page_mem:
	kfree(user_pages);
out:
	return rv;
}

/*
 * the RC address is virtual, this function will get its physical address,
 * generate link table, use edma link mode to transmit.
 */
static int edma_tranx_viraddr_mode(struct trans_pcie_edma *edma_info,
					   struct cb_tranx_t *tdev)
{
	int j, page_cnt;
	struct dma_link_table *link_table;
	unsigned int ctl = DMA_CB;
	unsigned long vaddr, ep_addr;
	struct rc_addr_info *paddr_array;
	int rv;

	if (edma_info->direct == RC2EP) {
		vaddr = edma_info->sar_high;
		vaddr = (vaddr<<32)|edma_info->sar_low;
		ep_addr = edma_info->dar_high;
		ep_addr = (ep_addr<<32)|edma_info->dar_low;
	} else {
		ep_addr = edma_info->sar_high;
		ep_addr = (ep_addr<<32)|edma_info->sar_low;
		vaddr = edma_info->dar_high;
		vaddr = (vaddr<<32)|edma_info->dar_low;
	}

	page_cnt = count_pages(vaddr, edma_info->size);
	paddr_array = vzalloc(sizeof(*paddr_array) * page_cnt);
	if (!paddr_array) {
		trans_dbg(tdev, TR_ERR, "edma: allocate paddr_array failed\n");
		goto out;
	}

	rv = cb_get_dma_addr(tdev, vaddr, edma_info->size, paddr_array);
	if (rv < 0) {
		trans_dbg(tdev, TR_ERR, "edma: get_user_pages failed.\n");
		goto out_free_paddr_array;
	}

	link_table = vzalloc(rv * sizeof(*link_table));
	if (!link_table) {
		trans_dbg(tdev, TR_ERR, "edma: allocate link table failed.\n");
		goto out_free_paddr_array;
	}

	/* create link table */
	for (j = 0; j < rv; j++) {
		if ((j+1) == rv)
			ctl |= (DMA_LIE | DMA_RIE);
		link_table[j].control = ctl;
		link_table[j].size = paddr_array[j].size;
		if (edma_info->direct == RC2EP) {
			link_table[j].sar_high = QWORD_HI(paddr_array[j].paddr);
			link_table[j].sar_low = QWORD_LO(paddr_array[j].paddr);
			link_table[j].dst_high = QWORD_HI(ep_addr);
			link_table[j].dst_low = QWORD_LO(ep_addr);
		} else {
			link_table[j].dst_high = QWORD_HI(paddr_array[j].paddr);
			link_table[j].dst_low = QWORD_LO(paddr_array[j].paddr);
			link_table[j].sar_high = QWORD_HI(ep_addr);
			link_table[j].sar_low = QWORD_LO(ep_addr);
		}
		ep_addr += paddr_array[j].size;
	}
	edma_info->element_size = j;
	rv = edma_link_xfer(link_table, edma_info, tdev);
	vfree(link_table);

out_free_paddr_array:
	vfree(paddr_array);
out:
	return rv;
}

/*
 * transfer data from RC to tcache with virtual rc address.
 * only two edma channel support this feature which are 0 and 3.
 * user app will transfer edma link table to ddr in ep side, the table
 * contain virtual rc address, so driver will translate it to physical
 * address one by one, regenerate a new link table for edma transmission.
 */
static int edma_tranx_tcache_mode(struct trans_pcie_edma *edma_info,
					 struct cb_tranx_t *tdev)
{
	int i, j, c;
	int rv;
	unsigned int ctl = DMA_CB;
	struct rc_addr_info *paddr_array;
	int page_cnt, size;
	int new_element_cnt = 0;
	unsigned long vaddr, ep_addr;
	u64 table_busaddr;
	struct dma_link_table __iomem *table_info;
	struct dma_link_table __iomem *user_table;
	struct dma_link_table __iomem *new_table;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	if (tdev->hw_err_flag)
		return tdev->hw_err_flag;

	c = edma_info->slice * 3;
	user_table = tedma->vedma_lt + LT_OFF(c);
	new_table = tedma->vedma_lt + LT_OFF(c) + NT_OFF;
	table_info = new_table;
	edma_info->element_size -= 1; /* Minus a table ending in zero */
	page_cnt = calc_total_cnt(user_table, edma_info->element_size);
	paddr_array = vzalloc(sizeof(*paddr_array) * page_cnt);
	if (!paddr_array) {
		trans_dbg(tdev, TR_ERR, "edma: allocate paddr_array failed\n");
		goto out;
	}

	edma_info->size = 0;
	/* create a new link table */
	for (i = 0; i < edma_info->element_size; i++) {
		vaddr = user_table[i].sar_high;
		vaddr = (vaddr<<32)|user_table[i].sar_low;
		ep_addr = user_table[i].dst_high;
		ep_addr = (ep_addr<<32)|user_table[i].dst_low;
		size = user_table[i].size;

		rv = cb_get_dma_addr(tdev, vaddr, size, paddr_array);
		if (rv < 0) {
			trans_dbg(tdev, TR_ERR,
				"edma: %s cb_get_dma_addr failed rv=%d\n",
				__func__, rv);
			goto out_free_paddr_array;
		}

		for (j = 0; j < rv; j++) {
			new_table[j].control = ctl;
			new_table[j].size = paddr_array[j].size;
			new_table[j].sar_high = QWORD_HI(paddr_array[j].paddr);
			new_table[j].sar_low = QWORD_LO(paddr_array[j].paddr);
			new_table[j].dst_high = QWORD_HI(ep_addr);
			new_table[j].dst_low = QWORD_LO(ep_addr);
			ep_addr += paddr_array[j].size;
			edma_info->size += new_table[j].size;
		}
		new_element_cnt += rv;
		new_table += rv;
	}
	ctl |= (DMA_LIE | DMA_RIE);
	table_info[new_element_cnt-1].control = ctl;
	edma_info->element_size = new_element_cnt;
	tedma->tcache_link_size[edma_info->slice] = new_element_cnt;
	table_busaddr = ELTA + LT_OFF(c) + NT_OFF;

	rv = edma_tranx_rc2ep(table_info, edma_info, tdev, table_busaddr, 1, c);
	atomic64_add(edma_info->size, &tedma->edma_perf.rc2ep_size);

out_free_paddr_array:
	vfree(paddr_array);
out:
	return rv;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static void pcie_bw_timer_isr(unsigned long data)
{
	struct edma_t *tedma = (struct edma_t *)data;
#else
static void pcie_bw_timer_isr(struct timer_list *t)
{
	struct edma_t *tedma = from_timer(tedma, t, perf_timer);
#endif
	mod_timer(&tedma->perf_timer, jiffies + PCIE_BW_TIMER);
	tedma->edma_perf.ep2rc_per =
		atomic64_read(&tedma->edma_perf.ep2rc_size) / 1024 / 1024;
	tedma->edma_perf.rc2ep_per =
		atomic64_read(&tedma->edma_perf.rc2ep_size) / 1024 / 1024;
	atomic64_set(&tedma->edma_perf.ep2rc_size, 0);
	atomic64_set(&tedma->edma_perf.rc2ep_size, 0);
}

irqreturn_t edma_isr(int irq, void *data)
{
	u32 val_r, val_w;
	int i;
	unsigned long flags;
	struct cb_tranx_t *tdev = data;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	spin_lock_irqsave(&tedma->edma_irq_lock, flags);

	val_r = edma_read(tdev, DMA_READ_INT_STATUS_OFF);
	/* clear read interrupt */
	edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val_r);

	val_w = edma_read(tdev, DMA_WRITE_INT_STATUS_OFF);
	/* clear write interrupt */
	edma_write(tdev, DMA_WRITE_INT_CLEAR_OFF, val_w);

	if (val_r & ABORT_INT_STATUS)
		trans_dbg(tdev, TR_ERR, "edma: edma isr rc2ep:0x%x \n", val_r);
	if (val_w & ABORT_INT_STATUS)
		trans_dbg(tdev, TR_ERR, "edma: edma isr ep2rc:0x%x \n", val_w);

	for (i = 0; i < 4; i++) {
		if ((val_r >> i) & 0x1)
			tedma->wait_condition_r[i] = 0x1;
	}
	for (i = 0; i < 4; i++) {
		if ((val_w >> i) & 0x1)
			tedma->wait_condition_w[i] = 0x1;
	}
	wake_up_interruptible_all(&tedma->queue_wait);

	spin_unlock_irqrestore(&tedma->edma_irq_lock, flags);

	return IRQ_HANDLED;
}

/* setting edma msi interrupt table */
static void edma_msi_config(struct cb_tranx_t *tdev,
				 u64 msi_addr, u32 msi_data)
{
	int id;
	u32 tmp;

	/* write channel MSI done addr - low, high */
	edma_write(tdev, DMA_WRITE_DONE_IMWR_LOW_OFF, QWORD_LO(msi_addr));
	edma_write(tdev, DMA_WRITE_DONE_IMWR_HIGH_OFF, QWORD_HI(msi_addr));

	/* write channel MSI abort addr - low, high */
	edma_write(tdev, DMA_WRITE_ABORT_IMWR_LOW_OFF, QWORD_LO(msi_addr));
	edma_write(tdev, DMA_WRITE_ABORT_IMWR_HIGH_OFF, QWORD_HI(msi_addr));

	/* write channel MSI data - low, high */
	for (id = 0; id < 7; id++) {
		tmp = edma_read(tdev, DMA_WRITE_CH01_IMWR_DATA_OFF+IMWR_OFF(id));
		if (id & 0x1) {
			tmp &= 0xFF;
			tmp |= (msi_data << 16);
		} else {
			tmp &= 0xFF00;
			tmp |= msi_data;
		}
		edma_write(tdev, DMA_WRITE_CH01_IMWR_DATA_OFF+IMWR_OFF(id), tmp);
	}

	/* read channel MSI done addr - low, high */
	edma_write(tdev, DMA_READ_DONE_IMWR_LOW_OFF, QWORD_LO(msi_addr));
	edma_write(tdev, DMA_READ_DONE_IMWR_HIGH_OFF, QWORD_HI(msi_addr));

	/* read channel MSI abort addr - low, high */
	edma_write(tdev, DMA_READ_ABORT_IMWR_LOW_OFF, QWORD_LO(msi_addr));
	edma_write(tdev, DMA_READ_ABORT_IMWR_HIGH_OFF, QWORD_HI(msi_addr));
	/* read channel MSI data - low, high */
	for (id = 0; id < 7; id++) {
		tmp = edma_read(tdev, DMA_READ_CH01_IMWR_DATA_OFF+IMWR_OFF(id));
		if (id & 0x1) {
			tmp &= 0xFF;
			tmp |= (msi_data << 16);
		} else {
			tmp &= 0xFF00;
			tmp |= msi_data;
		}
		edma_write(tdev, DMA_READ_CH01_IMWR_DATA_OFF+IMWR_OFF(id), tmp);
	}
}

/* display pcie tranx bandwidth, it is a probable value, EP2RC */
static ssize_t pcie_r_bw_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct cb_tranx_t *tdev = dev_get_drvdata(dev);
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	return sprintf(buf, "%d\n", tedma->edma_perf.ep2rc_per);
}

/* display pcie tranx bandwidth, it is a probable value, RC2EP */
static ssize_t pcie_w_bw_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct cb_tranx_t *tdev = dev_get_drvdata(dev);
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	return sprintf(buf, "%d\n", tedma->edma_perf.rc2ep_per);
}

static DEVICE_ATTR_RO(pcie_r_bw);
static DEVICE_ATTR_RO(pcie_w_bw);

static struct attribute *trans_edma_sysfs_entries[] = {
	&dev_attr_pcie_r_bw.attr,
	&dev_attr_pcie_w_bw.attr,
	NULL
};

static struct attribute_group trans_edma_attribute_group = {
	.name = NULL,
	.attrs = trans_edma_sysfs_entries,
};

static int edma_register_irq(struct cb_tranx_t *tdev)
{
	int ret;

	/* request edma irq. */
	ret = request_irq(pci_irq_vector(tdev->pdev, 0), edma_isr,
			IRQF_SHARED, "tedma", tdev);
	if (ret)
		trans_dbg(tdev, TR_ERR, "edma: request edma irq failed.\n");

	return ret;
}

int edma_init(struct cb_tranx_t *tdev)
{
	unsigned int val;
	u8 cap_off;
	u32 addr_hi, addr_lo;
	u16 flags;
	u64 msi_addr;
	u32 msi_data;
	struct edma_t *tedma;
	struct pci_dev *pdev = tdev->pdev;
	int ret;
	int i;
	void *tmp_mem;

	tedma = kzalloc(sizeof(struct edma_t), GFP_KERNEL);
	if (!tedma) {
		trans_dbg(tdev, TR_ERR, "edma: kzalloc tedma failed.\n");
		return -ENOMEM;
	}
	tdev->modules[TR_MODULE_EDMA] = tedma;
	tedma->tdev = tdev;

	val = edma_read(tdev, DMA_CTRL_OFF);
	trans_dbg(tdev, TR_DBG, "edma: wr_channel num:%d, rd_channel num:%d\n",
		  val & 0xF, (val >> 16) & 0xF);

	/* init all write_channel interrupt register */
	/*  mask all write interrupt */
	edma_write(tdev, DMA_WRITE_INT_MASK_OFF, MASK_ALL_INTE);
	/*  clear all write interrupt */
	edma_write(tdev, DMA_WRITE_INT_CLEAR_OFF, CLEAR_ALL_INTE);
	/* unable all write link abort error  */
	edma_write(tdev, DMA_WRITE_LINKED_LIST_ERR_EN_OFF, 0x0);

	/* init all read_channel interrupt register */
	/*  mask all read interrupt */
	edma_write(tdev, DMA_READ_INT_MASK_OFF, MASK_ALL_INTE);
	/*  clear all read interrupt */
	edma_write(tdev, DMA_READ_INT_CLEAR_OFF, CLEAR_ALL_INTE);
	/* unable all read link abort error  */
	edma_write(tdev, DMA_READ_LINKED_LIST_ERR_EN_OFF, 0x0);

	/* unmask all channel interrupt */
	edma_write(tdev, DMA_READ_INT_MASK_OFF, 0x0);
	edma_write(tdev, DMA_WRITE_INT_MASK_OFF, 0x0);

	/* there are 8 link tables which are saved in slice0 ddr. */
	tedma->vedma_lt = ioremap_nocache(pci_resource_start(pdev, 4),
					  EDMA_LT_SIZE * 8);
	if (!tedma->vedma_lt) {
		trans_dbg(tdev, TR_ERR, "edma: map edma link table failed.\n");
		goto out_free_edma;
	}

	memset(tedma->rc2ep_cs, EDMA_FREE, sizeof(tedma->rc2ep_cs));
	memset(tedma->ep2rc_cs, EDMA_FREE, sizeof(tedma->ep2rc_cs));

	atomic64_set(&tedma->edma_perf.ep2rc_size, 0);
	atomic64_set(&tedma->edma_perf.rc2ep_size, 0);

	tedma->perf_timer.expires = jiffies + PCIE_BW_TIMER;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	tedma->perf_timer.function = (void *)pcie_bw_timer_isr;
	tedma->perf_timer.data = (unsigned long)(tedma);
	init_timer(&tedma->perf_timer);
#else
	timer_setup(&tedma->perf_timer, pcie_bw_timer_isr, 0);
#endif
	add_timer(&tedma->perf_timer);

	mutex_init(&tedma->tcache_lock[0]);
	mutex_init(&tedma->tcache_lock[1]);
	sema_init(&tedma->ep2rc_sem, 0);
	sema_init(&tedma->rc2ep_sem, 0);
	init_waitqueue_head(&tedma->queue_wait);

	spin_lock_init(&tedma->rc2ep_cfg_lock);
	spin_lock_init(&tedma->ep2rc_cfg_lock);
	spin_lock_init(&tedma->rc2ep_cs_lock);
	spin_lock_init(&tedma->ep2rc_cs_lock);
	spin_lock_init(&tedma->edma_irq_lock);

	if (pdev->msi_cap && pdev->msi_enabled) {
		cap_off = pdev->msi_cap + PCI_MSI_FLAGS;
		pci_read_config_word(pdev, cap_off, &flags);
		if (flags & PCI_MSI_FLAGS_ENABLE) {
			cap_off = pdev->msi_cap + PCI_MSI_ADDRESS_LO;
			pci_read_config_dword(pdev, cap_off, &addr_lo);

			if (flags & PCI_MSI_FLAGS_64BIT) {
				cap_off = pdev->msi_cap + PCI_MSI_ADDRESS_HI;
				pci_read_config_dword(pdev, cap_off, &addr_hi);
				cap_off = pdev->msi_cap + PCI_MSI_DATA_64;
			} else {
				addr_hi = 0;
				cap_off = pdev->msi_cap + PCI_MSI_DATA_32;
			}

			msi_addr = addr_hi;
			msi_addr <<= 32;
			msi_addr |= addr_lo;

			pci_read_config_dword(pdev, cap_off, &msi_data);
			msi_data &= 0xffff;
		}

		edma_msi_config(tdev, msi_addr, msi_data);
	}

	ret = sysfs_create_group(&tdev->misc_dev->this_device->kobj,
				&trans_edma_attribute_group);
	if (ret) {
		trans_dbg(tdev, TR_ERR,
			"edma: failed to create sysfs device attributes\n");
		goto out_free_table;
	}

	if (edma_register_irq(tdev)) {
		trans_dbg(tdev, TR_ERR, "edma: register irq failed.\n");
		goto out_remove_sysnode;
	}

	tmp_mem = vzalloc(BAKUP_SIZE*8);
	if (!tmp_mem) {
		trans_dbg(tdev, TR_ERR, "edma: malloc backup mem failed.\n");
		goto out_remove_sysnode;
	}

	for (i = 0; i < 4; i++)
		tedma->backup_rc2ep[i].tab_inf = tmp_mem + i *BAKUP_SIZE;
	for (i = 0; i < 4; i++)
		tedma->backup_ep2rc[i].tab_inf = tmp_mem + (i+4) *BAKUP_SIZE;

	trans_dbg(tdev, TR_INF, "edma: module initialize done.\n");
	return 0;

out_remove_sysnode:
	sysfs_remove_group(&tdev->misc_dev->this_device->kobj,
				&trans_edma_attribute_group);
out_free_table:
	del_timer_sync(&tedma->perf_timer);
	iounmap(tedma->vedma_lt);
out_free_edma:
	kfree(tedma);
	trans_dbg(tedma->tdev, TR_ERR, "edma: module initialize filed.\n");
	return -1;
}

static void edma_free_irq(struct cb_tranx_t *tdev)
{
	if (pci_irq_vector(tdev->pdev, 0))
		free_irq(pci_irq_vector(tdev->pdev, 0), tdev);
}

void edma_release(struct cb_tranx_t *tdev)
{
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];

	del_timer_sync(&tedma->perf_timer);
	sysfs_remove_group(&tdev->misc_dev->this_device->kobj,
			   &trans_edma_attribute_group);

	/* mask all write interrupt */
	edma_write(tdev, DMA_WRITE_INT_MASK_OFF, MASK_ALL_INTE);
	/* clear all write interrupt */
	edma_write(tdev, DMA_WRITE_INT_CLEAR_OFF, CLEAR_ALL_INTE);
	/* mask all read interrupt */
	edma_write(tdev, DMA_READ_INT_MASK_OFF, MASK_ALL_INTE);
	/* clear all read interrupt */
	edma_write(tdev, DMA_READ_INT_CLEAR_OFF, CLEAR_ALL_INTE);

	edma_write(tdev, DMA_READ_ENGINE_EN_OFF, 0x0);
	edma_write(tdev, DMA_WRITE_ENGINE_EN_OFF, 0x0);

	iounmap(tedma->vedma_lt);
	edma_free_irq(tdev);
	mutex_destroy(&tedma->tcache_lock[0]);
	mutex_destroy(&tedma->tcache_lock[1]);

	vfree(tedma->backup_rc2ep[0].tab_inf);

	kfree(tedma);
	trans_dbg(tdev, TR_DBG, "edma: remove module done.\n");
}

long edma_ioctl(struct file *filp, unsigned int cmd,
		    unsigned long arg, struct cb_tranx_t *tdev)
{
	long ret = 0;
	void __user *argp = (void __user *)arg;
	struct trans_pcie_edma edma_info;
	unsigned int val, c, tl_s, ctl;
	struct trans_pcie_edma edma_trans;
	unsigned long ep_addr;
	struct edma_t *tedma = tdev->modules[TR_MODULE_EDMA];
	struct dma_link_table __iomem *new_table;
	struct dma_link_table __iomem *user_table;

	switch (cmd) {
	case CB_TRANX_EDMA_TRANX_TCACHE:
		if (copy_from_user(&edma_info, argp, sizeof(edma_info)))
			return -EFAULT;
		if (WARN_ON(edma_info.slice > SLICE_1))
			return -EFAULT;
		if (mutex_lock_interruptible(&tedma->tcache_lock[edma_info.slice]))
			return -ERESTARTSYS;
		val = edma_info.slice * 3;
		ep_addr = ELTA+LT_OFF(val);
		edma_trans.dar_low = QWORD_LO(ep_addr);
		edma_trans.dar_high = QWORD_HI(ep_addr);
		edma_trans.sar_low = QWORD_LO(edma_info.rc_ult);
		edma_trans.sar_high = QWORD_HI(edma_info.rc_ult);
		edma_trans.size = edma_info.element_size*sizeof(struct dma_link_table);
		edma_trans.direct = RC2EP;
		ret = edma_tranx_viraddr_mode(&edma_trans, tdev);
		if (ret) {
			trans_dbg(tdev, TR_ERR,
				"edma: get link table failed,ep:0x%x,rc:0x%lx size:0x%x\n",
				ep_addr, edma_info.rc_ult, edma_trans.size);
			mutex_unlock(&tedma->tcache_lock[edma_info.slice]);
			return -EFAULT;
		}
		ret = edma_tranx_tcache_mode(&edma_info, tdev);
		mutex_unlock(&tedma->tcache_lock[edma_info.slice]);
		break;
	case CB_TRANX_EDMA_TRANX:
		if (copy_from_user(&edma_info, argp, sizeof(edma_info)))
			return -EFAULT;
		ret = edma_tranx_viraddr_mode(&edma_info, tdev);
		break;
	case CB_TRANX_EDMA_STATUS:
		/* get edma link channel(0/3) status. */
		if (copy_from_user(&val, argp, sizeof(val))) {
			trans_dbg(tdev, TR_ERR,
				"edma: get_edma_status copy_from_user failed\n");
			return -EFAULT;
		}
		if (val > SLICE_1) {
			trans_dbg(tdev, TR_ERR,
				"edma: get_edma_status s:%d error.\n", val);
			return -EFAULT;
		}
		tl_s = tedma->tcache_link_size[val];
		tedma->tcache_link_size[val] = 0;
		c = val * 3;
		ret = wait_event_interruptible_timeout(tedma->queue_wait,
						tedma->wait_condition_r[c],
						EDMA_TIMEOUT);
		if (ret == 0) {
			val = edma_read(tdev, DMA_READ_INT_STATUS_OFF);
			ctl = edma_read(tdev, DMA_CHN(c)+DMA_CH_CTL1_RD_OFF);
			trans_dbg(tdev, TR_ERR,
				"edma: RC2tcache timeout: c:%d,status=0x%x,ctl=0x%x,condition=%d\n",
				c, val, ctl, tedma->wait_condition_r[c]);

			/* double check edma status */
			if (((DMA_DONE(c)|DMA_ABORT(c)) & val) || tedma->wait_condition_r[c]) {
				trans_dbg(tdev, TR_ERR,
					"edma: double check, RC2tcache done, c:%d, val=0x%x condition=%d\n",
					c, val, tedma->wait_condition_r[c]);
				/* clear the interrupt */
				val = (DMA_DONE(c) | DMA_ABORT(c));
				edma_write(tdev, DMA_READ_INT_CLEAR_OFF, val);
			} else {
				tdev->hw_err_flag = HW_ERR_FLAG;
				user_table = tedma->vedma_lt + LT_OFF(c);
				new_table = tedma->vedma_lt + LT_OFF(c) + NT_OFF;
				dump_link_table(user_table, tl_s, tdev, "user", 0, "RC2EP", c);
				dump_link_table(new_table, tl_s, tdev, "new", 1, "RC2EP", c);
				dump_edma_regs(tdev, "RC2EP", c);
			}
		} else if (ret < 0)
			trans_dbg(tdev, TR_ERR,
				"edma: tranx to tcache terminated, c:%d\n", c);
		if (ret <= 0)
			__put_user(LT_UNDONE, (unsigned int *)argp);
		else
			__put_user(LT_DONE, (unsigned int *)argp);
		return (ret <= 0) ? -EFAULT : 0;
	default:
		trans_dbg(tdev, TR_ERR,
			"edma: %s, cmd:0x%x is error.\n", __func__, cmd);
		ret = -EINVAL;
	}

	return ret;
}

