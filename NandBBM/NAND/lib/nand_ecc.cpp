/*
 *  nand_ecc.c
 *
 *    Wrapper layer over ECC code
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
#include "stdafx.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xprintf.h>
#include <nand.h>
#include <nand_ecc.h>

#include "nand_ecc_hamming.h"
#include "nand_ecc_bch.h"

/*
char *nand_ecc_strs[] = {
	[STMNAND_ECC_NONE]	= "noecc",
	[STMNAND_ECC_FLEX]	= "flex",
	[STMNAND_ECC_BOOT]	= "boot",
	[STMNAND_ECC_AFM]	= "afm",
	[STMNAND_ECC_BCH18]	= "bch18",
	[STMNAND_ECC_BCH30]	= "bch30",

	[STMNAND_ECC_EMPTYPAGE] = "emtpypage",
	[STMNAND_ECC_UNKNOWN]	= "unknown",
	[STMNAND_ECC_DETECT]	= "detect",
};*/
char *nand_ecc_strs[] = {
	"noecc",
	"flex",
	"boot",
	"afm",
	"bch18",
	"bch30",

	"emtpypage",
	"unknown",
	"detect",
};

/*
Description
The ffs() function returns the position of the first (least significant) bit set in the word i.
The least significant bit is position 1 and the most significant position is, for example, 32 or 64. 
The functions ffsll() and ffsl() do the same but take arguments of possibly different size.

Return Value
These functions return the position of the first bit set, or 0 if no bits are set in i.
*/
int ffs(int fs)
{
	for (int i = 0; i < 128; i++)
	{
		if (((fs >> i) & 0x01) == 1)
			return i+1;
	}

	return 0;
}


int nand_str2eccmode(char *str)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nand_ecc_strs); i++)
		if (nand_ecc_strs[i] && strcmp(str, nand_ecc_strs[i]) == 0)
			return i;

	return -1;
}

void nand_ecc_dump_stats(struct nand_ecc_stats *stats)
{
	int i;

	printf("\tecc          : %s\n", nand_ecc_strs[stats->ecc]);
	if (stats->ecc == STMNAND_ECC_BOOT || stats->ecc == STMNAND_ECC_FLEX ||
	    stats->ecc == STMNAND_ECC_AFM ||  stats->ecc == STMNAND_ECC_BCH18 ||
	    stats->ecc == STMNAND_ECC_BCH30) {
		if (stats->n_errors < 0) {
			printf("\tn_errors     : uncorrectable errors\n");
		} else {
			printf("\tn_errors     : %d\n", stats->n_errors);
			if (stats->n_errors > 0) {
				for (i = 0; i < stats->n_errors; i++)
					printf("\t\t [%d:%d]\n",
					       stats->bit_err_pos[i]/8,
					       stats->bit_err_pos[i]%8);
			}
		}
		if (stats->tag_fail >= 0) {
			printf("\ttag_status   : %s\n",
			       (stats->tag_fail == 0) ? "present/intact" :
			       "missing/corrupt");
		}
	}
}

