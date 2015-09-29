/*
 *  stmnand_test_page.c
 *
 *    Analyse a page of NAND Flash [Target]
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
#include <errno.h>

#define NAME	"stmnand_test_page"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tAnalyse page of NAND Flash\n");
	printf("Usage: %s [OPTIONS] <mtddev>\n\n", NAME);
	printf("Options\n");
	printf("\t-o | --offset=<offs>    Offset of page                  [0x00000000]\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        [detect]\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t                          detect : Detect ECC mode\n");
	printf("\t-d | --dump-page        Dump page                       [no]\n");
	printf("\t-y | --debugfs=<path>   Specify alternative path to DEBUGFS\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

int main(int argc, char *argv[])
{
	const char *short_options = "o:e:dy:vh";
	const struct option long_options[] = {
		{"offset", 1, 0, 'o'},
		{"ecc-mode", 1, 0, 'e'},
		{"dump", 0, 0, 'd'},
		{"debugfs", 1, 0, 'y'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	char *endptr;
	int option_index;
	int c;

	struct nand_io_mtd_config ioconfig = {
		.mtddev = NULL,
		.debugfs = NULL,
		.options = 0,
	};
	uint64_t offs = 0;
	int ecc_mode = STMNAND_ECC_DETECT;
	int dump_page = 0;

	struct nand_info info;
	void *iohandle;
	uint8_t *page, *oob;
	struct nand_ecc_stats stats;

	int ret = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'o':
			offs = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid '--offset' argument [%s]\n",
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
		case 'd':
			dump_page = 1;
			break;
		case 'y':
			ioconfig.debugfs = optarg;
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
		eprintf("Required argument missing [<mtddev>]\n");
		return 1;
	}

	ioconfig.mtddev = argv[optind];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tmtddev       : %s\n", ioconfig.mtddev);
		printf("\toffset       : 0x%012llx\n", offs);
		printf("\tecc-mode     : %s\n", nand_ecc_strs[ecc_mode]);
		printf("\tdump-page    : %s\n", dump_page ? "yes" : "no");
		if (ioconfig.debugfs)
			printf("\tdebugfs      : %s\n", ioconfig.debugfs);
		printf("\n");
	}

	iohandle = nand_io_init(&ioconfig, &info);
	if (!iohandle) {
		eprintf("failed to initialise NAND I/O support\n");
		return 1;
	}

	if (verbose)
		nand_dump_info(&info);

	if (offs >= info.size) {
		eprintf("specified offset [0x%012llx] is beyond end of device\n",
			offs);
		ret = 1;
		goto out1;

	}

	if (offs & (info.page_size - 1)) {
		eprintf("specified offset [0x%012llx] is not page-aligned\n",
			offs);
		ret = 1;
		goto out1;
	}

	page = xxmalloc(info.page_size + info.oob_size);
	oob = page + info.page_size;

	if (nandio_read_page_ecc(iohandle, &info, page, oob, offs, ecc_mode,
				 &stats) < 0) {
		eprintf("failed to read page\n");
		ret = 1;
		goto out2;
	}

	printf("Page Status:\n");
	nand_ecc_dump_stats(&stats);

	if (dump_page)
		nand_dump_page(&info, page, oob, offs);

 out2:
	free(page);
 out1:
	nand_io_exit(iohandle);

	return ret;
}
