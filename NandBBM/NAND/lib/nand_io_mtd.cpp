/*
 *  nand_io_mtd.c
 *
 *    'nand_io' interface, based on linux-MTD layer
 *
 *  Copyright (c) 2012-2013 STMicroelectronics Limited
 *  Author: Angus Clark <Angus.Clark@st.com>
 *
 *  ---------------------------------------------------------------------------
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 51
 *  Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *  ---------------------------------------------------------------------------
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <mtd/mtd-user.h>

#include <nand.h>
#include <nand_io.h>
#include <xprintf.h>

#include "nand_ecc_bch.h"
#include "nand_io_mtd.h"

#define DEFAULT_DEBUGFS_PATH	"/sys/kernel/debug"

char *stlinux_nand_drivers[] = {
	[STLINUX_NAND_EMI] = "stm-nand-emi",
	[STLINUX_NAND_FLEX] = "stm-nand-flex",
	[STLINUX_NAND_AFM] = "stm-nand-afm",
	[STLINUX_NAND_BCH] = "stm-nand-bch",
};

#define SYSFS_FN_LEN	128
static int detect_stlinux_nand_driver(char *mtddev)
{
	int mtdnum;
	char sysfs_driver_link[SYSFS_FN_LEN];
	char sysfs_driver_node[SYSFS_FN_LEN];
	int ret;
	int i;

	ret = sscanf(mtddev, "/dev/mtd%d", &mtdnum);
	if (ret != 1) {
		eprintf("failed to determine mtd device number\n");
		return -1;
	}

	snprintf(sysfs_driver_link, SYSFS_FN_LEN,
		 "/sys/class/mtd/mtd%d/device/driver", mtdnum);
	memset(sysfs_driver_node, 0, SYSFS_FN_LEN);

	ret = readlink(sysfs_driver_link, sysfs_driver_node,
		       SYSFS_FN_LEN - 1);
	if (ret < 0) {
		eprintf("failed to read sysfs driver entry\n");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(stlinux_nand_drivers); i++) {
		if (!stlinux_nand_drivers[i])
			continue;

		if (strstr(sysfs_driver_node, stlinux_nand_drivers[i]))
			return i;
	}

	return -1;
}

static int debugfs_nanderasebb(struct nand_io_mtd *mtd, int enable)
{
	int fd;
	char *buf = enable ? "1" : "0";
	int ret = 0;

	fd = open(mtd->debugfs_nanderasebb, O_WRONLY);
	if (fd == -1)
		return 1;

	if (write(fd, buf, strlen(buf)) != strlen(buf))
		ret = 1;

	close(fd);

	return ret;
}

void *nand_io_init(void *_config, struct nand_info *info)
{
	struct nand_io_mtd_config *config = _config;
	struct nand_io_mtd *mtd;
	struct mtd_info_user meminfo;

	mtd = malloc(sizeof(struct nand_io_mtd));
	if (!mtd) {
		eprintf("failed to malloc MTD IO info\n");
		return NULL;
	}

	strncpy(mtd->mtddev, config->mtddev, MAX_MTDDEV_LEN);

	mtd->fd = open(mtd->mtddev, O_RDWR);
	if (mtd->fd == -1) {
		eprintf("failed to open mtd device \"%s\"\n", mtd->mtddev);
		goto err1;
	}

	if (ioctl(mtd->fd, MEMGETINFO, &meminfo) != 0) {
		eprintf("failed to get Flash info\n");
		goto err2;
	}

	if (meminfo.type != MTD_NANDFLASH) {
		eprintf("Flash on \"%s\" is not NAND\n", mtd->mtddev);
		goto err2;
	}


	/* Check that MTD slave partition spans the backing master device */
	if (config->options & NAND_IO_MTD_ENFORCE_SPANS_MASTER) {
		if (!(meminfo.flags & MTD_SPANS_MASTER)) {
			eprintf("MTD partition [%s] does not span entire NAND device,\n",
				mtd->mtddev);
			eprintf("   or kernel does not support 'MTD_SPANS_MASTER' flag\n");
			goto err2;
		}
	}

	/* Bypass the kernel's bad block protection */
	if (config->options & NAND_IO_MTD_BYPASS_BAD_PROTECT) {
		snprintf(mtd->debugfs_nanderasebb, MAX_DEBUGFS_LEN,
			 "%s/nanderasebb",
			 config->debugfs ? config->debugfs :
			 DEFAULT_DEBUGFS_PATH);

		if (debugfs_nanderasebb(mtd, 1) != 0) {
			eprintf("Failed to enable erase operations on bad blocks\n");
			eprintf("\tDEBUGFS not enabled in kernel, or not mounted at %s?\n",
				mtd->debugfs_nanderasebb);
			strcpy(mtd->debugfs_nanderasebb, "");
		}
	}

	info->size = meminfo.size;
	info->page_size = meminfo.writesize;
	info->oob_size = meminfo.oobsize;
	info->block_size = meminfo.erasesize;
	info->block_shift = ffs64(info->block_size) - 1;
	info->block_count = info->size >> info->block_shift;

	mtd->bch_remap = REMAP_NONE;
	mtd->driver = detect_stlinux_nand_driver(mtd->mtddev);
	if (mtd->driver == -1) {
		eprintf("Failed to determine STLinux NAND Driver ('sysfs' not present?)\n");
		eprintf("If stm-nand-bch is in operation, Page<->OOB remapping will fail\n");
	} else if (mtd->driver == STLINUX_NAND_BCH) {
		struct nand_ecclayout_user ecclayout;
		int n_sectors;

		/* If using BCH driver, we need to remap page+oob */
		if (ioctl(mtd->fd, ECCGETLAYOUT, &ecclayout) != 0) {
			eprintf("failed to get Flash info\n");
			goto err2;
		}

		n_sectors = meminfo.writesize/BCH_SECTOR_BYTES;
		if (ecclayout.eccbytes == n_sectors * BCH18_ECC_BYTES)
			mtd->bch_remap = REMAP_BCH18;
		else if (ecclayout.eccbytes == n_sectors * BCH30_ECC_BYTES)
			mtd->bch_remap = REMAP_BCH30;
		else if (ecclayout.eccbytes == 0) {
			/*
			 * Appears to be 'BCH_NO_ECC', but more likely we have
			 * an old version of the stm_nand_bch driver that fails
			 * to report number of ECC bytes.
			 */
			eprintf("WARNING: Detected unlikely 'BCH_NO_ECC' mode.  This can result\n");
			eprintf("         from using an old version of the stm_nand_bch driver.\n");
			mtd->bch_remap = REMAP_NONE;
		} else {
			eprintf("Failed to determine BCH ECC mode\n");
			goto err2;
		}
	}

	/* Success */
	return mtd;

 err2:
	close(mtd->fd);
 err1:
	free(mtd);

	return NULL;
}

