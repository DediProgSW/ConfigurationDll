/*
 *  nand_bbt_linux.c
 *
 *    Support for 'linux' NAND BBTs
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
#include <sys/stat.h>

#include <xprintf.h>
#include <nand.h>
#include <nand_bbt.h>

#define NAND_BBT_LINUX_SOFFS	8
#define NAND_BBT_LINUX_VOFFS	12

int nand_bbt_linux_scan_sig(uint8_t *oob, int *backup, uint8_t *vers)
{
	int i, j;
	int found;

	for (i = 0; i < 2; i++) {
		found = 1;
		for (j = 0; j < NAND_BBT_SIGLEN; j++) {
			if (oob[NAND_BBT_LINUX_SOFFS + j] !=
			    nand_bbt_sigs[i][j]) {
				found = 0;
				break;
			}
		}
		if (found) {
			*backup = i;
			*vers = oob[NAND_BBT_LINUX_VOFFS];
			return 1;
		}
	}

	return 0;
}

void nand_bbt_linux_fill_sig(uint8_t *oob, int bak, uint8_t vers)
{
	memcpy(oob + NAND_BBT_LINUX_SOFFS, nand_bbt_sigs[bak], NAND_BBT_SIGLEN);
	oob[NAND_BBT_LINUX_VOFFS] = vers;
}
