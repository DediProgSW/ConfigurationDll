/*
 *  nand_ecc_bch.c
 *
 *    Wrapper layer over BCH ECC code
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

#include "nand_ecc_bch.h"
#include "bch_encoder.h"
#include "bch_decoder.h"

static void data_expand_to_bit(uint8_t *in, uint8_t *out, int len)
{
	int i, j, k;
	uint8_t byte;

	for (i = 0, k = 0; i < len; i++) {
		byte = in[i];
		for (j = 0; j < 8; j++, k++) {
			out[k] = byte & 0x1;
			byte >>= 1;
		}
	}
}

static void data_contract_to_byte(uint8_t *in, uint8_t *out, int len)
{
	int i, j, k;
	uint8_t byte;

	for (i = 0, k = 0; i < len; i++) {
		byte = 0;
		for (j = 0; j < 8; j++, k++) {
			byte |= (in[k] & 0x1) << j;
		}
		out[i] = byte;
	}
}

#define BCH_SECTOR_BITS		(BCH_SECTOR_BYTES*8)
#define BCH18_OUT_BITS		8444
//#define BCH30_OUT_BITS		8612
#define BCH30_OUT_BITS		8612

void nand_ecc_bch_encode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			 int bch_mode)
{
	int page_size = info->page_size;
	int oob_size = info->oob_size;
	int bch_len_in = BCH_SECTOR_BITS;
	int bch_len_out = (bch_mode == BCH_MODE_18BIT) ? BCH18_OUT_BITS :
		BCH30_OUT_BITS;
	int bch_len_out_aligned = (bch_len_out + 16) & ~15;
	int sectors = page_size/BCH_SECTOR_BYTES;

	uint8_t *sector_in;
	uint8_t *sector_out;

	uint8_t *src, *dst;
	uint8_t *data_out;

	int s;

	sector_in = (uint8_t *)xxmalloc(bch_len_in);
	sector_out = (uint8_t *)xxmalloc(bch_len_out_aligned);

	data_out = (uint8_t *)xxmalloc(page_size + oob_size);
	memset(data_out, 0xff, page_size + oob_size);

	src = page;
	dst = data_out;

	for (s = 0; s < sectors; s++) {
		data_expand_to_bit(src, sector_in, BCH_SECTOR_BYTES);
		memset(sector_out, 0x00, bch_len_out_aligned);
		bch_encoder((char *)sector_out, (char *)sector_in, bch_mode,
			    bch_len_in, bch_len_out);
		data_contract_to_byte(sector_out, dst, bch_len_out_aligned/8);

		src += BCH_SECTOR_BYTES;
		dst += bch_len_out_aligned/8;
	}

	memcpy(page, data_out, page_size);
	memcpy(oob, data_out + page_size, oob_size);

	free(sector_in);
	free(sector_out);
	free(data_out);
}

static int bch_check_erased_page(uint8_t *data, int page_size, int max_zeros)
{
	int i;
	uint8_t *b = data;
	int zeros = 0;

	for (i = 0; i < page_size; i++) {
		zeros += bits_set[(uint8_t)~*b++];
		if (zeros > max_zeros)
				return -1;
	}

	if (zeros)
		memset(data, 0xff, page_size);

	return zeros;
}

int nand_ecc_bch_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			int bch_mode, struct nand_ecc_stats *stats)
{
	int page_size = info->page_size;
	int oob_size = info->oob_size;
	int bch_len_in = BCH_SECTOR_BITS;
	int bch_len_out = (bch_mode == BCH_MODE_18BIT) ? BCH18_OUT_BITS :
		BCH30_OUT_BITS;
	int bch_len_out_aligned = (bch_len_out + 16) & ~15;
	int sectors = page_size/BCH_SECTOR_BYTES;

	uint8_t *sector_err;
	uint8_t *sector_chien;
	uint8_t *sector_fix;

	int bit_err_pos[30];
	int stat_decoder[3];

	uint8_t *src, *dst;
	uint8_t *data_in;

	int n_errors = 0;
	int uncorrectable = 0;

	int i;
	int s;

	sector_err = (uint8_t *)xxmalloc(bch_len_out_aligned);
	sector_fix = (uint8_t *)xxmalloc(bch_len_out_aligned);
	sector_chien = (uint8_t *)xxmalloc(bch_len_out_aligned);

	data_in = (uint8_t *)xxmalloc(page_size + oob_size);
	memcpy(data_in, page, page_size);
	memcpy(data_in + page_size, oob, oob_size);

	src = data_in;
	dst = page;

	for (s = 0; s < sectors; s++) {
		data_expand_to_bit(src, sector_err, bch_len_out_aligned/8);

		memset(sector_fix, 0x00, bch_len_out_aligned);
		memset(sector_chien, 0x00, bch_len_out_aligned);

		bch_decoder(bit_err_pos, (char *)sector_fix,
			    (char *)sector_chien, (char *)sector_err,
			    bch_len_out, bch_len_in, stat_decoder);

		if (stat_decoder[2]) {
			uncorrectable = 1;
			break;
		}

		data_contract_to_byte(sector_fix, dst, BCH_SECTOR_BYTES);
		if (stats)
			for (i = 0; i < stat_decoder[0]; i++)
				stats->bit_err_pos[n_errors++] =
					bit_err_pos[i] +
					(s * bch_len_out_aligned);
		else
			n_errors += stat_decoder[0];

		src += bch_len_out_aligned/8;
		dst += BCH_SECTOR_BYTES;
	}


	if (uncorrectable) {
		src = data_in;
		dst = page;

		for (s = 0; s < sectors; s++) {
			memcpy(dst, src, BCH_SECTOR_BYTES);
			src += bch_len_out_aligned/8;
			dst += BCH_SECTOR_BYTES;
		}

		n_errors = bch_check_erased_page(page, page_size, sectors);
		if (n_errors > 0 && stats) {
			/* Bit error positions not available, to so set to '0' */
			for (i = 0; i < n_errors; i++)
				stats->bit_err_pos[i] = 0;
		}
	}

	if (stats) {
		stats->n_errors = n_errors;
		stats->tag_fail = -1;
	}

	memset(oob, 0xff, oob_size);

	free(sector_err);
	free(sector_fix);
	free(sector_chien);
	free(data_in);

	return n_errors;
}

