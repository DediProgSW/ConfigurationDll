/*
 *  stmnand_ecc_test.c
 *
 *    Test ECC algorithms [Host]
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

#define NAME	"stmnand_ecc_test"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

/*
 * STM NAND ECC 'Sector 0' ranges:
 *
 * Boot-mode
 *	Data0:		0:128
 *	ECC0:		<page_size>:3
 *
 * Flex
 *      Data0:		0:256
 *      ECC0(SP):	512:3
 *      ECC0(LP):       2088:3
 *
 * AFM
 *	Data0:		0:512
 *      ECC0		<page_size>:3 + <page_size+6>:1(bits 0 and 1!!!)
 *
 * BCH18
 *	Data0:		0:1024
 *	ECC0:		1024:32
 *
 * BCH30
 *	Data0:		0:1024
 *	ECC0:		1024:54
 */

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tTest ECC algorithms on page of sample data <fn_in>\n");
	printf("Usage: %s [OPTIONS] <fn_in>\n\n", NAME);
	printf("Options:\n");
	printf("\t-p | --page-size        Page Size                       Required\n");
	printf("\t-o | --oob-size         OOB Size                        Required\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        Required\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t-i | --inject-errors    Number of bit-errors to inject\n");
	printf("\t-r | --errors-range     Bytes range in which to inject errors\n");
	printf("\t-s | --seed             Seed RNG, used for error injection\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

int main(int argc, char *argv[])
{
	const char *short_options = "p:o:e:i:r:s:vh";
	const struct option long_options[] = {
		{"page-size", 1, 0, 'p'},
		{"oob-size", 1, 0, 'o'},
		{"ecc-mode", 1, 0, 'e'},
		{"inject-errors", 1, 0, 'i'},
		{"errors-range", 1, 0, 'r'},
		{"seed", 1, 0, 's'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	char *endptr;
	int option_index;
	int c;

	char *fn_in;
	struct nand_info info = {
		.page_size = -1,
		.oob_size = -1,
	};
	int ecc = STMNAND_ECC_NONE;
	int inj_errors_base = -1;
	int inj_errors_size = -1;
	int n_errors_inj = 0;
	int seed = 123;

	int n_errors_err;
	int n_errors_out;
	int *err_pos_inj;
	int *err_pos_err;
	int *err_pos_out;

	uint8_t *page_in;
	uint8_t *page_enc, *oob_enc;
	uint8_t *page_err, *oob_err;
	uint8_t *page_fix, *oob_fix;

	struct nand_ecc_stats stats;

	int pos;
	int i;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'p':
			info.page_size = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'page-size' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'o':
			info.oob_size = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'oob-size' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'e':
			ecc = nand_str2eccmode(optarg);
			if (ecc == -1) {
				eprintf("invalid 'ecc-mode' argument [%s]\n", optarg);
				return 1;
			}

			break;
		case 'i':
			n_errors_inj = atoi(optarg);
			break;
		case 'r':
			if (sscanf(optarg, "%d:%d",
				   &inj_errors_base, &inj_errors_size) != 2) {
				eprintf("invalid 'errors-range' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 's':
			seed = atoi(optarg);
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

	if (argc - optind < 1) {
		eprintf("Required argument missing [<fn_in>]\n");
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

	if (inj_errors_base == -1) {
		inj_errors_base = 0;
		inj_errors_size = info.page_size + info.oob_size;
	}

	if ((inj_errors_base + inj_errors_size) >
	    (info.page_size + info.oob_size)) {
		eprintf("specified 'inj-range' extends beyound device\n");
		return 1;
	}

	fn_in = argv[optind];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tfn_in         : %s\n", fn_in);
		printf("\tpage-size     : %d\n", info.page_size);
		printf("\toob-size      : %d\n", info.oob_size);
		printf("\tecc           : %s\n", nand_ecc_strs[ecc]);
		printf("\tinject-errors : %d\n", n_errors_inj);
		printf("\terrors-range  : start = %d, size = %d\n",
		       inj_errors_base, inj_errors_size);
		printf("\tseed          : %d\n", seed);
		printf("\n");
	}

	srand(seed);
	page_in = xxmalloc(info.page_size);

	if (read_data(fn_in, page_in, info.page_size) != 0) {
		eprintf("failed to read data file [%s]\n", fn_in);
		return 1;
	}

	page_enc = xxmalloc(info.page_size + info.oob_size);
	oob_enc = page_enc + info.page_size;
	memcpy(page_enc, page_in, info.page_size);
	memset(oob_enc, 0xff, info.oob_size);
	nand_ecc_encode(&info, page_enc, oob_enc, ecc);

	nand_dump_page(&info, page_enc, oob_enc, 0);

	page_err = xxmalloc(info.page_size + info.oob_size);
	oob_err = page_err + info.page_size;
	memcpy(page_err, page_enc, info.page_size);
	memcpy(oob_err, oob_enc, info.oob_size);

	inject_errors(page_err + inj_errors_base, inj_errors_size, n_errors_inj,
		      &err_pos_inj);
	for (i = 0; i < n_errors_inj; i++)
		err_pos_inj[i] += inj_errors_base * 8;

	printf("Injected Errors:   ");
	for (i = 0; i < n_errors_inj; i++) {
		pos = err_pos_inj[i];
		printf("%d[%d:%d] ", pos, pos/8, pos%8);
	}
	printf("\n");


	n_errors_err = detect_errors(page_err, page_enc,
				     info.page_size + info.oob_size, &err_pos_err);
	printf("Dectected Errors:  ");
	if (n_errors_err > 0) {
		for (i = 0; i < n_errors_err; i++) {
			pos = err_pos_err[i];
			printf("%d[%d:%d] ", pos, pos/8, pos%8);
		}
		printf("\n");
	} else {
		printf("n_errors = %d\n", stats.n_errors);
	}



	page_fix = xxmalloc(info.page_size + info.oob_size);
	oob_fix = page_fix + info.page_size;
	memcpy(page_fix, page_err, info.page_size);
	memcpy(oob_fix, oob_err, info.oob_size);

	nand_ecc_decode(&info, page_fix, oob_fix, ecc, &stats);
	printf("Corrected Errors:  ");
	if (stats.n_errors > 0) {
		for (i = 0; i < stats.n_errors; i++) {
			pos = stats.bit_err_pos[i];
			printf("%d[%d:%d] ", pos, pos/8, pos%8);
		}
		printf("\n");
	} else {
		printf("n_errors = %d\n", stats.n_errors);
	}

	n_errors_out = detect_errors(page_in, page_fix, info.page_size, &err_pos_out);
	printf("Remaining Errors:  ");
	if (n_errors_out) {
		for (i = 0; i < n_errors_out; i++) {
			pos = err_pos_out[i];
			printf("%d[%d:%d] ", pos, pos/8, pos%8);
		}
		printf("\n");
	} else {
		printf("none\n");
	}

	free(page_in);
	free(page_enc);
	free(page_err);
	free(page_fix);

	return 0;
}
