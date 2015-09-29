/*
 *  nand_bbt.c
 *
 *     Support for NAND BBTs (linux and 'inband')
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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <xprintf.h>
#include <nand.h>
#include <nand_bbt.h>
#include <nand_ecc.h>

char* nand_bbt_strs[] = {
	"nobbt",
	"linux",
	"inband",
	"detect"
};

int nand_str2bbttype(char *str)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nand_bbt_strs); i++)
		if (nand_bbt_strs[i] && strcmp(str, nand_bbt_strs[i]) == 0)
			return i;

	return -1;
}

char *nand_bbt_bak_strs[] = {
	"primary",
	"mirror",
};

void nand_bbt_dump_stats(struct nand_bbt_stats *stats)
{
	printf("\t\ttype    : %s\n", (stats->bbt == STMNAND_BBT_LINUX) ?
	       "Linux BBT" : "Inband(BCH) BBT");
	printf("\t\toffs    : 0x%012" PRIx64 "\n", stats->offs);
	printf("\t\tecc     : %s\n", nand_ecc_strs[stats->ecc]);
	printf("\t\tbackup  : %s\n", nand_bbt_bak_strs[stats->backup]);
	printf("\t\tversion : %u\n", stats->version);
	if (stats->bbt == STMNAND_BBT_INBAND)
		printf("\t\tauthor  : %s\n", stats->ibbt_author);

}


uint8_t nand_bbt_sigs[2][NAND_BBT_SIGLEN] = {
	{'B', 'b', 't', '0'},
	{'1', 't', 'b', 'B'},
};

void nand_bbt_set_mark(uint8_t *bbt, uint32_t block, uint8_t mark)
{
	unsigned int byte = block >> 2;
	unsigned int shift = (block & 0x3) << 1;

	bbt[byte] &= ~(0x3 << shift);
	bbt[byte] |= ((mark & 0x3) << shift);
}

uint8_t nand_bbt_get_mark(uint8_t *bbt, uint32_t block)
{
	unsigned int byte = block >> 2;
	unsigned int shift = (block & 0x3) << 1;

	return (bbt[byte] >> shift) & 0x3;
}

int nand_bbt_isbad(uint8_t *bbt, uint32_t block)
{
	return nand_bbt_get_mark(bbt, block) == BBT_MARK_GOOD ? 0 : 1;
}

int nand_bbt_countbad(struct nand_info *info, uint8_t *bbt)
{
	int i;
	int bad = 0;

	for (i = 0; i < info->block_count; i++)
		if (nand_bbt_isbad(bbt, i))
			bad++;

	return bad;
}

void nand_bbt_dump(struct nand_info *info, uint8_t *bbt, char *tag)
{
	int block_count = info->block_count;
	int block_shift = info->block_shift;
	int bad_count = 0;
	uint8_t mark;
	int i;

	printf("%s:\n", tag ? tag : "BBT");
	for (i = 0; i < block_count; i++) {
		mark = nand_bbt_get_mark(bbt, i);
		if (mark != BBT_MARK_GOOD) {
			printf("\t0x%012" PRIx64 " [%04d] : Bad (%s)\n",
			       ((uint64_t)i) << block_shift, i,
			       mark == BBT_MARK_BAD_WEAR ? "Wear" : "Factory");
			bad_count++;
		}
	}
	if (bad_count == 0)
		printf("\tNo bad blocks listed in table\n");
	printf("\n");

}
