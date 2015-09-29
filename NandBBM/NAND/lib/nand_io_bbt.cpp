/*
 *  nand_io_bbt.c
 *
 *    Support for reading/writing NAND BBTs (based on 'nand_io' interface)
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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <xprintf.h>
#include <nand.h>
#include <nand_bbt.h>
#include <nand_io.h>
#include <nand_io_ecc.h>
#include <nand_io_bbt.h>

#include "nand_ecc_hamming.h"

int nandio_write_bbm(void *iohandle, struct nand_info *info,
		     uint8_t *page, uint8_t *oob, uint64_t offs)
{
	nandio_erase_block(iohandle, info, offs);

	memset(page, 0xff, info->page_size);
	strcpy((char *)page, "Marked bad by STM NAND Tools");
	memset(oob, 0x00, info->oob_size);

	return nandio_write_page_raw(iohandle, info, page, oob, offs);
}

int nandio_bbt_linux_scan_block(void *iohandle, struct nand_info *info,
				uint64_t offs, int ecc_mode,
				struct nand_bbt_stats *bbt_stats)
{
	struct nand_ecc_stats ecc_stats;
	uint8_t *page, *oob;
	int ret;

	if (ecc_mode != STMNAND_ECC_DETECT &&
	    ecc_mode != STMNAND_ECC_NONE &&
	    ecc_mode != STMNAND_ECC_FLEX &&
	    ecc_mode != STMNAND_ECC_AFM) {
		eprintf("invalid ECC mode [%s]\n", nand_ecc_strs[ecc_mode]);
			return -1;
	}

	ret = 0;
	bbt_stats->offs = offs;
	bbt_stats->bbt = STMNAND_BBT_NONE;
	bbt_stats->backup = -1;
	bbt_stats->version = 0;

	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;

	if (nandio_read_page_ecc(iohandle, info, page, oob, offs, ecc_mode,
				 &ecc_stats) != 0) {
		eprintf("failed to read page\n");
		ret = -1;
		goto out1;
	}

	bbt_stats->ecc = ecc_stats.ecc;

	if (bbt_stats->ecc == STMNAND_ECC_NONE ||
	    bbt_stats->ecc == STMNAND_ECC_FLEX ||
	    bbt_stats->ecc == STMNAND_ECC_AFM) {
		if (nand_bbt_linux_scan_sig(oob, &bbt_stats->backup,
					    &bbt_stats->version)) {
			bbt_stats->bbt = STMNAND_BBT_LINUX;
			ret = 1;
		}
	}

 out1:
	free(page);

	return ret;
}

int nandio_bbt_inband_scan_block(void *iohandle, struct nand_info *info,
				 uint64_t offs, int ecc_mode,
				 struct nand_bbt_stats *bbt_stats)
{
	struct nand_ecc_stats ecc_stats;
	uint8_t ibbt_ecc_size;
	uint8_t *page, *oob;
	int ret = 0;

	bbt_stats->bbt = STMNAND_BBT_NONE;
	bbt_stats->offs = offs;
	bbt_stats->backup = -1;
	bbt_stats->version = 0;

	if (ecc_mode != STMNAND_ECC_DETECT &&
	    ecc_mode != STMNAND_ECC_NONE &&
	    ecc_mode != STMNAND_ECC_BCH18 &&
	    ecc_mode != STMNAND_ECC_BCH30) {
		eprintf("invalid ECC mode [%s]\n", nand_ecc_strs[ecc_mode]);
		return -1;
	}

	offs += info->block_size - info->page_size;
	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;

	if (nandio_read_page_ecc(iohandle, info, page, oob, offs, ecc_mode,
				 &ecc_stats) != 0) {
		eprintf("failed to read page\n");
		ret = -1;
		goto out1;
	}
	bbt_stats->ecc = ecc_stats.ecc;

	if (bbt_stats->ecc != STMNAND_ECC_NONE &&
	    bbt_stats->ecc != STMNAND_ECC_BCH18 &&
	    bbt_stats->ecc != STMNAND_ECC_BCH30)
		goto out1;

	if (nand_bbt_inband_scan_sig(page, &bbt_stats->backup,
				     &bbt_stats->version,
				     &ibbt_ecc_size,
				     bbt_stats->ibbt_author) == 0)
		goto out1;

	if (bbt_stats->ecc == STMNAND_ECC_BCH30 &&
	    ibbt_ecc_size != BCH30_ECC_BYTES)
		goto out1;

	if (bbt_stats->ecc == STMNAND_ECC_BCH18 &&
	    ibbt_ecc_size != BCH18_ECC_BYTES)
		goto out1;

	bbt_stats->bbt = STMNAND_BBT_INBAND;
	ret = 1;

 out1:
	free(page);

	return ret;
}

int nandio_bbt_scan_block(void *iohandle, struct nand_info *info,
			  uint64_t offs, int ecc_mode, int bbt_type,
			  struct nand_bbt_stats *bbt_stats)
{
	if (offs & (info->block_size - 1)) {
		eprintf("offset not block-aligned\n");
		return -1;
	}

	if (ecc_mode == STMNAND_ECC_EMPTYPAGE ||
	    ecc_mode == STMNAND_ECC_UNKNOWN ||
	    ecc_mode == STMNAND_ECC_BOOT) {
		eprintf("invalid ECC mode [%s]\n", nand_ecc_strs[ecc_mode]);
		return -1;
	}

	if (bbt_type == STMNAND_BBT_LINUX)
		return  nandio_bbt_linux_scan_block(iohandle, info, offs,
						    ecc_mode, bbt_stats);

	if (bbt_type == STMNAND_BBT_INBAND)
		return  nandio_bbt_inband_scan_block(iohandle, info, offs,
						     ecc_mode, bbt_stats);

	if (bbt_type == STMNAND_BBT_DETECT) {
		if (ecc_mode == STMNAND_ECC_DETECT ||
		    ecc_mode == STMNAND_ECC_NONE ||
		    ecc_mode == STMNAND_ECC_FLEX ||
		    ecc_mode == STMNAND_ECC_AFM)
			if (nandio_bbt_linux_scan_block(iohandle, info, offs,
							ecc_mode, bbt_stats) == 1)
				return 1;

		if (ecc_mode == STMNAND_ECC_DETECT ||
		    ecc_mode == STMNAND_ECC_NONE ||
		    ecc_mode == STMNAND_ECC_BCH18 ||
		    ecc_mode == STMNAND_ECC_BCH30)
			if (nandio_bbt_inband_scan_block(iohandle, info, offs,
							 ecc_mode, bbt_stats) == 1)
				return 1;
	}

	return 0;
}

char *nand_bbt_scan_strs[] = {
	[NAND_BBT_SCAN_BBM]	= "bbm",
	[NAND_BBT_SCAN_TAG]	= "tag",
	[NAND_BBT_SCAN_ECC]	= "ecc",
};

int nand_str2bbtscanmode(char *str)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nand_bbt_scan_strs); i++)
		if (nand_bbt_scan_strs[i] && strcmp(str, nand_bbt_scan_strs[i]) == 0)
			return i;

	return -1;
}

int nandio_bbt_scan_bbms_block(void *iohandle, struct nand_info *info,
			       uint64_t offs, uint8_t *page, uint8_t *oob,
			       int mode)
{
	struct nand_ecc_stats stats;
	int bbm_pos = (info->page_size == 512) ? 5 : 0;

	if (nandio_read_page_raw(iohandle, info, page, oob, offs) != 0) {
		eprintf("failed to read page at 0x%012llx\n", offs);
		return -1;
	}

	if ((mode & NAND_BBT_SCAN_BBM) == NAND_BBT_SCAN_BBM) {
		if (oob[bbm_pos] == 0xff)
			return 0;
	}

	if ((mode & NAND_BBT_SCAN_TAG) == NAND_BBT_SCAN_TAG) {
		if (nand_ecc_afm_check_tag(info, oob, 1) == 0)
			return 0;

		if (nand_ecc_boot_check_tag(info, oob, 1) == 0)
			return 0;
	}

	if ((mode & NAND_BBT_SCAN_ECC) == NAND_BBT_SCAN_ECC) {
		nand_ecc_decode(info, page, oob, STMNAND_ECC_DETECT, &stats);
		if (stats.ecc != STMNAND_ECC_UNKNOWN &&
		    stats.ecc != STMNAND_ECC_FLEX)
			return 0;
	}

	return 1;
}

uint8_t *nandio_bbt_scan_bbms(void *iohandle, struct nand_info *info,
			      int mode, int show_progress)
{
	uint8_t *bbt, *page, *oob;
	int block_count = info->size / info->block_size;
	int bbt_size = block_count/4;
	int i, bad;
	uint64_t offs;

	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;
	bbt = xxmalloc(bbt_size);

	memset(bbt, 0xff, bbt_size);
	offs = 0;
	for (i = 0; i < block_count; i++) {
		if (show_progress) {
			printf("\r\t0x%012llx [%2d%%]",
			       offs, (i * 100) / block_count);
			fflush(stdout);
		}

		bad = nandio_bbt_scan_bbms_block(iohandle, info, offs,
						 page, oob, mode);
		if (bad < 0) {
			eprintf("\nfailed to scan block %04d\n", i);
			free(bbt);
			bbt = NULL;
			break;
		}

		if (bad)
			nand_bbt_set_mark(bbt, i, BBT_MARK_BAD_FACTORY);

		offs += info->block_size;
	}
	if (show_progress) {
		printf("\n");
	}

	free(page);
	return bbt;
}


int nandio_bbt_linux_write_data(void *iohandle, struct nand_info *info, uint8_t *bbt,
				int ecc_mode, uint8_t vers, int bak, uint64_t offs)
{
	uint8_t *page, *oob;
	int bbt_size;
	int first_page = 1;
	int write_size;
	int ret = 0;

	if (ecc_mode != STMNAND_ECC_FLEX &&
	    ecc_mode != STMNAND_ECC_AFM) {
		eprintf("invalid ECC mode [%s]\n", nand_ecc_strs[ecc_mode]);
		return 1;
	}

	bbt_size = (info->size/info->block_size)/4;
	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;

	while (bbt_size) {
		memset(page, 0xff, info->page_size + info->oob_size);

		write_size = min(info->page_size, bbt_size);
		memcpy(page, bbt, write_size);

		if (first_page) {
			nand_bbt_linux_fill_sig(oob, bak, vers);
			first_page = 0;
		}

		ret = nandio_write_page_ecc(iohandle, info, page, oob, offs, ecc_mode);

		if (ret)
			break;

		bbt_size -= write_size;
		bbt += write_size;
		offs += write_size;
	}

	free(page);

	return ret;
}


int nandio_bbt_inband_write_data(void *iohandle, struct nand_info *info,
				 uint8_t *bbt, int ecc_mode, uint8_t vers,
				 int bak, uint64_t offs)
{
	uint8_t *page, *oob;
	int bbt_size;
	int ecc_size;

	int ret = 0;

	if (ecc_mode != STMNAND_ECC_BCH18 &&
	    ecc_mode != STMNAND_ECC_BCH30) {
		eprintf("invalid ECC mode [%s]\n", nand_ecc_strs[ecc_mode]);
		return 1;
	}

	bbt_size = (info->size/info->block_size)/4;
	if (bbt_size > info->page_size) {
		eprintf("need to implement multi-page inband BBTs!\n");
		return 1;
	}

	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;

	memset(page, 0xff, info->page_size);
	memcpy(page, bbt, bbt_size);

	/* Write BBT contents to first page of block*/
	ret = nandio_write_page_ecc(iohandle, info, page, oob, offs, ecc_mode);
	if (ret != 0)
		goto out1;

	/* Write IBBT signature to last page of block */
	memset(page, 0xff, info->page_size);
	ecc_size = (ecc_mode == STMNAND_ECC_BCH18) ?
		BCH18_ECC_BYTES : BCH30_ECC_BYTES;
	nand_bbt_inband_fill_sig(page, bak, vers, ecc_size);
	offs += info->block_size - info->page_size;
	ret = nandio_write_page_ecc(iohandle, info, page, oob, offs, ecc_mode);
	if (ret != 0)
		goto out1;

 out1:
	free(page);

	return ret;
}

