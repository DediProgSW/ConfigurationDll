/*
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

#ifndef NAND_ECC_H
#define NAND_ECC_H

#include <stdint.h>
#include <nand.h>

/*
 * ECC Schemes employed by STM NAND drivers
 */
#define STMNAND_ECC_NONE	0
#define STMNAND_ECC_FLEX	1
#define STMNAND_ECC_BOOT	2
#define STMNAND_ECC_AFM		3
#define STMNAND_ECC_BCH18	4
#define STMNAND_ECC_BCH30	5

#define STMNAND_ECC_EMPTYPAGE	6
#define STMNAND_ECC_UNKNOWN	7
#define STMNAND_ECC_DETECT	8

struct nand_ecc_stats {
	int	ecc;
	int	n_errors;
	int	bit_err_pos[NAND_MAX_BIT_ERRORS];
	int	tag_fail;
};

extern char *nand_ecc_strs[];
int nand_str2eccmode(char *str);

void nand_ecc_dump_stats(struct nand_ecc_stats *stats);

int nand_ecc_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
		    int ecc_mode, struct nand_ecc_stats *stats);

int nand_ecc_encode(struct nand_info *info, uint8_t *page, uint8_t *oob,
		    int ecc_mode);

/*
 * BCH ECC specifics (required globally for 'Page-OOB' remapping)
 */
#define BCH_SECTOR_BYTES	1024
#define BCH_MODE_18BIT		0
#define BCH_MODE_30BIT		1
#define BCH18_ECC_BYTES		32
#define BCH30_ECC_BYTES		54
void nand_ecc_bch_remap(struct nand_info *info, uint8_t *page, uint8_t *oob,
			int bch_mode);

void nand_ecc_bch_unmap(struct nand_info *info, uint8_t *page, uint8_t *oob,
			int bch_mode);

#endif /* NAND_ECC_H */
