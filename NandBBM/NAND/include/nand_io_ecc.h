/*
 *  nand_io_ecc.h
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

#ifndef NAND_IO_ECC_H
#define NAND_IO_ECC_H

#include <stdint.h>
#include <nand_ecc.h>

int nandio_write_page_ecc(void *nandio, struct nand_info *info,
			  uint8_t *page, uint8_t *oob, uint64_t offs,
			  int ecc_mode);

int nandio_read_page_ecc(void *iohandle, struct nand_info *info,
			 uint8_t *page, uint8_t *oob, uint64_t offs,
			 int ecc_mode, struct nand_ecc_stats *stats);

#endif /* NAND_IO_ECC_H */