uint8_t *nandio_bbt_load(void *iohandle, struct nand_info *info,
			 int ecc_mode, int bbt_type,
			 struct nand_bbt_stats *bbt_stats)
{
	struct nand_bbt_stats stats[NAND_BBT_NBLOCKS];
	struct nand_ecc_stats ecc_stats;
	int bbt_size = (info->size/info->block_size)/4;
	int read_size;
	int i;
	uint64_t offs;
	uint8_t *b, *bbt, *page, *oob;
	int found = -1;

	offs = info->size - info->block_size;
	for (i = 0; i < NAND_BBT_NBLOCKS; i++) {
		if (nandio_bbt_scan_block(iohandle, info, offs,
					  ecc_mode, bbt_type, &stats[i]) < 0)
			eprintf("error while testing block at 0x%012llx\n", offs);
		offs -= info->block_size;
	}

	for (i = 0; i < NAND_BBT_NBLOCKS; i++) {
		if (stats[i].bbt == STMNAND_BBT_NONE)
			continue;

		if (found < 0) {
			found = i;
			continue;
		}

		if (ecc_mode == STMNAND_ECC_DETECT) {
			if (stats[i].ecc != stats[found].ecc) {
				eprintf("found BBTs with different ECC schemes\n");
				return NULL;
			}
		} else {
			if (stats[i].ecc != ecc_mode)
				continue;
		}

		if (bbt_type == STMNAND_BBT_DETECT) {
			if (stats[i].bbt != stats[found].bbt) {
				eprintf("found BBTs of different types\n");
				return NULL;
			}
		} else {
			if (stats[i].bbt != bbt_type)
				continue;
		}

		if ((int8_t)(stats[i].version - stats[found].version) > 0)
			found = i;
	}