void nand_ecc_bch_remap(struct nand_info *info, uint8_t *page, uint8_t *oob,
			int bch_mode)
{
	int page_size = info->page_size;
	int oob_size = info->oob_size;
	int n_sectors, s;
	int ecc_bytes_per_sector;
	int oob_bytes_remainder;
	uint8_t *data;
	uint8_t *dst, *src_p, *src_o;

	n_sectors = page_size / BCH_SECTOR_BYTES;
	ecc_bytes_per_sector =  (bch_mode == BCH_MODE_18BIT) ? BCH18_ECC_BYTES :
		BCH30_ECC_BYTES;
	oob_bytes_remainder = oob_size - (n_sectors * ecc_bytes_per_sector);

	data = (uint8_t *)xxmalloc(page_size * oob_size);
	memset(data, 0xff, page_size + oob_size);

	src_p = page;
	src_o = oob;
	dst = data;

	for (s = 0; s < n_sectors; s++) {
		memcpy(dst, src_p, BCH_SECTOR_BYTES);
		dst += BCH_SECTOR_BYTES;
		src_p += BCH_SECTOR_BYTES;

		memcpy(dst, src_o, ecc_bytes_per_sector);
		dst += ecc_bytes_per_sector;
		src_o += ecc_bytes_per_sector;
	}

	if (oob_bytes_remainder)
		memcpy(dst, src_o, oob_bytes_remainder);

	memcpy(page, data, page_size);
	memcpy(oob, data + page_size, oob_size);

	free(data);
}

void nand_ecc_bch_unmap(struct nand_info *info, uint8_t *page, uint8_t *oob,
			int bch_mode)
{
	int page_size = info->page_size;
	int oob_size = info->oob_size;
	int n_sectors, s;
	int ecc_bytes_per_sector;
	int oob_bytes_remainder;
	uint8_t *data;
	uint8_t *dst_p, *dst_o, *src;

	n_sectors = page_size / BCH_SECTOR_BYTES;
	ecc_bytes_per_sector =  (bch_mode == BCH_MODE_18BIT) ? BCH18_ECC_BYTES :
		BCH30_ECC_BYTES;
	oob_bytes_remainder = oob_size - (n_sectors * ecc_bytes_per_sector);

	data = (uint8_t *)xxmalloc(page_size * oob_size);
	memcpy(data, page, page_size);
	memcpy(data + page_size, oob, oob_size);

	memset(oob, 0xff, oob_size);
	src = data;
	dst_p = page;
	dst_o = oob;

	for (s = 0; s < n_sectors; s++) {
		memcpy(dst_p, src, BCH_SECTOR_BYTES);
		src += BCH_SECTOR_BYTES;
		dst_p += BCH_SECTOR_BYTES;

		memcpy(dst_o, src, ecc_bytes_per_sector);
		src += ecc_bytes_per_sector;
		dst_o += ecc_bytes_per_sector;
	}

	if (oob_bytes_remainder)
		memcpy(dst_o, src, oob_bytes_remainder);

	free(data);
}
