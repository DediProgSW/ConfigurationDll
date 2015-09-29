/*
 *  stmnand_bbt_author.c
 *
 *    Generate BBT data from file listing bad blocks [Host]
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

#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"
#include <sys/stat.h>
#include <errno.h>

#define NAME		"stmnand_bbt_author"
#include <xprintf.h>
#include <stmnand.h>

#define MAX_FN_LEN	1024
#define MAX_LINE_LEN	64

static int verbose = 0;

static void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tGenerate BBT data from list of bad blocks using the\n");
	printf("\tspecified format and ECC mode.\n");
	printf("Usage: %s [OPTIONS] <fn_bad_list> <fn_bbt_base>\n", NAME);
	printf("Options:\n");
	printf("\t-p | --page-size        Page Size                       Required\n");
	printf("\t-o | --oob-size         OOB Size                        Required\n");
	printf("\t-b | --block-size       Block Size                      BCH-ONLY\n");
	printf("\t-c | --block-count      Device block count              Required\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        Required\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t-V | --bbt_vers=<vers>  BBT version                     [1]\n");
	printf("\t-n | --bch-no-remap     Do not perform BCH ECC remapping\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

int main_bbt_author(int argc, char *argv[])
{
	const char *short_options = "p:o:b:e:c:V:nvh";
	const struct option long_options[] = {
		{"page-size", 1, 0, 'p'},
		{"oob-size", 1, 0, 'o'},
		{"block-size", 1, 0, 'b'},
		{"ecc-mode", 1, 0, 'e'},
		{"block-count", 1, 0, 'c'},
		{"bbt-vers", 1, 0, 'V'},
		{"bch-no-remap",  0, 0, 'n'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	char *endptr;
	int option_index;
	int c;

	char *fn_bad_list = NULL;
	char *fn_bbt_base = NULL;
	char fn_out[MAX_FN_LEN];
	struct nand_info info;

	info.page_size = -1;
	info.oob_size = -1;
	info.block_size = -1;

	int ecc_mode = -1;
	int bbt_type = -1;
	int block_count = -1;
	uint8_t bbt_vers = 1;
	int bch_no_remap = 0;

	char line[MAX_LINE_LEN];
	int bbt_size;
	uint8_t *bbt, *b;
	int n_pages, buf_size;
	uint8_t *buf, *page, *oob;

	uint32_t chunk_size, total_size;
	int first_page;

	FILE *fp;
	int block;
	int bak;

	int ret = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'p':
			info.page_size = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'page-size' argument[%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'o':
			info.oob_size = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'oob-size' [%s] argument\n",
					optarg);
				return 1;
			}
			break;
		case 'b':
			info.block_size = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'block-size' [%s] argument\n",
					optarg);
				return 1;
			}
			break;
		case 'e':
			ecc_mode = nand_str2eccmode(optarg);
			if (ecc_mode == -1 || ecc_mode == STMNAND_ECC_DETECT) {
				eprintf("invalid 'ecc-mode' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'c':
			block_count = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0' || (block_count & 0x3)) {
				eprintf("invalid 'block-count' [%s] argument\n",
					optarg);
				return 1;
			}
			break;
		case 'V':
			bbt_vers = atoi(optarg);
			break;
		case 'n':
			bch_no_remap = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			print_usage();
			return 1;
		}
	}

	if (info.page_size == -1) {
		eprintf("Required argument missing [page-size]\n");
		return 1;
	}
	if (info.oob_size == -1) {
		eprintf("Required argument missing [oob-size]\n");
		return 1;
	}
	if (ecc_mode == -1) {
		eprintf("Required argument missing [ecc-mode]\n");
		return 1;
	}
	if (block_count == -1) {
		eprintf("Required argument missing [block-count]\n");
		return 1;
	}

	if ((argc - optind) < 2) {
		eprintf("Required argument missing [<fn_bad_list> and/or <fn_bbt_base>\n");
		return 1;
	}

	if (ecc_mode == STMNAND_ECC_AFM || ecc_mode == STMNAND_ECC_FLEX)
		bbt_type = STMNAND_BBT_LINUX;
	else
		bbt_type = STMNAND_BBT_INBAND;

	if (bbt_type == STMNAND_BBT_INBAND && info.block_size == -1) {
		eprintf("Required argument missing [block-size]\n");
		return 1;
	}

	fn_bad_list = argv[optind];
	fn_bbt_base = argv[optind + 1];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tfn_bad_list : %s\n", fn_bad_list);
		printf("\tfn_bbt_base : %s\n", fn_bbt_base);
		printf("\tpage-size   : %d\n", info.page_size);
		printf("\toob-size    : %d\n", info.oob_size);
		if (bbt_type == STMNAND_BBT_INBAND)
			printf("\tblock-size  : 0x%08x (%uKiB)\n",
			       info.block_size, info.block_size >> 10);
		printf("\tblock-count : %d\n", block_count);
		printf("\tecc-mode    : %s\n", nand_ecc_strs[ecc_mode]);
		if (ecc_mode == STMNAND_ECC_BCH18 ||
		    ecc_mode == STMNAND_ECC_BCH30)
			printf("\tbch-remap   : %s\n",
			       bch_no_remap ? "disabled" : "enabled");
		printf("\tbbt-type    : %s\n", nand_bbt_strs[bbt_type]);
		printf("\tbbt-vers    : %u\n", bbt_vers);
	}

	/* BBT uses 2-bits per block */
	bbt_size = block_count >> 2;
	bbt = (uint8_t *)xxmalloc(bbt_size);

	/* Mark all blocks as good */
	memset(bbt, 0xff, bbt_size);

	/* Buffer to hold page-aligned BBT data, including OOB/ECC */
	if (bbt_type == STMNAND_BBT_INBAND)
		n_pages = (info.block_size / info.page_size);
	else
		n_pages = (bbt_size + info.page_size - 1) / info.page_size;

	buf_size = n_pages * (info.page_size + info.oob_size);
	buf = (uint8_t *)xxmalloc(buf_size);

	/* Read list of bad blocks from file */
	fp = fopen(fn_bad_list, "r");
	while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
		if (sscanf(line, "%d", &block) != 1)
			continue;
		if (block >= block_count) {
			eprintf("block number [%d] exceeds block count [%d]\n",
				block, block_count);
			continue;
		}
		vbprintf("Marking block %04d bad\n", block);
		nand_bbt_set_mark(bbt, block, BBT_MARK_BAD_FACTORY);
	}
	fclose(fp);

	/* Generate Primary and Mirror BBT data */
	for (bak = 0; bak < 2; bak++) {

		/*
		 * Prepare BBT data 'linux' and 'inband'
		 */
		memset(buf, 0xff, buf_size);
		page = buf;
		oob = page + info.page_size;
		first_page = 1;
		b = bbt;
		total_size = bbt_size;
		while (total_size) {
			chunk_size = min(total_size, info.page_size);

			memcpy(page, b, chunk_size);

			/* 'linux' BBT signature is put in OOB of first page */
			if (bbt_type == STMNAND_BBT_LINUX && first_page) {
				nand_bbt_linux_fill_sig(oob, bak, bbt_vers);
				first_page = 0;
			}

			/* Perform ECC encoding */
			if (nand_ecc_encode(&info, page, oob, ecc_mode) != 0) {
				eprintf("failed to encode BBT data\n");
				ret = 1;
				goto out1;
			}

			/* Undo BCH Page-OOB mapping, if required */
			if (bch_no_remap) {
				nand_ecc_bch_unmap(&info, page, oob,
						   (ecc_mode ==
						    STMNAND_ECC_BCH18) ?
						   BCH_MODE_18BIT :
						   BCH_MODE_30BIT);
			}

			page += info.page_size + info.oob_size;
			oob += info.page_size + info.oob_size;
			total_size -= chunk_size;
			b += chunk_size;
		}

		if (bbt_type == STMNAND_BBT_INBAND) {
			/* 'inband' BBT signature is put in last page of block */
			page = (buf + buf_size) - (info.page_size + info.oob_size);
			oob = page + info.page_size;

			nand_bbt_inband_fill_sig(page, bak, bbt_vers,
						 (ecc_mode == STMNAND_ECC_BCH18) ?
						 BCH18_ECC_BYTES : BCH30_ECC_BYTES);

			/* Perform ECC encoding */
			if (nand_ecc_encode(&info, page, oob, ecc_mode) != 0) {
				eprintf("failed to encode BBT 'inband' signature\n");
				ret = 1;
				goto out1;
			}

			/* Undo BCH Page-OOB mapping, if required */
			if (bch_no_remap) {
				nand_ecc_bch_unmap(&info, page, oob,
						   (ecc_mode ==
						    STMNAND_ECC_BCH18) ?
						   BCH_MODE_18BIT :
						   BCH_MODE_30BIT);
			}
		}

		/* Write BBT data to file */
		_snprintf(fn_out, MAX_FN_LEN, "%s_%s", fn_bbt_base,
			 nand_bbt_bak_strs[bak]);

		vbprintf("Writing %s BBT data to file [%s]\n",
			 nand_bbt_bak_strs[bak], fn_out);
		if (write_data(fn_out, buf, buf_size) != 0) {
			eprintf("failed to write BBT data to file\n");
			ret = 1;
			goto out1;
		}
	}

 out1:
	free(buf);
	free(bbt);

	return ret;
}
