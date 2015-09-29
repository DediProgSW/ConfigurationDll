/*
 *  nand.h
 *
 *    Generic NAND support
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

#ifndef NAND_H
#define NAND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define NAND_MAX_PAGE_SIZE	16384
#define NAND_MAX_OOB_SIZE	512
#define NAND_MAX_BIT_ERRORS	(30*(NAND_MAX_PAGE_SIZE/1024))

struct nand_info {
	uint64_t	size;
	uint32_t	page_size;
	uint32_t	oob_size;
	uint32_t	block_size;
	int		block_count;
	int		block_shift;
};

/*
 * General NAND functions (nand.c)
 */
int nand_is_page_empty(struct nand_info *info, uint8_t *page, uint8_t *oob,
		       int max_bit_errors);

void nand_dump_page(struct nand_info *info, uint8_t *page, uint8_t *oob,
		    uint64_t offs);

void nand_dump_info(struct nand_info *info);

int nand_is_page_zeros(struct nand_info *info, uint8_t *page, uint8_t *oob,
		       int max_bit_error);

/*
 * Miscillaneous functions, useful in NAND-related tools (misc.c)
 */
int ffs64(uint64_t x);
extern const uint8_t bits_set[];
extern const uint8_t byte_parity_table[];

int str2xxx(char *str, char *strs[], int n_strs);
#define MAKE_str2xxx(xxx, strs)					\
	int str2##xxx(char *str)				\
	{							\
		return str2xxx(str, strs, ARRAY_SIZE(strs));	\
	}

int confirm_operation(void);
int read_data(char *fn_dat, uint8_t *data, uint32_t size);
int write_data(char *fn_dat, uint8_t *data, int size);

void inject_errors(uint8_t *data, int size, int n_errors, int **err_pos);
int detect_errors(uint8_t *data1, uint8_t *data2, int len, int **err_pos);

#ifndef MIN	/* some C lib headers define this for us */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define min(a, b) MIN(a, b) /* glue for linux kernel source */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void *xxmalloc(size_t size);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* NAND_H */