	if (found < 0)
		return NULL;

	*bbt_stats = stats[found];
	bbt = xxmalloc(bbt_size);
	page = xxmalloc(info->page_size + info->oob_size);
	oob = page + info->page_size;

	b = bbt;
	offs = bbt_stats->offs;
	while (bbt_size) {
		read_size = min(bbt_size, info->page_size);

		if (nandio_read_page_ecc(iohandle, info, page, oob,
					 offs, bbt_stats->ecc,
					 &ecc_stats) != 0) {
			eprintf("failed to load BBT\n");
			found = -1;
			break;
		}

		memcpy(b, page, read_size);

		b += read_size;
		bbt_size -= read_size;
		offs += read_size;
	}

	free(page);

	if (found < 0) {
		free(bbt);
		bbt = NULL;
	}

	return bbt;
}



static int nandio_bbt_write_data(void *iohandle, struct nand_info *info, uint8_t *bbt,
				 int ecc_mode, int bbt_type,
				 uint8_t vers, int bak, uint64_t offs)
{
	if (bbt_type == STMNAND_BBT_LINUX)
		return nandio_bbt_linux_write_data(iohandle, info, bbt,
						   ecc_mode, vers, bak, offs);

	if (bbt_type == STMNAND_BBT_INBAND)
		return nandio_bbt_inband_write_data(iohandle, info, bbt,
						    ecc_mode, vers, bak, offs);

	eprintf("invalid BBT type [%s]\n", nand_bbt_strs[bbt_type]);

	return 1;
}

