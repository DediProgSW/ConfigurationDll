/*
 *  nand_io_mtd.h
 *
 *    'nand_io' interface, based on linux-MTD layer
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

#ifndef NAND_IO_MTD_H
#define NAND_IO_MTD_H

#define MAX_MTDDEV_LEN		12
#define MAX_DEBUGFS_LEN		1024

#define STLINUX_NAND_EMI	0
#define STLINUX_NAND_FLEX	1
#define STLINUX_NAND_AFM	2
#define STLINUX_NAND_BCH	3
extern char *stlinux_nand_drivers[];

#define REMAP_NONE		0
#define REMAP_BCH18		1
#define REMAP_BCH30		2

struct nand_io_mtd {
	char	mtddev[MAX_MTDDEV_LEN];
	char	debugfs_nanderasebb[MAX_DEBUGFS_LEN];
	int	fd;
	int	bch_remap;
	int	driver;
};

#endif /* NAND_IO_MTD_H */
