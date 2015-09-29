/*
 *  stmnand_bbt_build.c
 *
 *    Scan NAND device for bad blocks and write BBTs [Target]
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

#define NAME	"stmnand_bbt_build"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tScan NAND device for bad blocks and write BBTs\n");
	printf("Usage: %s [OPTIONS] <mtddev>\n\n", NAME);
	printf("Options:\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        Required\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t-s | --scan-mode=<scan> Bad block scanning mode         [tag]\n");
	printf("\t                             bbm : test for BBMs\n");
	printf("\t                             tag : test for valid STM ECC tags\n");
	printf("\t                             ecc : test for valid STM ECC data\n");
	printf("\t-x | --no-spans-master  Disable 'MTD Spans Master' check\n");
	printf("\t-y | --debugfs=<path>   Specify alternative path to DEBUGFS\n");
	printf("\t-q | --quiet            Supress confirmation messages   [no]\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

int main(int argc, char *argv[])
{
	const char *short_options = "e:s:xy:qvh";
	const struct option long_options[] = {
		{"ecc-mode", 1, 0, 'e'},
		{"scan-mode", 1, 0, 's'},
		{"no-spans-master", 0, 0, 'x'},
		{"debugfs", 1, 0, 'y'},
		{"quiet", 0, 0, 'q'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	int option_index;
	int c;

	struct nand_io_mtd_config ioconfig = {
		.mtddev	= NULL,
		.debugfs = NULL,
		.options = (NAND_IO_MTD_ENFORCE_SPANS_MASTER |
			    NAND_IO_MTD_BYPASS_BAD_PROTECT),
	};
	int ecc_mode = -1;
	int bbt_type = -1;
	int scan_mode = NAND_BBT_SCAN_TAG;
	int quiet = 0;

	struct nand_bbt_stats stats;
	uint8_t *bbt;
	uint64_t bbt_pair_offs[2];
	uint8_t bbt_pair_vers[2];
	int bad;

	struct nand_info info;
	void *iohandle;

	int ret = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'e':
			ecc_mode = nand_str2eccmode(optarg);
			if (ecc_mode == -1 || ecc_mode == STMNAND_ECC_DETECT) {
				eprintf("invalid 'ecc-mode' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 's':
			scan_mode = nand_str2bbtscanmode(optarg);
			if (scan_mode == -1) {
				eprintf("invalid 'scan-mode' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'x':
			ioconfig.options &= ~NAND_IO_MTD_ENFORCE_SPANS_MASTER;
			break;
		case 'y':
			ioconfig.debugfs = optarg;
			break;
		case 'q':
			quiet = 1;
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

	if (ecc_mode == -1) {
		eprintf("Required argument missing [ecc-mode]\n");
		return 1;
	}

	if (ecc_mode == STMNAND_ECC_AFM || ecc_mode == STMNAND_ECC_FLEX)
		bbt_type = STMNAND_BBT_LINUX;
	else
		bbt_type = STMNAND_BBT_INBAND;

	ioconfig.mtddev = argv[optind];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tmtddev       : %s\n", ioconfig.mtddev);
		printf("\tecc-mode     : %s\n", nand_ecc_strs[ecc_mode]);
		printf("\tbbt-type     : %s\n", nand_bbt_strs[bbt_type]);
		printf("\tscan-mode    : %s\n", nand_bbt_scan_strs[scan_mode]);
		printf("\tspans-master : check %s\n",
		       (ioconfig.options & NAND_IO_MTD_ENFORCE_SPANS_MASTER) ?
		       "enabled" : "disabled");
		if (ioconfig.debugfs)
			printf("\tdebugfs      : %s\n", ioconfig.debugfs);
		printf("\tquiet        : %s\n", quiet ? "yes" : "no");
		printf("\n");
	}

	iohandle = nand_io_init(&ioconfig, &info);
	if (!iohandle) {
		eprintf("failed to initialise NAND I/O support\n");
		return 1;
	}

	if (verbose)
		nand_dump_info(&info);

	if (!quiet || verbose) {
		vbprintf("Checking for existing BBTs\n");
		bbt = nandio_bbt_load(iohandle, &info,
				      STMNAND_ECC_DETECT, STMNAND_BBT_DETECT,
				      &stats);
		if (bbt) {
			printf("Exisiting BBT(s) present on NAND device\n");
			if (verbose)
				nand_bbt_dump_stats(&stats);

			free(bbt);

			if (!quiet) {
				printf("Continue anyway? (yes/no) ");
				if (confirm_operation() == 0) {
					ret = 1;
					goto out1;
				}
			}
		}
	}

	vbprintf("Scanning device for bad blocks\n");
	bbt = nandio_bbt_scan_bbms(iohandle, &info, scan_mode, verbose);
	if (!bbt) {
		eprintf("failed to scan NAND Flash\n");
		ret = 1;
		goto out1;
	}
	vbprintf("\n");

	if (verbose)
		nand_bbt_dump(&info, bbt, "Bad Blocks");

	if (!quiet || verbose) {
		bad = nand_bbt_countbad(&info, bbt);
		if (bad > info.block_count/20) {
			printf("Bad block count exceeds 5%%: suspect BBM corruption\n");

			if (!quiet) {
				printf("Continue anyway? (yes/no) ");
				fflush(stdout);
				if (confirm_operation() == 0) {
					ret = 1;
					goto out1;
				}
			}
		}
	}

	vbprintf("Writing BBT to NAND device\n");
	bbt_pair_vers[0] = 0;
	bbt_pair_vers[1] = 0;
	bbt_pair_offs[0] = 0;
	bbt_pair_offs[1] = 0;
	if (nandio_bbt_update(iohandle, &info, bbt, ecc_mode, bbt_type,
			      bbt_pair_vers[0] + 1, NAND_BBT_UPDATE_BOTH,
			      bbt_pair_offs, bbt_pair_vers) != 0) {
		eprintf("failed to write BBTs\n");
		ret = 1;
		goto out2;
	}

	if (verbose) {
		printf("Written new BBTs to Flash\n");
		printf("*** Reboot for changes to be picked up by kernel ***\n");
	}

 out2:
	free(bbt);
 out1:
	nand_io_exit(iohandle);

	return ret;
}