static int nandio_bbt_write(void *iohandle, struct nand_info *info, uint8_t *bbt,
			    int ecc_mode, int bbt_type, uint8_t vers, int bak,
			    uint64_t bbt_pair_offs[2], uint8_t bbt_pair_vers[2])
{
	int block_shift = ffs64(info->block_size) - 1;
	int block;
	uint64_t offs;
	uint64_t offs_other;
	int i;

	offs_other = bak ? bbt_pair_offs[0] : bbt_pair_offs[1];

	for (i = 0, offs = info->size - info->block_size;
	     i < NAND_BBT_NBLOCKS;
	     i++, offs -= info->block_size) {
		block = offs >> block_shift;

		/* Skip if block used by other table */
		if (offs == offs_other)
			continue;

		/* Skip if block is marked bad */
		if (nand_bbt_isbad(bbt, block))
			continue;

		if (nandio_erase_block(iohandle, info, offs) != 0) {
			eprintf("failed to erase block [%u:0x%012llx] while updating BBT\n",
				block, offs);
			vers++;
			nand_bbt_set_mark(bbt, block, BBT_MARK_BAD_WEAR);
			continue;
		}

		/* Erase block, mark bad and skip on failure */
		if (nandio_bbt_write_data(iohandle, info, bbt, ecc_mode, bbt_type,
					  vers, bak, offs) != 0) {
			eprintf("failed to write BBT to block [%u:0x%012llx]\n",
				block, offs);
			/* Note, the BBT may be have been partially written and
			 * identifiable as a valid BBT.  To ensure subsequent
			 * BBTs take precedance, update the version number. */
			vers++;
			nand_bbt_set_mark(bbt, block, BBT_MARK_BAD_WEAR);
			continue;
		}

		/* Success */
		bbt_pair_offs[bak] = offs;
		bbt_pair_vers[bak] = vers;

		vbprintf("wrote BBT [%s:%d] at 0x%012llx\n",
			 nand_bbt_bak_strs[bak], vers, offs);

		return 0;
	}

	eprintf("no space left in BBT area\n");
	eprintf("failed to update %s BBT\n",  nand_bbt_bak_strs[bak]);

	return 1;
}

