/*
 *  stmnand_bbt_info.c
 *
 *    Examine NAND device BBT information
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

#define NAME		"stmnand_bbt_info"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tExamine NAND device BBT information\n");
	printf("Usage: %s [OPTIONS] <mtddev>\n\n", NAME);
	printf("Options:\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode                        [detect]\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t                          detect : Detect ECC mode\n");
	printf("\t-b | --bbt-type=<bbt>   BBT type                        [detect]\n");
	printf("\t                           linux : default linux BBT\n");
	printf("\t                          inband : inband 'BCH' BBT\n");
	printf("\t                          detect : Detect BBT type\n");
	printf("\t-t | --scan-bbt         Scan BBT area                   [no]\n");
	printf("\t-m | --scan-bbm         Scan Blocks for BBMs            [no]\n");
	printf("\t-a | --scan-tag         Scan Blocks for STM ECC tags    [no]\n");
	printf("\t-c | --scan-ecc         Scan Blocks for STM ECC data    [no]\n");
	printf("\t-x | --no-spans-master  Disable 'MTD Spans Master' check\n");
	printf("\t-y | --debugfs=<path>   Specify alternative path to DEBUGFS\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}


int main(int argc, char *argv[])
{
	const char *short_options = "e:b:tmacxy:vh";
	const struct option long_options[] = {
		{"ecc-mode", 1, 0, 'e'},
		{"bbt-type", 1, 0, 'b'},
		{"scan-bbt", 0, 0, 't'},
		{"scan-bbm", 0, 0, 'm'},
		{"scan-tag", 0, 0, 'a'},
		{"scan-ecc", 0, 0, 'c'},
		{"no-spans-master", 0, 0, 'x'},
		{"debugfs", 1, 0, 'y'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	int option_index;
	int c;

	struct nand_io_mtd_config ioconfig = {
		.mtddev	= NULL,
		.debugfs = NULL,
		.options = NAND_IO_MTD_ENFORCE_SPANS_MASTER,
	};
	int ecc_mode = STMNAND_ECC_DETECT;
	int bbt_type = STMNAND_BBT_DETECT;
	int scan_bbt = 0;
	int scan_bbm = 0;
	int scan_tag = 0;
	int scan_ecc = 0;

	struct nand_info info;
	void *iohandle;

	struct nand_bbt_stats stats;
	uint8_t *bbt_flash = NULL;
	uint8_t *bbt_bbm = NULL;
	uint8_t *bbt_tag = NULL;
	uint8_t *bbt_ecc = NULL;

	int i;
	uint64_t offs;
	uint32_t bad;
	int bad_count;
	int found;

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
		case 'b':
			bbt_type = nand_str2bbttype(optarg);
			if (bbt_type == -1 || bbt_type == STMNAND_BBT_DETECT) {
				eprintf("invalid 'bbt-type' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 't':
			scan_bbt = 1;
			break;
		case 'm':
			scan_bbm = 1;
			break;
		case 'a':
			scan_tag = 1;
			break;
		case 'c':
			scan_ecc = 1;
			break;
		case 'x':
			ioconfig.options &= ~NAND_IO_MTD_ENFORCE_SPANS_MASTER;
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
		printf("\tecc-mode     : %s\n", nand_ecc_strs[ecc_mode]);
		printf("\tbbt-type     : %s\n", nand_bbt_strs[bbt_type]);
		printf("\tscan-bbt     : %s\n", scan_bbt ? "yes" : "no");
		printf("\tscan-bbm     : %s\n", scan_bbm ? "yes" : "no");
		printf("\tscan-tag     : %s\n", scan_tag ? "yes" : "no");
		printf("\tscan-ecc     : %s\n", scan_ecc ? "yes" : "no");
		printf("\tspans-master : check %s\n",
		       (ioconfig.options & NAND_IO_MTD_ENFORCE_SPANS_MASTER) ?
		       "enabled" : "disabled");
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

	if (scan_bbt) {
		printf("Scanning BBT blocks:\n");
		for (offs = info.size - (NAND_BBT_NBLOCKS * info.block_size);
		     offs < info.size;
		     offs += info.block_size) {
			found = nandio_bbt_scan_block(iohandle, &info, offs,
						      STMNAND_ECC_DETECT,
						      STMNAND_BBT_DETECT,
						      &stats);

			printf("\t[%u @ 0x%012llx]: ",
			       (unsigned int)(offs >> info.block_shift), offs);
			if (found == 1) {
				printf("found BBT:\n");
				nand_bbt_dump_stats(&stats);
			} else if (found == 0) {
				printf("BBT not found\n");
			} else {
				printf("failed to scan block\n");
			}
		}
		printf("\n");
	}

	printf("Loading BBT from NAND Flash:\n");
	bbt_flash = nandio_bbt_load(iohandle, &info, ecc_mode, bbt_type, &stats);
	if (bbt_flash) {
		nand_bbt_dump_stats(&stats);
		printf("\n");
	} else {
		eprintf("failed to load BBT from Flash\n");
	}

	if (scan_bbm) {
		printf("Scanning device for BBMs\n");
		bbt_bbm = nandio_bbt_scan_bbms(iohandle, &info,
					       NAND_BBT_SCAN_BBM, verbose);
		if (!bbt_bbm)
			eprintf("failed to scan BBMs\n");
		printf("\n");
	}

	if (scan_tag) {
		printf("Scanning device for STM ECC tags\n");
		bbt_tag = nandio_bbt_scan_bbms(iohandle, &info,
					       NAND_BBT_SCAN_TAG, verbose);
		if (!bbt_tag)
			eprintf("failed to scan STM ECC tags\n");
		printf("\n");
	}


	if (scan_ecc) {
		printf("Scanning device for STM ECC patterns [SLOW!]\n");
		bbt_ecc = nandio_bbt_scan_bbms(iohandle, &info,
					       NAND_BBT_SCAN_ECC, verbose);
		if (!bbt_ecc)
			eprintf("failed to scan STM ECC patterns\n");
		printf("\n");
	}

	if (!bbt_flash && !bbt_bbm && !bbt_tag && !bbt_ecc) {
		vbprintf("No BBTs loaded\n");
		goto out1;
	}

#define BBT_FLASH_BLOCKBAD	0x1
#define BBT_SCAN_BBM_BLOCKBAD	0x2
#define BBT_SCAN_TAG_BLOCKBAD	0x4
#define BBT_SCAN_ECC_BLOCKBAD	0x8
	printf("Bad Blocks:\n");
	printf("[%6s @ %14s]: ", "Block", "Offset");
	if (bbt_flash)
		printf("%10s ", "Flash BBT");
	if (bbt_bbm)
		printf("%10s ", "OOB BBMs");
	if (bbt_tag)
		printf("%10s ", "STM TAG");
	if (bbt_ecc)
		printf("%10s ", "STM ECC");
	printf("\n");
	bad_count = 0;
	for (i = 0; i < info.block_count; i++) {
		bad = 0;
		if (bbt_flash && nand_bbt_isbad(bbt_flash, i))
			bad |= BBT_FLASH_BLOCKBAD;
		if (bbt_bbm && nand_bbt_isbad(bbt_bbm, i))
			bad |= BBT_SCAN_BBM_BLOCKBAD;
		if (bbt_tag && nand_bbt_isbad(bbt_tag, i))
			bad |= BBT_SCAN_TAG_BLOCKBAD;
		if (bbt_ecc && nand_bbt_isbad(bbt_ecc, i))
			bad |= BBT_SCAN_ECC_BLOCKBAD;

		if (bad) {
			printf("[%06u @ 0x%012llx]: ",
			       i, (uint64_t)i << info.block_shift);
			if (bbt_flash)
				printf("%10s ", (bad & BBT_FLASH_BLOCKBAD) ?
				       "x" : " ");
			if (bbt_bbm)
				printf("%10s ", (bad & BBT_SCAN_BBM_BLOCKBAD) ?
				       "x" : " ");
			if (bbt_tag)
				printf("%10s ", (bad & BBT_SCAN_TAG_BLOCKBAD) ?
				       "x" : " ");
			if (bbt_ecc)
				printf("%10s ", (bad & BBT_SCAN_ECC_BLOCKBAD) ?
				       "x" : " ");
			printf("\n");
			bad_count++;
		}

	}
	if (!bad_count)
		printf("\tNo bad blocks detected\n");

	if (bbt_flash)
		free(bbt_flash);

	if (bbt_bbm)
		free(bbt_bbm);

	if (bbt_tag)
		free(bbt_tag);

	if (bbt_ecc)
		free(bbt_ecc);

 out1:
	nand_io_exit(iohandle);

	return 0;
}
