/*
 *  stmnand_clean_device.c
 *
 *    Clean NAND device of data and/or BBTs, with option to sync BBMs
 *    [Target]
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

#define NAME	"stmnand_clean_device"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tClean device of data and/or BBTs, with option to sync BBMs\n");
	printf("Usage: %s [OPTIONS] <mtddev>\n\n", NAME);
	printf("Options:\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode of BBT                 [detect]\n");
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
	printf("\t-m | --sync-bbms        Sync BBMs with BBT data         [no]\n");
	printf("\t-t | --erase-bbts       Erase BBT area                  [no]\n");
	printf("\t-d | --erase-data       Erase Data area                 [no]\n");
	printf("\t-s | --skip-bad=<skip>  Skip bad blocks when erasing    [bbt]\n");
	printf("\t                            none : skip none (erase bad blocks)\n");
	printf("\t                             bbt : skip blocks marked as bad in BBT\n");
	printf("\t                             bbm : skip blocks marked as bad in OOB area\n");
	printf("\t                             tag : skip blocks failing STM tags check\n");
	printf("\t                             ecc : skip blocks failing STM ECC tests\n");
	printf("\t-x | --no-spans-master  Disable 'MTD Spans Master' check\n");
	printf("\t-y | --debugfs=<path>   Specify alternative path to DEBUGFS\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n");
	printf("\n");
}


#define SKIP_BAD_NONE	0
#define SKIP_BAD_BBT	1
#define SKIP_BAD_BBM	2
#define SKIP_BAD_TAG	3
#define SKIP_BAD_ECC	4
char *skip_bad_strs[] = {
	[SKIP_BAD_NONE] = "none",
	[SKIP_BAD_BBT] = "bbt",
	[SKIP_BAD_BBM] = "bbm",
	[SKIP_BAD_TAG] = "tag",
	[SKIP_BAD_ECC] = "ecc",
};
MAKE_str2xxx(skip_bad, skip_bad_strs)

int main(int argc, char *argv[])
{
	const char *short_options = "e:b:ms:tdxy:vh";
	const struct option long_options[] = {
		{"ecc-mode", 1, 0, 'e'},
		{"bbt-type", 1, 0, 'b'},
		{"sync-bbms", 0, 0, 'm'},
		{"skip-bad", 1, 0, 's'},
		{"erase-bbts", 0, 0, 't'},
		{"erase-data", 0, 0, 'd'},
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
		.options = (NAND_IO_MTD_ENFORCE_SPANS_MASTER |
			    NAND_IO_MTD_BYPASS_BAD_PROTECT),
	};
	int ecc_mode = STMNAND_ECC_DETECT;
	int bbt_type = STMNAND_BBT_DETECT;
	int skip_bad = SKIP_BAD_BBT;
	int sync_bbms = 0;
	int erase_bbts = 0;
	int erase_data = 0;

	struct nand_info info;
	void *iohandle;

	struct nand_bbt_stats stats;
	uint8_t *page, *oob;
	uint8_t *bbt = NULL;

	int start_block, end_block;
	uint64_t offs;

	int scan_mode[] = {
		[SKIP_BAD_BBM] = NAND_BBT_SCAN_BBM,
		[SKIP_BAD_TAG] = NAND_BBT_SCAN_TAG,
		[SKIP_BAD_ECC] = NAND_BBT_SCAN_ECC,
	};
	char *scan_strs[] = {
		[SKIP_BAD_BBM] = "BBMs",
		[SKIP_BAD_TAG] = "STM ECC tags",
		[SKIP_BAD_ECC] = "STM ECC data [VERY SLOW]",
	};

	int i;
	int sync_count = 0;

	int ret = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'e':
			ecc_mode = nand_str2eccmode(optarg);
			if (ecc_mode == -1) {
				eprintf("invalid 'ecc-mode' argument [%s]\n",
					optarg);
				return 1;
			}

			break;
		case 'b':
			bbt_type = nand_str2bbttype(optarg);
			if (bbt_type == -1) {
				eprintf("invalid 'bbt-type' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'm':
			sync_bbms = 1;
			break;
		case 't':
			erase_bbts = 1;
			break;
		case 'd':
			erase_data = 1;
			break;
		case 's':
			skip_bad = str2skip_bad(optarg);
			if (skip_bad == -1) {
				eprintf("invalid 'skip-bad' argument [%s]\n",
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

	if (!sync_bbms && !erase_bbts && !erase_data) {
		eprintf("specify at least one of '--sync-bbms', '--erase-data' or '--erase_bbts'\n");
		return 1;
	}

	if (verbose) {
		printf("Arguments:\n");
		printf("\tmtddev       : %s\n", ioconfig.mtddev);
		printf("\tecc-mode     : %s\n", nand_ecc_strs[ecc_mode]);
		printf("\tbbt-type     : %s\n", nand_bbt_strs[bbt_type]);
		printf("\tsync-bbms    : %s\n", sync_bbms ? "yes" : "no");
		printf("\terase-bbts   : %s\n", erase_bbts ? "yes" : "no");
		printf("\terase-data   : %s\n", erase_data ? "yes" : "no");
		printf("\tskip-bad     : %s\n", skip_bad_strs[skip_bad]);
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

	if (sync_bbms) {
		sync_count = 0;
		vbprintf("Syncing BBMs to BBT data:\n");
		vbprintf("\tLoading BBT from device\n");
		bbt = nandio_bbt_load(iohandle, &info, ecc_mode, bbt_type, &stats);
		if (!bbt) {
			eprintf("Failed to load BBT\n");
			ret = 1;
			goto out1;
		}

		if (verbose)
			nand_bbt_dump_stats(&stats);

		page = xxmalloc(info.page_size + info.oob_size);
		oob = page + info.page_size;

		vbprintf("\tScanning BBMs\n");
		for (i = 0; i < info.block_count; i++) {
			if (!nand_bbt_isbad(bbt, i))
				continue;

			offs = ((uint64_t)i) << info.block_shift;

			if (nandio_bbt_scan_bbms_block(iohandle, &info, offs,
						       page, oob,
						       NAND_BBT_SCAN_ECC) == 0) {
				vbprintf("\t\t[%04u @ 0x%012llx] : syncing BBM\n",
					 i, offs);
				ret = nandio_write_bbm(iohandle, &info, page, oob,
						       offs);

				if (ret != 0)
					eprintf("failed to write BBM to 0x%012llx\n",
						offs);
				else
					sync_count++;
			}
		}
		vbprintf("\t\tSync'ed %d BBM(s)\n", sync_count);
		free(page);
	}

	if (erase_data || erase_bbts) {
		start_block = erase_data ?
			0 : info.block_count - NAND_BBT_NBLOCKS;
		end_block = erase_bbts ?
			info.block_count : info.block_count - NAND_BBT_NBLOCKS;

		switch (skip_bad) {
		case SKIP_BAD_NONE:
			break;
		case SKIP_BAD_BBT:
			if (!bbt) {
				vbprintf("Loading BBT from device\n");
				bbt = nandio_bbt_load(iohandle, &info, ecc_mode,
						      bbt_type, &stats);
				if (!bbt) {
					eprintf("failed to load BBT\n");
					ret = 1;
					goto out1;
				}

				if (verbose)
					nand_bbt_dump_stats(&stats);
			}
			break;
		case SKIP_BAD_BBM:
		case SKIP_BAD_TAG:
		case SKIP_BAD_ECC:
			if (bbt)
				free(bbt);

			vbprintf("Building BBT: scanning for %s\n",
				 scan_strs[skip_bad]);

			bbt = nandio_bbt_scan_bbms(iohandle, &info,
						   scan_mode[skip_bad],
						   verbose);

			if (!bbt) {
				eprintf("failed to build BBT\n");
				ret = 1;
				goto out1;
			}
			break;
		}

		vbprintf("Erasing device [0x%012llx - 0x%012llx]\n",
			 (uint64_t)start_block << info.block_shift,
			 ((uint64_t)end_block << info.block_shift) - 1);

		for (i = start_block; i < end_block; i++) {
			if (skip_bad != SKIP_BAD_NONE && nand_bbt_isbad(bbt, i)) {
				vbprintf("\n\tskipping bad block [%04u @ 0x%012llx]\n",
					 i, (uint64_t)i << info.block_shift);
				continue;
			}

			offs = ((uint64_t)i) << info.block_shift;
			ret = nandio_erase_block(iohandle, &info, offs);
			if (ret != 0) {
				eprintf("failed to erase block [%d @ 0x%012llx]\n",
					i, (uint64_t)i << info.block_shift);
			} else {
				if (verbose) {
					printf("\r\t%04u @ 0x%012llx [%2d%%]",
					       i, offs, (i * 100) / info.block_count);
					fflush(stdout);
				}
			}
		}
		vbprintf("\n");
	}

	vbprintf("*** Reboot for changes to be picked up by kernel ***\n");

 out1:
	if (bbt)
		free(bbt);

	nand_io_exit(iohandle);

	return ret;
}
