/*
 *  nand_driver_raw.c
 *
 *    'nand_driver' interface, based on raw 'nand_io' layer
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

#include <nand.h>
#include <nand_ecc.h>
#include <nand_io_ecc.h>
#include <nand_bbt.h>
#include <nand_io_bbt.h>
#include <nand_driver.h>
#include <xprintf.h>

struct driver_data {
	int		ecc_mode;
	int		bbt_type;
	uint8_t		*bbt;
	uint64_t	bbt_pair_offs[2];
	uint8_t		bbt_pair_vers[2];
	uint8_t		*oob;
};


struct nand_driver *nand_driver_init(void *_config)
{
	struct nand_driver_raw_config *config = _config;
	struct nand_driver *driver;
	struct driver_data *data;
	struct nand_bbt_stats bbt_stats;

	if (config->ecc_mode != STMNAND_ECC_AFM &&
	    config->ecc_mode != STMNAND_ECC_FLEX &&
	    config->ecc_mode != STMNAND_ECC_BCH18 &&
	    config->ecc_mode != STMNAND_ECC_BCH30) {
		eprintf("unsupported ECC mode [%d]\n", config->ecc_mode);
		return NULL;
	}

	if ((config->ecc_mode == STMNAND_ECC_BCH18 ||
	     config->ecc_mode == STMNAND_ECC_BCH30) &&
	    !(config->bbt_options & NAND_DRIVER_BBT_FLASH)) {
		eprintf("BCH ECC mandates use of Flash-resident BBTs\n");
		return NULL;
	}

	driver = xxmalloc(sizeof(*driver));
	data = xxmalloc(sizeof(*data));
	driver->priv = data;

	data->ecc_mode = config->ecc_mode;
	data->bbt_pair_vers[0] = 0;
	data->bbt_pair_vers[1] = 0;
	data->bbt_pair_offs[0] = 0;
	data->bbt_pair_offs[1] = 0;

	if (config->bbt_options & NAND_DRIVER_BBT_FLASH) {
		if (data->ecc_mode == STMNAND_ECC_FLEX ||
		    data->ecc_mode == STMNAND_ECC_AFM)
			data->bbt_type = STMNAND_BBT_LINUX;
		else
			data->bbt_type = STMNAND_BBT_INBAND;
	}

	driver->iohandle = nand_io_init(config->ioconfig, &driver->info);
	if (!driver->iohandle)
		goto err1;

	data->oob = xxmalloc(driver->info.oob_size);

	/* Check for existing Flash-resident BBTs. */
	data->bbt = nandio_bbt_load(driver->iohandle, &driver->info,
				    STMNAND_ECC_DETECT, STMNAND_BBT_DETECT,
				    &bbt_stats);

	if (config->bbt_options & NAND_DRIVER_BBT_FLASH) {
		/*
		 * Configured for Flash-resident BBTs.
		 */
		if (data->bbt) {
			/* Found Existing BBT. */

			/* Check it matches driver settings. */
			if (data->ecc_mode != bbt_stats.ecc ||
			    data->bbt_type != bbt_stats.bbt) {
				eprintf("Found 'alien' BBT [%s:%s:%s] at 0x%012llx\n",
					nand_bbt_bak_strs[bbt_stats.backup],
					nand_bbt_strs[bbt_stats.bbt],
					nand_ecc_strs[bbt_stats.ecc],
					bbt_stats.offs);
				goto err3;
			}

			if (verbose) {
				printf("Loaded BBT from Flash\n");
				nand_bbt_dump_stats(&bbt_stats);
			}

			/* Sync. Primary and Mirror BBTs. */
			if (nandio_bbt_sync_pair(driver->iohandle, &driver->info,
						 data->bbt, &bbt_stats,
						 data->bbt_pair_offs,
						 data->bbt_pair_vers) != 0) {
				eprintf("Failed to sync Primary/Mirror BBTs\n");
				goto err3;
			}
		} else {
			/* No BBTs found. */
			vbprintf("Failed to find Flash-resident BBTs\n");
			if (!(config->bbt_options & NAND_DRIVER_BBT_FLASH_CREATE)) {
				eprintf("Enable creation of Flash-resident BBTs, or disable Flash-resident BBT support\n");
				goto err2;
			}

			/* Scan for bad blocks. */
			vbprintf("Scanning device for bad blocks\n");
			data->bbt = nandio_bbt_scan_bbms(driver->iohandle,
							 &driver->info,
							 config->scan_mode, verbose);
			if (!data->bbt) {
				eprintf("Failed to scan device for bad blocks\n");
				goto err2;
			}

			/* Write BBTs to Flash. */
			if (nandio_bbt_update(driver->iohandle, &driver->info,
					      data->bbt,
					      data->ecc_mode, data->bbt_type,
					      data->bbt_pair_vers[0] + 1,
					      NAND_BBT_UPDATE_BOTH,
					      data->bbt_pair_offs,
					      data->bbt_pair_vers) != 0) {
				eprintf("failed to write BBTs\n");
				goto err3;
			}
		}
	} else {
		/*
		 * Configured for no Flash-resident BBTs.
		 */
		if (data->bbt) {
			eprintf("Found valid BBT [%s:%s:%s] at 0x%012llx\n",
				nand_bbt_bak_strs[bbt_stats.backup],
				nand_bbt_strs[bbt_stats.bbt],
				nand_ecc_strs[bbt_stats.ecc],
				bbt_stats.offs);
			eprintf("Clean device first, or enable Flash-resident BBT support\n");
			goto err3;

		}

		/* Scan for bad blocks. */
		vbprintf("Scanning device for bad blocks\n");
		data->bbt = nandio_bbt_scan_bbms(driver->iohandle,
						 &driver->info,
						 config->scan_mode, verbose);
		if (!data->bbt) {
			eprintf("Failed to scan device for bad blocks\n");
			goto err2;
		}
	}

	if (verbose)
		nand_bbt_dump(&driver->info, data->bbt, "Bad Blocks");

	/* Success */
	return driver;

 err3:
	free(data->bbt);
 err2:
	nand_io_exit(driver->iohandle);
	free(data->oob);
 err1:
	free(data);
	free(driver);
	return NULL;
}

