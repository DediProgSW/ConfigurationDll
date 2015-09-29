/*
 *  nand_bbt_inband.c
 *
 *    Support for 'inband' NAND BBTs
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xprintf.h>
#include <nand.h>
#include <nand_bbt.h>

#include "nand_ecc_bch.h"

#define NAND_IBBT_SIGLEN		4
#define NAND_IBBT_SCHEMA		0x10
#define NAND_IBBT_BCH_SCHEMA		0x10

/* IBBT header */
#pragma pack ()//，取消自定义字节对齐方式。
struct nand_ibbt_header
{
	uint8_t	signature[4];		/* "Bbt0" or "1tbB" signature */
	uint8_t version;		/* BBT version ("age") */
	uint8_t reserved[3];		/* padding */
	uint8_t schema[4];		/* "base" schema (x4) */
};// __attribute__((__packed__));

/* Extend IBBT header with some stm-nand-bch niceties */
#pragma pack ()//，取消自定义字节对齐方式。
struct nand_ibbt_bch_header
{
	struct nand_ibbt_header base;
	uint8_t schema[4];		/* "private" schema (x4) */
	uint8_t ecc_size[4];		/* ECC bytes (0, 32, 54) per sector (x4) */
	char	author[64];		/* Arbitrary string for S/W to use */
};// __attribute__((__packed__));


int nand_bbt_inband_scan_sig(uint8_t *page, int *backup, uint8_t *vers,
			     uint8_t *ecc_size, char author[64])
{
	struct nand_ibbt_bch_header ibbt_hdr;
	int b, i;
	int match_sig;

	memcpy((uint8_t *)&ibbt_hdr, page, sizeof(ibbt_hdr));

	/* Get IBBT signature */
	for (b = 0; b < 2; b++) {
		match_sig = 1;
		for (i = 0; i < NAND_BBT_SIGLEN; i++) {
			if (ibbt_hdr.base.signature[i] !=
			    nand_bbt_sigs[b][i]) {
				match_sig = 0;
				break;
			}
		}
		if (match_sig)
			break;
	}

	/* Test IBBT signature */
	if (!match_sig)
		return 0;

	/* Test IBBT schema */
	for (i = 0; i < 4; i++)
		if (ibbt_hdr.base.schema[i] != NAND_IBBT_SCHEMA)
			return 0;

	/* Test IBBT BCH schema */
	for (i = 0; i < 4; i++)
		if (ibbt_hdr.schema[i] != NAND_IBBT_BCH_SCHEMA)
			return 0;

	/* We have a match */
	*backup = b;
	*vers = ibbt_hdr.base.version;
	*ecc_size = ibbt_hdr.ecc_size[0];
	strncpy(author, ibbt_hdr.author, 64);

	return 1;

}

void nand_bbt_inband_fill_sig(uint8_t *page, int bak, uint8_t vers,
			      uint8_t ecc_size)
{
	const char author[] = "STM NAND Tools (" VERSION ")";
	struct nand_ibbt_bch_header ibbt_hdr;

	memset(&ibbt_hdr, 0x00, sizeof(ibbt_hdr));

	memcpy(ibbt_hdr.base.signature, nand_bbt_sigs[bak], NAND_BBT_SIGLEN);
	ibbt_hdr.base.version = vers;
	memset(ibbt_hdr.base.schema, NAND_IBBT_SCHEMA, 4);

	memset(ibbt_hdr.schema, NAND_IBBT_BCH_SCHEMA, 4);
	memset(ibbt_hdr.ecc_size, ecc_size, 4);
	memcpy(ibbt_hdr.author, author, sizeof(author));

	memcpy(page, &ibbt_hdr, sizeof(ibbt_hdr));
}
