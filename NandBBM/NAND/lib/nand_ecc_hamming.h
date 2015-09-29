/*
 *  nand_ecc_hamming.c
 *
 *    Wrapper layer over Hamming ECC code
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

#ifndef NAND_ECC_HAMMING_H
#define NAND_ECC_HAMMING_H

#include <stdint.h>
#include <nand.h>

/*
 * Hamming ECC
 */
void nand_ecc_boot_encode(struct nand_info *info, uint8_t *page, uint8_t *oob);

int nand_ecc_boot_check_tag(struct nand_info *info, uint8_t *oob,
			    int max_errors);

int nand_ecc_boot_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			 struct nand_ecc_stats *stats);

void nand_ecc_afm_encode(struct nand_info *info, uint8_t *page, uint8_t *oob);

int nand_ecc_afm_check_tag(struct nand_info *info, uint8_t *oob,
			   int max_errors);

int nand_ecc_afm_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			struct nand_ecc_stats *stats);

void nand_ecc_flex_encode(struct nand_info *info, uint8_t *page, uint8_t *oob);

int nand_ecc_flex_decode(struct nand_info *info, uint8_t *page, uint8_t *oob,
			 struct nand_ecc_stats *stats);


#endif /* NAND_ECC_HAMMING_H */
