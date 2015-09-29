/*
 *  nand_bbt.h
 *
 *    Support for NAND BBTs
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

#ifndef NAND_BBT_H
#define NAND_BBT_H

#include <nand.h>

#define STMNAND_BBT_NONE		0
#define STMNAND_BBT_LINUX		1
#define STMNAND_BBT_INBAND		2

#define STMNAND_BBT_DETECT		3

struct nand_bbt_stats {
	uint64_t	offs;
	int		bbt;
	int		ecc;
	int		backup;
	uint8_t		version;
	char		ibbt_author[64];
};

#define BBT_MARK_BAD_FACTORY		0x0
#define BBT_MARK_BAD_WEAR		0x1
#define BBT_MARK_GOOD			0x3

extern char *nand_bbt_strs[];
int nand_str2bbttype(char *str);

void nand_bbt_dump_stats(struct nand_bbt_stats *stats);

void nand_bbt_set_mark(uint8_t *bbt, uint32_t block, uint8_t mark);
uint8_t nand_bbt_get_mark(uint8_t *bbt, uint32_t block);
int nand_bbt_isbad(uint8_t *bbt, uint32_t block);
int nand_bbt_countbad(struct nand_info *info, uint8_t *bbt);
void nand_bbt_dump(struct nand_info *info, uint8_t *bbt, char *tag);

#define NAND_BBT_NBLOCKS		4

#define NAND_BBT_NONE			0
#define NAND_BBT_LINUX			1
#define NAND_BBT_BCH			2

#define NAND_BBT_PRIMARY		0
#define NAND_BBT_MIRROR			1
#define NAND_BBT_SIGLEN			4
extern uint8_t nand_bbt_sigs[2][NAND_BBT_SIGLEN];

extern char *nand_bbt_bak_strs[];

int nand_bbt_linux_scan_sig(uint8_t *oob, int *backup, uint8_t *vers);

void nand_bbt_linux_fill_sig(uint8_t *oob, int bak, uint8_t vers);

int nand_bbt_inband_scan_sig(uint8_t *page, int *backup, uint8_t *vers,
			     uint8_t *ecc_size, char author[64]);

void nand_bbt_inband_fill_sig(uint8_t *page, int bak, uint8_t vers,
			      uint8_t ecc_size);


#endif /* NAND_BBT_H */
