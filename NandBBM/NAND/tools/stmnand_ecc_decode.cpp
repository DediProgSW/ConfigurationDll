/*
 *  stmnand_ecc_decode.c
 *
 *    Decode NAND ECC data (page+oob) to retrieve original payload [Host]
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

#define NAME	"stmnand_ecc_decode"
#include <xprintf.h>
#include <stmnand.h>

static int verbose = 0;

static void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tDecode NAND image [page:oob:page:oob...] to retrieve original\n");
	printf("\tpayload\n");
	printf("Usage: %s [OPTIONS] <fn_in> <fn_out>\n\n", NAME);
	printf("\t-p | --page-size        Page Size                       Required\n");
	printf("\t-o | --oob-size         OOB Size                        Required\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        [detect]\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t                          detect : Detect ECC mode\n");
	printf("\t-n | --bch-no-remap     Input data is not BCH ECC remapped\n");
	printf("\t-v | --verbose          Verbose messages\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

int main_ecc_decode(int argc, char *argv[])
{
	const char *short_options = "p:o:e:nvh";
	const struct option long_options[] = {
		{"page-size", 1, 0, 'p'},
		{"oob-size", 1, 0, 'o'},
		{"ecc-mode", 1, 0, 'e'},
		{"bch-no-remap",  0, 0, 'n'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	char *endptr;
	int option_index;
	int c;

	char *fn_in;
	char *fn_out;
	struct nand_info info;

	info.page_size = -1;
	info.oob_size = -1;

	int ecc_mode = STMNAND_ECC_DETECT;
	int bch_no_remap = 0;

	FILE *fp_in;
	FILE *fp_out;
	struct nand_ecc_stats stats;
	uint8_t *page, *oob;
	long len_in;


	int ret = 0;

	/* Parse options */
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
		case 'e':
			ecc_mode = nand_str2eccmode(optarg);
			if (ecc_mode == -1) {
				eprintf("invalid 'ecc-mode' argument [%s]\n",
					optarg);
				return 1;
			}
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

	if (argc - optind < 2) {
		eprintf("Required argument missing [<fn_in> and/or <fn_out>]\n");
		return 1;
	}
	if (info.page_size == -1) {
		eprintf("Required argument missing [page-size]\n");
		return 1;
	}
	if (info.oob_size == -1) {
		eprintf("Required argument missing [oob-size]\n");
		return 1;
	}

	fn_in = argv[optind];
	fn_out = argv[optind + 1];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tfn_in       : %s\n", fn_in);
		printf("\tfn_out      : %s\n", fn_out);
		printf("\tpage-size   : %d\n", info.page_size);
		printf("\toob-size    : %d\n", info.oob_size);
		printf("\tecc-mode    : %s\n", nand_ecc_strs[ecc_mode]);
		if (ecc_mode == STMNAND_ECC_BCH18 ||
		    ecc_mode == STMNAND_ECC_BCH30)
			printf("\tbch-no-remap : %s\n",
			       bch_no_remap ? "disabled" : "enabled");
	}

	page = (uint8_t*)xxmalloc(info.page_size + info.oob_size);
	oob = page + info.page_size;

	/*
	 * Open/Create image files
	 */
	fp_in = fopen(fn_in, "rb");
	if (!fp_in) {
		eprintf("Failed to open input file [%s]\n", fn_in);
		ret = 1;
		goto out1;
	}

	fp_out = fopen(fn_out, "wb");
	if (!fp_out) {
		eprintf("Failed to create output file [%s]\n", fn_out);
		ret = 1;
		goto out2;
	}

	/* Get size of input image */
	if (fseek(fp_in, 0, SEEK_END) != 0) {
		eprintf("Failed to seek input file\n");
		ret = 1;
		goto out3;
	}
	len_in = ftell(fp_in);
	if (len_in < 0) {
		eprintf("Failed to get size of input file\n");
		ret = 1;
		goto out3;
	}
	vbprintf("Input file: %ld bytes\n", len_in);

	if (len_in % (info.page_size + info.oob_size) != 0) {
		eprintf("Input image is not multiple of (page + OOB) size\n");
		ret = 1;
		goto out3;
	}

	rewind(fp_in);
	while (len_in > 0) {

		/* Read data from file [page:oob] */
		if (fread(page, info.page_size + info.oob_size,
			  1, fp_in) != 1) {
			eprintf("Failed to read input image\n");
			ret = 1;
			goto out3;
		}

		/* Perform BCH Page-OOB remapping, if required */
		if ((ecc_mode == STMNAND_ECC_BCH18 ||
		     ecc_mode == STMNAND_ECC_BCH30) &&
		    bch_no_remap)
			nand_ecc_bch_remap(&info, page, oob,
					   (ecc_mode ==
					    STMNAND_ECC_BCH18) ?
					   BCH_MODE_18BIT :
					   BCH_MODE_30BIT);

		if (nand_ecc_decode(&info, page, oob, ecc_mode, &stats) != 0) {
			eprintf("Failed to decode data\n");
			ret = 1;
			goto out3;
		}

		if (ecc_mode == STMNAND_ECC_DETECT) {
			if (verbose)
				nand_ecc_dump_stats(&stats);
			ecc_mode = stats.ecc;
		}

		/* Write data to file */
		if (fwrite(page, info.page_size, 1, fp_out) != 1) {
			eprintf("Failed to write output image\n");
			ret = 1;
			goto out1;
		}

		len_in -= info.page_size + info.oob_size;
	}

 out3:
	fclose(fp_out);
 out2:
	fclose(fp_in);
 out1:
	free(page);

	return ret;
}
