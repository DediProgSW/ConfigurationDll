/*
 *  stmnand_bbt_transcode.c
 *
 *    Convert NAND Flash BBT from one format to another [Target]
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

#define NAME	"stmnand_bbt_transcode"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tConvert NAND Flash BBT from one format to another\n");
	printf("Usage: %s [OPTIONS] <mtddev>\n\n", NAME);
	printf("Options:\n");
	printf("\t-e | --src-ecc <ecc>   ECC mode of existing BBT     [detect]\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t                          detect : Detect ECC mode (src only)\n");
	printf("\t-b | --src-bbt <bbt>   BBT Type of existing BBT     [detect]\n");
	printf("\t                           linux : default linux BBT\n");
	printf("\t                          inband : inband 'BCH' BBT\n");
	printf("\t                          detect : Detect BBT type\n");
	printf("\t-E | --dst-ecc <ecc>   ECC mode of target BBT        Required\n");
	printf("\t-y | --debugfs=<path>  Specify alternative path to DEBUGFS\n");
	printf("\t-v | --verbose         Verbose messages             [no]\n");
	printf("\t-h | --help            Display this help and exit\n\n");
}

int main(int argc, char *argv[])
{
	const char *short_options = "e:b:E:y:vh";
	const struct option long_options[] = {
		{"src-ecc", 1, 0, 'e'},
		{"src-bbt", 1, 0, 'b'},
		{"dst-ecc", 1, 0, 'E'},
		{"debugfs", 1, 0, 'y'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	int option_index;
	int c;

	struct nand_io_mtd_config ioconfig = {
		.mtddev = NULL,
		.debugfs = NULL,
		.options = NAND_IO_MTD_BYPASS_BAD_PROTECT,
	};
	int src_ecc = STMNAND_ECC_DETECT;
	int src_bbt = STMNAND_BBT_DETECT;
	int dst_ecc = -1;
	int dst_bbt = -1;

	struct nand_bbt_stats stats;
	uint8_t *bbt;
	uint64_t bbt_pair_offs[2];
	uint8_t bbt_pair_vers[2];

	struct nand_info info;
	void *iohandle;

	int ret = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'e':
			src_ecc = nand_str2eccmode(optarg);
			if (src_ecc == -1) {
				eprintf("invalid 'src-ecc' argument [%s]\n",
					optarg);
				return 1;
			}

			break;
		case 'b':
			src_bbt = nand_str2bbttype(optarg);
			if (src_bbt == -1) {
				eprintf("invalid 'src-bbt' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'E':
			dst_ecc = nand_str2eccmode(optarg);
			if (dst_ecc == -1 || dst_ecc == STMNAND_ECC_DETECT) {
				eprintf("invalid 'dst-ecc' argument [%s]\n",
					optarg);
				return 1;
			}
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
		eprintf("Required argument missing [mtddev]\n");
		return 1;
	}

	if (dst_ecc == -1) {
		eprintf("Required argument missing [dst-ecc]\n");
		return 1;
	}

	if (dst_ecc == STMNAND_ECC_AFM || dst_ecc == STMNAND_ECC_FLEX)
		dst_bbt = STMNAND_BBT_LINUX;
	else
		dst_bbt = STMNAND_BBT_INBAND;

	ioconfig.mtddev = argv[optind];

	if (verbose) {
		printf("Arguments:\n");
		printf("\tmtddev       : %s\n", ioconfig.mtddev);
		printf("\tsrc-ecc      : %s\n", nand_ecc_strs[src_ecc]);
		printf("\tsrc-bbt      : %s\n", nand_bbt_strs[src_bbt]);
		printf("\tdst-ecc      : %s\n", nand_ecc_strs[dst_ecc]);
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

	vbprintf("Loading BBT from Flash\n");
	bbt = nandio_bbt_load(iohandle, &info, src_ecc, src_bbt, &stats);
	if (!bbt) {
		eprintf("Failed to load BBT\n");
		ret = 1;
		goto out1;
	}
	if (verbose)
		nand_bbt_dump_stats(&stats);

	vbprintf("Syncing Primary/Mirror BBTs\n");
	if (nandio_bbt_sync_pair(iohandle, &info, bbt, &stats,
				 bbt_pair_offs,
				 bbt_pair_vers) != 0) {
		eprintf("Failed to sync  Primary/Mirror BBTs\n");
		ret = 1;
		goto out2;
	}

	vbprintf("Converting BBT\n");
	/* Reset version, but keep existing offsets to ensure primary/mirror
	 * safeguards remain in force while converting BBTs (e.g. if erase fails
	 * while updating primary, we don't want to also erase the mirror BBT!)
	 */
	bbt_pair_vers[0] = 0;
	bbt_pair_vers[1] = 0;
	vbprintf("Writing converted BBTs to Flash\n");
	if (nandio_bbt_update(iohandle, &info, bbt, dst_ecc, dst_bbt,
			      bbt_pair_vers[0] + 1, NAND_BBT_UPDATE_BOTH,
			      bbt_pair_offs, bbt_pair_vers) != 0) {
		eprintf("Failed to write BBTs\n");
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
