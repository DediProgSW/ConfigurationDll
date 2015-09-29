/*
 *  nand_io_ecc.c
 *
 *    Add ECC support to raw 'nand_io' layer
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
#include <nand_ecc.h>
#include <nand_io.h>

int nandio_write_page_ecc(void *iohandle, struct nand_info *info,
			  uint8_t *page, uint8_t *oob, uint64_t offs,
			  int ecc_mode)
{
	uint8_t *oob_buf, *o;
	int ret;

	if (!oob && ecc_mode != STMNAND_ECC_NONE) {
		oob_buf = xxmalloc(info->oob_size);
		memset(oob_buf, 0xff, info->oob_size);
		o = oob_buf;
	} else {
		o = oob;
	}

	ret = nand_ecc_encode(info, page, o, ecc_mode);
	if (ret != 0) {
		goto out1;
	}

	ret = nandio_write_page_raw(iohandle, info, page, o, offs);

 out1:
	if (!oob && ecc_mode != STMNAND_ECC_NONE)
		free(oob_buf);

	return ret;
}

int nandio_read_page_ecc(void *iohandle, struct nand_info *info,
			 uint8_t *page, uint8_t *oob, uint64_t offs,
			 int ecc_mode, struct nand_ecc_stats *stats)
{

	uint8_t *oob_buf, *o;
	int ret = 0;

	if (!oob) {
		oob_buf = xxmalloc(info->oob_size);
		o = oob_buf;
	} else {
		o = oob;
	}

	if (nandio_read_page_raw(iohandle, info, page, o, offs) != 0)  {
		ret = 1;
		goto out1;
	}

	if (nand_ecc_decode(info, page, o, ecc_mode, stats) < 0) {
		ret = 1;
		goto out1;
	}

 out1:
	if (!oob)
		free(oob_buf);

	return ret;
}
