/*
 *  nand_ecc_hamming.c
 *
 *    Wrapper layer over Hamming ECC code
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

#include "hamming_ecc.h"

#define AFM_SECTOR		512
#define AFM_OOB_SECTOR		16
void nand_ecc_afm_encode(struct nand_info *info, uint8_t *page, uint8_t *oob)
{
	int i;

	for (i = 0; i < info->page_size / AFM_SECTOR; i++) {
		ecc_gen(page, oob, ECC_512);
		oob[3] = 'A';
		oob[4] = 'F';
		oob[5] = 'M';
		oob[6] = oob[2] & 0x3;

		page += AFM_SECTOR;
		oob += AFM_OOB_SECTOR;
	}
}

int nand_ecc_afm_check_tag(struct nand_info *info, uint8_t *oob,
			   int max_errors)
{
	int i;
	int errors = 0;

	for (i = 0; i < info->oob_size; i += AFM_OOB_SECTOR) {
		errors += bits_set[(uint8_t)(oob[i + 3] ^ 'A')];
		errors += bits_set[(uint8_t)(oob[i + 4] ^ 'F')];
		errors += bits_set[(uint8_t)(oob[i + 5] ^ 'M')];
	}

	return (errors <= max_errors) ? 0 : 1;
}

int nand_ecc_afm_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			struct nand_ecc_stats *stats)
{
	int page_size = info->page_size;
	uint8_t e1[3], e2[3];
	int n_errors = 0;
	int byte, bit;
	uint8_t *o, *p;
	int i;
	int ret;

	p = page;
	o = oob;

	for (i = 0; i < page_size / AFM_SECTOR; i++) {
		ecc_gen(p, e2, ECC_512);

		e1[0] = o[0];
		e1[1] = o[1];
		e1[2] = o[2] & 0xfc;
		e1[2] |= o[6] & 0x3;

		/* Handle special case of an erased page */
		if (i == 0 && e1[0] == 0xff && e1[1] == 0xff && e1[2] == 0xff &&
		    nand_is_page_empty(info, page, oob, 1))
			break;

		ret = ecc_correct(p, e1, e2, ECC_512, &byte, &bit);

		if (ret == E_UN_CHK) {
			n_errors = -1;
			break;
		}

		if (ret == E_D1_CHK || ret == E_C1_CHK) {
			if (stats) {
				if (ret == E_C1_CHK) {
					byte = page_size + (i * AFM_OOB_SECTOR);
					bit = 0;
				} else {
					byte += (i * AFM_SECTOR);
				}
				stats->bit_err_pos[n_errors++] =
					(byte * 8) + bit;
			} else {
				n_errors++;
			}

		}
		p += AFM_SECTOR;
		o += AFM_OOB_SECTOR;
	}

	if (stats) {
		stats->n_errors = n_errors;
		stats->tag_fail = nand_ecc_afm_check_tag(info, oob, 1);
	}

	return n_errors;

}

#define BOOT_SECTOR		128
#define BOOT_OOB_SECTOR		4
void nand_ecc_boot_encode(struct nand_info *info, uint8_t *page, uint8_t *oob)
{
	int i;

	for (i = 0; i < info->page_size / BOOT_SECTOR; i++) {
		ecc_gen(page, oob, ECC_128);
		oob[3] = 'B';

		page += BOOT_SECTOR;
		oob += BOOT_OOB_SECTOR;
	}
}

int nand_ecc_boot_check_tag(struct nand_info *info, uint8_t *oob,
			    int max_errors)
{
	int i;
	int errors = 0;

	for (i = 0; i < info->oob_size; i += BOOT_OOB_SECTOR)
		errors += bits_set[(uint8_t)(oob[i + 3] ^ 'B')];

	return (errors <= max_errors) ? 0 : 1;
}

