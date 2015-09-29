/*
 *  nand_io_bbt.h
 *
 *    Support for reading/writing NAND BBTs (based on 'nand_io' interface)
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

#ifndef NAND_BBT_IO_H
#define NAND_BBT_IO_H

int nandio_write_bbm(void *iohandle, struct nand_info *info,
		     uint8_t *page, uint8_t *oob, uint64_t offs);

int nandio_bbt_scan_block(void *iohandle, struct nand_info *info,
			  uint64_t offs, int ecc_mode, int bbt_type,
			  struct nand_bbt_stats *bbt_stats);

uint8_t *nandio_bbt_load(void *iohandle, struct nand_info *info,
			 int ecc_mode, int bbt_type,
			 struct nand_bbt_stats *bbt_stats);

/*
 * Scan device for bad blocks
 */
#define NAND_BBT_SCAN_BBM	0x1
#define NAND_BBT_SCAN_TAG	0x3
#define NAND_BBT_SCAN_ECC	0x7

extern char *nand_bbt_scan_strs[];
int nand_str2bbtscanmode(char *str);

int nandio_bbt_scan_bbms_block(void *iohandle, struct nand_info *info,
			       uint64_t offs, uint8_t *page, uint8_t *oob,
			       int mode);

uint8_t *nandio_bbt_scan_bbms(void *iohandle, struct nand_info *info,
			      int mode, int show_progress);

#define NAND_BBT_UPDATE_PRIMARY		0x1
#define NAND_BBT_UPDATE_MIRROR		0x2
#define NAND_BBT_UPDATE_BOTH		0x3
int nandio_bbt_update(void *iohandle, struct nand_info *info, uint8_t *bbt,
		      int ecc_mode, int bbt_type, uint8_t vers, uint32_t update,
		      uint64_t bbt_pair_offs[2], uint8_t bbt_pair_vers[2]);

int nandio_bbt_sync_pair(void *iohandle, struct nand_info *info, uint8_t *bbt,
			 struct nand_bbt_stats *bbt_stats,
			 uint64_t bbt_pair_offs[2], uint8_t bbt_pair_vers[2]);

#endif /* NAND_BBT_IO_H */