/* Update BBT(s), incrementing 'vers' number if required, and ensuring Primary
 * and Mirror are kept in sync */
int nandio_bbt_update(void *iohandle, struct nand_info *info, uint8_t *bbt,
		      int ecc_mode, int bbt_type, uint8_t vers, uint32_t update,
		      uint64_t bbt_pair_offs[2], uint8_t bbt_pair_vers[2])
{
	int err;

	do {
		/* Update Primary if specified */
		if (update & NAND_BBT_UPDATE_PRIMARY) {
			err = nandio_bbt_write(iohandle, info, bbt,
					       ecc_mode, bbt_type, vers, 0,
					       bbt_pair_offs, bbt_pair_vers);

			/* Bail out on error (e.g. no space left in BBT area) */
			if (err)
				return err;

			/* If update resulted in a new BBT version
			 * (e.g. Erase/Write fail on BBT block) update version
			 * here, and force update of other table.
			 */
			if (bbt_pair_vers[0] != vers) {
				vers = bbt_pair_vers[0];
				update = NAND_BBT_UPDATE_MIRROR;
			}
		}

		/* Update Mirror if specified */
		if (update & NAND_BBT_UPDATE_MIRROR) {
			err = nandio_bbt_write(iohandle, info, bbt,
					       ecc_mode, bbt_type, vers, 1,
					       bbt_pair_offs, bbt_pair_vers);
			if (err)
				return err;

			/* If update resulted in a new BBT version
			 * (e.g. Erase/Write fail on BBT block) update version
			 * here, and force update of other table.
			 */
			if (bbt_pair_vers[1] != vers) {
				vers = bbt_pair_vers[1];
				update = NAND_BBT_UPDATE_PRIMARY;
			}

		}

		/* Continue, until Primary and Mirror versions are in sync */
	} while (bbt_pair_vers[0] != bbt_pair_vers[1]);

	return 0;
}

int nandio_bbt_sync_pair(void *iohandle, struct nand_info *info, uint8_t *bbt,
			 struct nand_bbt_stats *bbt_stats,
			 uint64_t bbt_pair_offs[2], uint8_t bbt_pair_vers[2])
{
	struct nand_bbt_stats stats;
	uint32_t update;

	int i;
	uint64_t offs;
	int block_shift = ffs64(info->block_size) - 1;
	int bak = bbt_stats->backup;
	int oth = bak ? 0 : 1;
	int ret;

	bbt_pair_offs[bak] = bbt_stats->offs;
	bbt_pair_vers[bak] = bbt_stats->version;

	bbt_pair_offs[oth] = 0;
	bbt_pair_vers[oth] = 0;

	for (i = 0, offs = info->size - info->block_size;
	     i < NAND_BBT_NBLOCKS;
	     i++, offs -= info->block_size) {

		if (offs == bbt_stats->offs)
			continue;

		if (nandio_bbt_scan_block(iohandle, info, offs,
					  bbt_stats->ecc, bbt_stats->bbt,
					  &stats) < 0) {
			eprintf("error while scanning block [%d:0x%012llx] for BBT\n",
				(int)(offs >> block_shift), offs);
			continue;
		}

		if (stats.bbt == bbt_stats->bbt &&
		    stats.ecc == bbt_stats->ecc &&
		    stats.version == bbt_stats->version &&
		    stats.backup == oth) {
			bbt_pair_offs[oth] = stats.offs;
			bbt_pair_vers[oth] = stats.version;
			return 0;
		}
	}

	update = oth ? NAND_BBT_UPDATE_MIRROR : NAND_BBT_UPDATE_PRIMARY;

	ret = nandio_bbt_update(iohandle, info, bbt, bbt_stats->ecc,
				bbt_stats->bbt, bbt_stats->version,
				update, bbt_pair_offs, bbt_pair_vers);

	return ret;
}