void nand_io_exit(void *iohandle)
{
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)iohandle;

	if (strlen(mtd->debugfs_nanderasebb) > 0)
		debugfs_nanderasebb(mtd, 0);

	close(mtd->fd);

	free(mtd);
}


int nandio_read_page_raw(void *iohandle, struct nand_info *info,
			 uint8_t *page, uint8_t *oob, uint64_t offs)
{
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)iohandle;
	struct mtd_oob_buf64 mtd_oob;
	int allocated_oob = 0;
	loff_t seek;
	int ret = 0;

	if (offs & (info->page_size - 1)) {
		eprintf("offset not page-aligned [0x%012llx]\n", offs);
		return 1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return 1;
	}

	if (ioctl(mtd->fd, MTDFILEMODE, MTD_FILE_MODE_RAW) != 0) {
		eprintf("failed to enter RAW mode");
		return 1;
	}

	seek = (loff_t)offs;
	if (lseek(mtd->fd, seek, SEEK_SET) != seek) {
		eprintf("failed to seek MTD device\n");
		return 1;
	}

	if (read(mtd->fd, page, info->page_size) != info->page_size) {
		eprintf("failed to read page data\n");
		return 1;
	}

	/* For BCH remapping, we must have "OOB" data */
	if (!oob && mtd->bch_remap) {
		oob = xxmalloc(info->oob_size);
		allocated_oob = 1;
	}

	if (oob) {
		mtd_oob.length = info->oob_size;
		mtd_oob.usr_ptr = (uint64_t)(unsigned long)oob;
		mtd_oob.start = offs;
		if (ioctl(mtd->fd, MEMREADOOB64, &mtd_oob) != 0) {
			eprintf("failed to read oob data\n");
			ret = 1;
			goto out1;
		}
	}

	if (mtd->bch_remap)
		nand_ecc_bch_remap(info, page, oob,
				   (mtd->bch_remap == REMAP_BCH18) ?
				   BCH_MODE_18BIT : BCH_MODE_30BIT);

 out1:
	if (allocated_oob)
		free(oob);

	return ret;
}

int nandio_write_page_raw(void *iohandle, struct nand_info *info,
			  uint8_t *page, uint8_t *oob, uint64_t offs)
{
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)iohandle;
	struct mtd_oob_buf64 mtd_oob;
	uint8_t *tmp = NULL;
	loff_t seek;
	int ret = 0;

	if (offs & (info->page_size - 1)) {
		eprintf("offset not page-aligned [0x%012llx]\n", offs);
		return 1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return 1;
	}

	if (mtd->bch_remap) {
		tmp = xxmalloc(info->page_size + info->oob_size);
		memcpy(tmp, page, info->page_size);
		memcpy(tmp + info->page_size, oob, info->oob_size);
		page = tmp;
		oob = tmp + info->page_size;
		nand_ecc_bch_unmap(info, page, oob,
				   (mtd->bch_remap == REMAP_BCH18) ?
				   BCH_MODE_18BIT : BCH_MODE_30BIT);
	}

	if (ioctl(mtd->fd, MTDFILEMODE, MTD_FILE_MODE_RAW) != 0) {
		eprintf("failed to enter RAW mode");
		ret = 1;
		goto out1;
	}

	seek = (loff_t)offs;
	if (lseek(mtd->fd, seek, SEEK_SET) != seek) {
		eprintf("failed to seek MTD device\n");
		ret = 1;
		goto out1;
	}

	if (write(mtd->fd, page, info->page_size) != info->page_size) {
		eprintf("failed to write page data\n");
		ret = 1;
		goto out1;
	}

	if (oob) {
		mtd_oob.length = info->oob_size;
		mtd_oob.start = offs;
		mtd_oob.usr_ptr = (uint64_t)(unsigned long)oob;
		if (ioctl(mtd->fd, MEMWRITEOOB64, &mtd_oob) != 0) {
			eprintf("failed to write oob data\n");
			ret = 1;
			goto out1;
		}
	}

 out1:
	if (tmp)
		free(tmp);
	return ret;
}

int nandio_erase_block(void *iohandle, struct nand_info *info, uint64_t offs)
{
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)iohandle;
	struct erase_info_user64 erase;

	if (offs & (info->block_size - 1)) {
		eprintf("offset not block-aligned [0x%012llx]\n", offs);
		return 1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return 1;
	}


	erase.start = offs;
	erase.length = info->block_size;

	if (ioctl(mtd->fd, MEMERASE64, &erase) < 0) {
		eprintf("memerase operation failed\n");
		return 1;
	}

	return 0;
}
