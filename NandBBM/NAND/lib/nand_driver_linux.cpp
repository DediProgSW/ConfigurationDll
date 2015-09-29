/*
 *  nand_driver_linux.c
 *
 *    'nand_driver' interface, based on MTD-linux layer
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
#include <nand_ecc.h>
#include <nand_io_ecc.h>
#include <nand_bbt.h>
#include <nand_io_bbt.h>
#include <nand_driver.h>
#include <xprintf.h>

#include "nand_io_mtd.h"

struct nand_driver *nand_driver_init(void *_config)
{
	struct nand_driver_linux_config *config = _config;
	struct nand_io_mtd_config *ioconfig = config->ioconfig;
	struct nand_driver *driver;
	struct nand_io_mtd *mtd;
	int ecc_mode = config->ecc_mode;
	int ecc_mismatch;

	driver = xxmalloc(sizeof(*driver));

	mtd = nand_io_init(ioconfig, &driver->info);
	if (!mtd) {
		free(driver);
		return NULL;
	}
	driver->iohandle = mtd;

	if (mtd->driver == -1) {
		eprintf("WARNING: STLinux NAND driver not known:\n");
		eprintf("\tUnable to confirm ECC compatibility [%s]\n",
			nand_ecc_strs[ecc_mode]);
	} else {
		ecc_mismatch = 0;
		switch (ecc_mode) {
		case STMNAND_ECC_FLEX:
			if (mtd->driver != STLINUX_NAND_FLEX &&
			    mtd->driver != STLINUX_NAND_EMI)
				ecc_mismatch = 1;
			break;
		case STMNAND_ECC_AFM:
			if (mtd->driver != STLINUX_NAND_AFM)
				ecc_mismatch = 1;
			break;
			/*
			 *  Note, for BCH18 and BCH30 we tolerate 'bch_remap ==
			 *  REMAP_NONE', since this may be due to using an old
			 *  version of the stm_nand_bch driver which fails to
			 *  report the number of ECC bytes used.
			 */
		case STMNAND_ECC_BCH18:
			if (mtd->driver != STLINUX_NAND_BCH ||
			    mtd->bch_remap == REMAP_BCH30)
				ecc_mismatch = 1;
			break;
		case STMNAND_ECC_BCH30:
			if (mtd->driver != STLINUX_NAND_BCH ||
			    mtd->bch_remap == REMAP_BCH18)
				ecc_mismatch = 1;
			break;
		default:
			eprintf("Invalid ECC mode [%d]\n", ecc_mode);
			nand_io_exit(driver->iohandle);
			free(driver);
			return NULL;
		}

		if (ecc_mismatch) {
			eprintf("STLinux NAND driver [%s] does not match specified ECC mode [%s]\n",
				stlinux_nand_drivers[mtd->driver],
				nand_ecc_strs[config->ecc_mode]);

			nand_io_exit(driver->iohandle);
			free(driver);
			return NULL;
		}

		if ((ecc_mode == STMNAND_ECC_BCH18 ||
		     ecc_mode == STMNAND_ECC_BCH30) &&
		    mtd->bch_remap == REMAP_NONE) {
			eprintf("WARNING: Overriding detected 'BCH_NO_ECC' mode with 'BCH_%sBIT_ECC'\n",
				(ecc_mode == STMNAND_ECC_BCH18) ? "18" : "30");
			mtd->bch_remap = (ecc_mode == STMNAND_ECC_BCH18) ?
				REMAP_BCH18 : REMAP_BCH30;
		}
	}

	return driver;
}

void nand_driver_exit(struct nand_driver *driver)
{
	nand_io_exit(driver->iohandle);

	free(driver);
}


int nand_read_page(struct nand_driver *driver, uint8_t *page, uint64_t offs)
{
	struct nand_info *info = &driver->info;
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)driver->iohandle;
	loff_t loffs = (loff_t)offs;

	if (offs & (info->page_size - 1)) {
		eprintf("offset not page-aligned [0x%012llx]\n", offs);
		return 1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return 1;
	}

	if (ioctl(mtd->fd, MTDFILEMODE, (void *)MTD_FILE_MODE_NORMAL) != 0) {
		eprintf("failed to switch to 'normal' ECC mode\n");
		return 1;
	}

	if (lseek(mtd->fd, loffs, SEEK_SET) != loffs) {
		eprintf("failed to seek to offset [0x%012llx]\n", offs);
		return 1;
	}

	if (read(mtd->fd, page, info->page_size) != info->page_size) {
		eprintf("failed to write page data\n");
		return 1;
	}

	return 0;
}

int nand_write_page(struct nand_driver *driver, uint8_t *page, uint64_t offs)
{
	struct nand_info *info = &driver->info;
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)driver->iohandle;
	loff_t loffs = (loff_t)offs;

	if (offs & (info->page_size - 1)) {
		eprintf("offset not page-aligned [0x%012llx]\n", offs);
		return 1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return 1;
	}

	if (ioctl(mtd->fd, MTDFILEMODE, (void *)MTD_FILE_MODE_NORMAL) != 0) {
		eprintf("failed to switch to 'normal' ECC mode\n");
		return 1;
	}

	if (lseek(mtd->fd, loffs, SEEK_SET) != loffs) {
		eprintf("failed to seek to offset [0x%012llx]\n", offs);
		return 1;
	}

	if (write(mtd->fd, page, info->page_size) != info->page_size) {
		eprintf("failed to write page data\n");
		return 1;
	}

	return 0;

}

int nand_erase_block(struct nand_driver *driver, uint64_t offs)
{
	int ret;

	ret = nand_is_block_bad(driver, offs);
	if (ret < 0)
		return ret;

	if (ret == 1) {
		eprintf("aborted attempt to erase bad block [0x%012llx]\n",
			offs);
	} else
		ret = nandio_erase_block(driver->iohandle, &driver->info, offs);

	return ret;
}

int nand_is_block_bad(struct nand_driver *driver, uint64_t offs)
{
	struct nand_info *info = &driver->info;
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)driver->iohandle;
	loff_t loffs = (loff_t)offs;
	int ret;

	if (offs & (info->block_size - 1)) {
		eprintf("offset not block-aligned [0x%012llx]\n", offs);
		return -1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return -1;
	}

	ret = ioctl(mtd->fd, MEMGETBADBLOCK, &loffs);
	if (ret < 0) {
		eprintf("memgetbadblock operation failed\n");
		ret = -1;
	}

	return ret;
}

int nand_mark_block_bad(struct nand_driver *driver, uint64_t offs)
{
	struct nand_io_mtd *mtd = (struct nand_io_mtd *)driver->iohandle;
	loff_t loffs = (loff_t)offs;
	int ret;

	ret = nand_is_block_bad(driver, offs);
	if (ret < 0)
		return ret;

	if (ret == 1)
		return 0;

	if (ioctl(mtd->fd, MEMSETBADBLOCK, &loffs) < 0) {
		eprintf("memsetbadblock operation failed\n");
		return 1;
	}

	return ret;
}