int nand_ecc_boot_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			 struct nand_ecc_stats *stats)
{
	int page_size = info->page_size;
	uint8_t e1[3], e2[3];
	int byte, bit;
	int n_errors = 0;
	uint8_t *o, *p;
	int i;
	int ret;

	p = page;
	o = oob;

	for (i = 0; i < page_size / BOOT_SECTOR; i++) {
		ecc_gen(p, e2, ECC_128);

		e1[0] = o[0];
		e1[1] = o[1];
		e1[2] = o[2];

		/* Handle special case of an erased page */
		if (i == 0 && e1[0] == 0xff && e1[1] == 0xff && e1[2] == 0xff &&
		    nand_is_page_empty(info, page, oob, 1))
			break;

		ret = ecc_correct(p, e1, e2, ECC_128, &byte, &bit);
		if (ret == E_UN_CHK) {
			n_errors = -1;
			break;
		}

		if (ret == E_D1_CHK || ret == E_C1_CHK) {
			if (stats) {
				if (ret == E_C1_CHK) {
					byte = page_size +
						(i * BOOT_OOB_SECTOR);
					bit = 0;
				} else {
					byte += (i * BOOT_SECTOR);
				}
				stats->bit_err_pos[n_errors++] =
					(byte * 8) + bit;
			} else {
				n_errors++;
			}
		}
		p += BOOT_SECTOR;
		o += BOOT_OOB_SECTOR;
	}

	if (stats) {
		stats->n_errors = n_errors;
		stats->tag_fail = nand_ecc_boot_check_tag(info, oob, 1);
	}

	return n_errors;
}


#define FLEX_SECTOR 256
static uint8_t flex_ecclayout_sp[] = {0, 1, 2,
				      3, 6, 7};
static uint8_t flex_ecclayout_lp[] = {40, 41, 42,
				      43, 44, 45,
				      46, 47, 48,
				      49, 50, 51,
				      52, 53, 54,
				      55, 56, 57,
				      58, 59, 60,
				      61, 62, 63};
void nand_ecc_flex_encode(struct nand_info *info, uint8_t *page, uint8_t *oob)
{
	uint8_t ecc[3];
	uint8_t *ecclayout;

	int o;
	int i;

	ecclayout = (info->page_size == 512) ?
		flex_ecclayout_sp : flex_ecclayout_lp;

	o = 0;
	for (i = 0; i < info->page_size / FLEX_SECTOR; i++) {
		ecc_gen(page, ecc, ECC_256);

		/* Invert ECC and and swap LP bytes */
		oob[ecclayout[o++]] = ~ecc[1];
		oob[ecclayout[o++]] = ~ecc[0];
		oob[ecclayout[o++]] = ~ecc[2];

		page += FLEX_SECTOR;
	}
}


int nand_ecc_flex_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			 struct nand_ecc_stats *stats)
{
	int page_size = info->page_size;
	uint8_t e1[3], e2[3];
	uint8_t *ecclayout;
	int byte, bit;
	int n_errors = 0;
	int i;
	int o;
	int ret;

	ecclayout = (page_size == 512) ?
		flex_ecclayout_sp : flex_ecclayout_lp;
	o = 0;

	for (i = 0; i < page_size / FLEX_SECTOR; i++) {
		ecc_gen(page, e2, ECC_256);

		/* Invert ECC and and swap LP bytes */
		e1[1] = ~oob[ecclayout[o++]];
		e1[0] = ~oob[ecclayout[o++]];
		e1[2] = ~oob[ecclayout[o++]];

		byte = -1; bit = -1;
		ret = ecc_correct(page, e1, e2, ECC_256, &byte, &bit);
		if (ret == E_UN_CHK) {
			n_errors = -1;
			break;
		}

		if (ret == E_D1_CHK || ret == E_C1_CHK) {
			if (stats) {
				if (ret == E_C1_CHK) {
					byte = page_size + ecclayout[o-3];
					bit = 0;
				} else {
					byte += (i * FLEX_SECTOR);
				}
				stats->bit_err_pos[n_errors++] =
					(byte * 8) + bit;
			} else {
				n_errors++;
			}
		}
		page += FLEX_SECTOR;
	}

	if (stats) {
		stats->n_errors = n_errors;
		stats->tag_fail = -1;
	}

	return n_errors;
}