int nand_ecc_encode(struct nand_info *info, uint8_t *page, uint8_t *oob,
		    int ecc_mode)
{
	if (ecc_mode == STMNAND_ECC_NONE)
		return 0;

	if (ecc_mode == STMNAND_ECC_BOOT ||
	    ecc_mode == STMNAND_ECC_FLEX ||
	    ecc_mode == STMNAND_ECC_AFM) {

		if (!(info->page_size == 512 && info->oob_size == 16) &&
		    !(info->page_size == 2048 && info->oob_size == 64)) {
			eprintf("ECC Mode [%s] not supported on device "
				"geometry [%u:%u]\n",
				nand_ecc_strs[ecc_mode],
				(unsigned int)info->page_size,
				(unsigned int)info->oob_size);
			return 1;
		}

		switch (ecc_mode) {
		case STMNAND_ECC_BOOT:
			nand_ecc_boot_encode(info, page, oob);
			break;
		case STMNAND_ECC_FLEX:
			nand_ecc_flex_encode(info, page, oob);
			break;
		case STMNAND_ECC_AFM:
			nand_ecc_afm_encode(info, page, oob);
			break;
		}
		return 0;
	}

	if (ecc_mode == STMNAND_ECC_BCH18 ||
	    ecc_mode == STMNAND_ECC_BCH30) {
		int ecc_bytes_per_sector;
		int n_sectors;

		if (info->page_size < 2048) {
			eprintf("BCH ECC not supported on Small Page NAND\n");
			return 1;
		}
		n_sectors = info->page_size/BCH_SECTOR_BYTES;
		ecc_bytes_per_sector = (ecc_mode == STMNAND_ECC_BCH18) ?
			BCH18_ECC_BYTES : BCH30_ECC_BYTES;
		if (info->oob_size < n_sectors * ecc_bytes_per_sector) {
			eprintf("Device OOB too small for BCH%d ECC\n",
				(ecc_mode == STMNAND_ECC_BCH18) ? 18 : 30);
			return 1;
		}
		nand_ecc_bch_encode(info, page, oob,
				    (ecc_mode == STMNAND_ECC_BCH18) ?
				    BCH_MODE_18BIT : BCH_MODE_30BIT);

		return 0;
	}

	eprintf("Unrecognised ECC scheme [%d]\n", ecc_mode);

	return 1;

}

static int nand_ecc_device_supported(struct nand_info *info, int ecc_mode)
{
	int ecc_bytes_per_sector;
	int n_sectors;

	switch (ecc_mode) {
	case STMNAND_ECC_NONE:
		return 1;
		break;
	case STMNAND_ECC_BOOT:
	case STMNAND_ECC_FLEX:
	case STMNAND_ECC_AFM:
		if ((info->page_size == 512 && info->oob_size == 16) ||
		    (info->page_size == 2048 && info->oob_size == 64))
			return 1;
		else
			return 0;
		break;
	case STMNAND_ECC_BCH18:
	case STMNAND_ECC_BCH30:
		if (info->page_size < 2048)
			return 0;

		n_sectors = info->page_size/BCH_SECTOR_BYTES;
		ecc_bytes_per_sector = (ecc_mode == STMNAND_ECC_BCH18) ?
			BCH18_ECC_BYTES : BCH30_ECC_BYTES;
		if (info->oob_size < (n_sectors * ecc_bytes_per_sector))
			return 0;

		return 1;
	}

	return 0;
}

