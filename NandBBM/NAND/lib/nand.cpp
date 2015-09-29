/*
 *  nand.c
 *
 *    Generic NAND support
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

#include <nand.h>
#include <xprintf.h>

void nand_dump_info(struct nand_info *info)
{
	printf("NAND Device geometry:\n");
	printf("\tsize         : 0x%012" PRIx64 " [%uMiB]\n",
	       info->size, (unsigned int)(info->size >> 20));
	printf("\tpage_size    : 0x%04x [%u]\n",
	       (unsigned int)info->page_size, (unsigned int)info->page_size);
	printf("\toob_size     : 0x%04x [%u]\n",
	       (unsigned int)info->oob_size, (unsigned int)info->oob_size);
	printf("\tblock_size   : 0x%08x [%uKiB]\n",
	       (unsigned int)info->block_size,
	       (unsigned int)info->block_size >> 10);
	printf("\n");
}

void nand_dump_page(struct nand_info *info, uint8_t *data, uint8_t *oob,
		    uint64_t offs)
{
	int i;

	for (i = 0; i < info->page_size; i++) {
		if (i % 16 == 0)
			printf("0x%012" PRIx64 ": ", offs + i);
		printf("%02x ", data[i]);

		if ((i + 1) % 16 == 0)
			printf("\n");
	}

	if (oob) {
		for (i = 0; i < info->oob_size; i++) {
			if (i % 16 == 0)
				printf("     OOB 0x%03x: ", i);

			printf("%02x ", oob[i]);

			if ((i + 1) % 16 == 0)
				printf("\n");
		}
	}
}

int nand_is_page_empty(struct nand_info *info, uint8_t *page, uint8_t *oob,
		       int max_bit_errors)
{
	int i;
	int e = 0;

	for (i = 0; i < info->page_size; i++) {
		e += bits_set[(uint8_t)~*page++];
		if (e > max_bit_errors)
			return 0;
	}

	if (!oob || info->oob_size == 0)
		return 1;

	for (i = 0; i < info->oob_size; i++) {
		e += bits_set[(uint8_t)~*oob++];
		if (e > max_bit_errors)
			return 0;
	}

	return 1;
}

int nand_is_page_zeros(struct nand_info *info, uint8_t *page, uint8_t *oob,
		       int max_bit_errors)
{
	int i;
	int e = 0;

	for (i = 0; i < info->page_size; i++) {
		e += bits_set[(uint8_t)*page++];
		if (e > max_bit_errors)
			return 0;
	}

	if (!oob || info->oob_size == 0)
		return 1;

	for (i = 0; i < info->oob_size; i++) {
		e += bits_set[(uint8_t)*oob++];
		if (e > max_bit_errors)
			return 0;
	}

	return 1;
}