void nand_driver_exit(struct nand_driver *driver)
{
	struct driver_data *data = driver->priv;

	nand_io_exit(driver->iohandle);
	free(data->bbt);
	free(data->oob);
	free(data);
	free(driver);
}


int nand_read_page(struct nand_driver *driver, uint8_t *page, uint64_t offs)
{
	struct driver_data *data = (struct driver_data *)
		driver->priv;
	struct nand_ecc_stats stats;
	int ret;

	ret = nandio_read_page_ecc(driver->iohandle, &driver->info,
				   page, data->oob, offs, data->ecc_mode,
				   &stats);

	return ret;

}


int nand_write_page(struct nand_driver *driver, uint8_t *page, uint64_t offs)
{
	struct driver_data *data = (struct driver_data *)
		driver->priv;
	int ret;

	ret = nandio_write_page_ecc(driver->iohandle, &driver->info,
				    page, data->oob, offs, data->ecc_mode);

	return ret;
}

int nand_erase_block(struct nand_driver *driver, uint64_t offs)
{
	int ret;

	ret = nand_is_block_bad(driver, offs);
	if (ret < 0)
		return ret;

	if (ret == 1)
		eprintf("aborted attempt to erase bad block [0x%012llx]\n",
			offs);
	else
		ret = nandio_erase_block(driver->iohandle, &driver->info, offs);

	return ret;
}

int nand_is_block_bad(struct nand_driver *driver, uint64_t offs)
{
	struct nand_info *info = &driver->info;
	struct driver_data *data = (struct driver_data *)
		driver->priv;

	if (offs & (info->block_size - 1)) {
		eprintf("offset not block-aligned [0x%012llx]\n", offs);
		return -1;
	}

	if (offs >= info->size) {
		eprintf("offset is beyond end of device [0x%012llx]\n", offs);
		return -1;
	}

	return nand_bbt_isbad(data->bbt, offs >> driver->info.block_shift);
}

int nand_mark_block_bad(struct nand_driver *driver, uint64_t offs)
{
	struct driver_data *data = (struct driver_data *)
		driver->priv;
	struct nand_info *info = &driver->info;
	uint8_t *page, *oob;
	int ret;

	ret = nand_is_block_bad(driver, offs);
	if (ret < 0)
		return ret;

	if (ret == 1)
		return 0;

	nand_bbt_set_mark(data->bbt, offs >> info->block_shift,
			  BBT_MARK_BAD_WEAR);

	if (data->bbt_type == STMNAND_BBT_NONE) {
		page = xxmalloc(info->page_size + info->oob_size);
		oob = page + info->page_size;
		ret = nandio_write_bbm(driver->iohandle, &driver->info,
				       page, oob, offs);
		free(page);
	} else {
		ret = nandio_bbt_update(driver->iohandle, &driver->info,
					data->bbt,
					data->ecc_mode, data->bbt_type,
					data->bbt_pair_vers[0] + 1,
					NAND_BBT_UPDATE_BOTH,
					data->bbt_pair_offs,
					data->bbt_pair_vers);
	}

	return ret;
}