int nand_ecc_decode_mode(struct nand_info *info, uint8_t *page, uint8_t *oob,
				int ecc_mode, struct nand_ecc_stats *stats)
{
	if (!nand_ecc_device_supported(info, ecc_mode)) {
		eprintf("ECC Mode [%s] not supported on device "
			"geometry [%u:%u]\n", nand_ecc_strs[ecc_mode],
			(unsigned int)info->page_size,
			(unsigned int)info->oob_size);
		return -1;
	}

	stats->ecc = ecc_mode;
	stats->n_errors = 0;
	stats->tag_fail = 0;

	switch (ecc_mode) {
	case STMNAND_ECC_NONE:
		return 0;
	case STMNAND_ECC_BOOT:
		return nand_ecc_boot_decode(info, page, oob, stats);
	case STMNAND_ECC_FLEX:
		return nand_ecc_flex_decode(info, page, oob, stats);
	case STMNAND_ECC_AFM:
		return nand_ecc_afm_decode(info, page, oob, stats);
	case STMNAND_ECC_BCH18:
	case STMNAND_ECC_BCH30:
		return nand_ecc_bch_decode(info, page, oob,
					   (ecc_mode == STMNAND_ECC_BCH18) ?
					   BCH_MODE_18BIT : BCH_MODE_30BIT,
					   stats);
	}

	eprintf("Unrecognised ECC scheme [%d]\n", ecc_mode);

	return -1;
}


	int nand_ecc_decode_detect(struct nand_info *info,
		uint8_t *page, uint8_t *oob,
	struct nand_ecc_stats *stats)
	{
		int ecc_modes[] = { STMNAND_ECC_FLEX,
			STMNAND_ECC_AFM,
			STMNAND_ECC_BOOT,
			STMNAND_ECC_BCH18,
			STMNAND_ECC_BCH30 };
		int i, ecc;
		int page_size = info->page_size;
		int oob_size = info->oob_size;
		uint8_t *page_ecc, *oob_ecc;
		int ret = -1;
		uint32_t valid_eccs = 0;
		int n_valid;

		stats->ecc = STMNAND_ECC_UNKNOWN;

		/* Empty/Erased page? */
		if (nand_is_page_empty(info, page, oob, 1)) {
			stats->ecc = STMNAND_ECC_EMPTYPAGE;
			return 0;
		}

		/* All zeros?  In most cases, a page of all zeros denotes a bad block
		 * marker.  However, a page of all zeros is also consistent with Boot,
		 * AFM, and BCH ECC.  For Boot Mode and AFM ECC, we would either expect
		 * ECC tags, or the non-ECC bytes in OOB to be non-zero.  BCH18 is used
		 * typically on devices where it consumes the entire OOB area.  As such,
		 * a page of all zeros is indistinguishable from a block marked as bad
		 * with all zeros.  In any case, it is not possible to automatically
		 * determine the ECC mode for a page of all zeros, so return with error.
		 */
		if (nand_is_page_zeros(info, page, oob, 1))
			return -1;

		page_ecc = (uint8_t*)xxmalloc(page_size + oob_size);
		oob_ecc = page_ecc + page_size;

		for (i = 0; i < ARRAY_SIZE(ecc_modes); i++) {
			ecc = ecc_modes[i];

			if (!nand_ecc_device_supported(info, ecc))
				continue;

			memcpy(page_ecc, page, page_size);
			memcpy(oob_ecc, oob, oob_size);
			ret = nand_ecc_decode_mode(info, page_ecc, oob_ecc, ecc, stats);

			if (ret >= 0) {
				/* Certain patterns of data are consistent with multiple
				 * ECC schemes.  We perform some additional checks and
				 * attempt to determine a single ECC scheme if
				 * possible. */

				/* FLEX ECC is unambiguous. */
				if (ecc == STMNAND_ECC_FLEX) {
					valid_eccs = 0;
					memcpy(page, page_ecc, page_size);
					memcpy(oob, oob_ecc, oob_size);
					break;
				}

				/* Trust AFM and Boot Mode ECC tags, if present. */
				if ((ecc == STMNAND_ECC_AFM || ecc == STMNAND_ECC_BOOT) &&
					stats->tag_fail == 0) {
					valid_eccs = 0;
					memcpy(page, page_ecc, page_size);
					memcpy(oob, oob_ecc, oob_size);
					break;
				}

				/* Data is consistent with current ECC scheme. */
				valid_eccs |= (1 << ecc);
			}

			/* Continue search...*/
			stats->ecc = STMNAND_ECC_UNKNOWN;
		}

		/* Attempt to handle multiple ECC candidates */
		if (stats->ecc == STMNAND_ECC_UNKNOWN && valid_eccs) {
			n_valid = 0;
			for (i = 0; i < 32; i++)
			if (valid_eccs & (1 << i))
				n_valid++;

			if (n_valid == 1) {
				/* Only one candidate */
				ecc = ffs(valid_eccs) - 1;
				ret = nand_ecc_decode_mode(info, page, oob, ecc, stats);
			}
			else if (n_valid == 2 &&
				(valid_eccs & (1 << STMNAND_ECC_BOOT)) &&
				(valid_eccs & (1 << STMNAND_ECC_AFM))) {
				/* AFM and Boot ECC both valid, but AFM that also passes
				 * for Boot is *extrememly unlikely*, so assume Boot...
				 */
				ecc = STMNAND_ECC_BOOT;
				ret = nand_ecc_decode_mode(info, page, oob, ecc, stats);
			}
			else {
				/* Not possible to differentiate between multiple
				 * compatible ECC schemes.
				 */
				ret = -1;
			}
		}

		free(page_ecc);

		return ret;
	}


int nand_ecc_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
		    int ecc_mode, struct nand_ecc_stats *stats)
{
	if (ecc_mode == STMNAND_ECC_DETECT)
		return nand_ecc_decode_detect(info, page, oob, stats);
	else
		return nand_ecc_decode_mode(info, page, oob, ecc_mode, stats);
}
