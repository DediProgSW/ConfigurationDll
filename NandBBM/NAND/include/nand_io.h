/*
 * nand_io.h
 *
 *    'nand_io' interface
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

#ifndef NAND_IO_H
#define NAND_IO_H

#include <nand.h>

void *nand_io_init(void *config, struct nand_info *info);
void nand_io_exit(void *iohandle);

int nandio_read_page_raw(void *iohandle, struct nand_info *info,
			    uint8_t *page, uint8_t *oob, uint64_t offs);

int nandio_write_page_raw(void *iohandle, struct nand_info *info,
			     uint8_t *page, uint8_t *oob, uint64_t offs);

int nandio_erase_block(void *iohandle, struct nand_info *info, uint64_t offs);

/*
 * Configuration data for 'nand_io_mtd' implementation
 */

#define NAND_IO_MTD_ENFORCE_SPANS_MASTER	0x00000001
#define NAND_IO_MTD_BYPASS_BAD_PROTECT		0x00000002
struct nand_io_mtd_config {
	char		*mtddev;
	char		*debugfs;
	uint32_t	options;
};

#endif /* NAND_IO_H */
