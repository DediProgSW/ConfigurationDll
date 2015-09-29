/*
 *  nand_driver.h
 *
 *    'nand_driver' interface
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

#ifndef NAND_DRIVER_H
#define NAND_DRIVER_H

#include <stdint.h>
#include <nand.h>
#include <nand_io.h>

struct nand_driver {
	struct nand_info	info;
	void			*iohandle;
	void			*priv;
};

struct nand_driver *nand_driver_init(void *config);

void nand_driver_exit(struct nand_driver *driver);

int nand_read_page(struct nand_driver *driver, uint8_t *page, uint64_t offs);

int nand_write_page(struct nand_driver *driver, uint8_t *page, uint64_t offs);

int nand_erase_block(struct nand_driver *driver, uint64_t offs);

int nand_is_block_bad(struct nand_driver *driver, uint64_t offs);

int nand_mark_block_bad(struct nand_driver *driver, uint64_t offs);

/*
 * Configuration data for 'nand_driver_raw' implementation
 */
#define NAND_DRIVER_BBT_FLASH		0x00000001
#define NAND_DRIVER_BBT_FLASH_CREATE	0x00000002

struct nand_driver_raw_config {
	void	*ioconfig;
	int	ecc_mode;
	int	bbt_options;
	int	scan_mode;
};

/*
 * Configuration data for 'nand_driver_linux' implementation
 */
struct nand_driver_linux_config {
	struct nand_io_mtd_config *ioconfig;
	int	ecc_mode;
};

#endif /* NAND_DRIVER_H */
