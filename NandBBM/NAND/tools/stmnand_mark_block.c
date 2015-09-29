/*
 *  stmnand_mark_block.c
 *
 *      Update a block's marker status in the BBT and/or OOB [Target]
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

#define NAME	"stmnand_mark_block"
#include <xprintf.h>
#include <stmnand.h>

int verbose = 0;

void print_usage(void)
{
	printf("STM NAND Tools (%s): %s\n", VERSION, NAME);
	printf("\tUpdate a block's marker status in the BBT and/or OOB\n");
	printf("Usage: %s [OPTIONS] <mtddev> \n\n", NAME);
	printf("Options:\n");
	printf("\t-e | --ecc-mode=<ecc>   ECC mode of BBT                 [detect]\n");
	printf("\t                           noecc : No ECC applied\n");
	printf("\t                            flex : Hamming Flex mode ECC\n");
	printf("\t                             afm : Hamming AFM ECC\n");
	printf("\t                            boot : Hamming Boot-mode ECC\n");
	printf("\t                           bch18 : BCH 18-bit ECC\n");
	printf("\t                           bch30 : BCH 30-bit ECC\n");
	printf("\t                          detect : Detect ECC mode\n");
	printf("\t-b | --bbt-type=<bbt>   Type of BBT                     [detect]\n");
	printf("\t                           linux : default linux BBT\n");
	printf("\t                          inband : inband 'BCH' BBT\n");
	printf("\t                          detect : Detect BBT type\n");
	printf("\t-l | --location=<loc>   Mark block in                   [bbt]\n");
	printf("\t                             bbt : BBT\n");
	printf("\t                             bbm : BBM (OOB)\n");
	printf("\t                            both : BBT and BBM\n");
	printf("\t-m | --mark=<mark>      Mark block as                   [bad]\n");
	printf("\t                             bad : Bad\n");
	printf("\t                            good : Good\n");
	printf("\t-o | --offset=<offs>    Offset of block to mark\n");
	printf("\t-n | --block-num=<num>  Block number to mark\n");
	printf("\t-x | --no-spans-master  Disable 'MTD Spans Master' check\n");
	printf("\t-y | --debugfs=<path>   Specify alternative path to DEBUGFS\n");
	printf("\t-v | --verbose          Verbose messages                [no]\n");
	printf("\t-h | --help             Display this help and exit\n\n");
}

#define MARK_LOC_BBT		0
#define MARK_LOC_BBM		1
#define MARK_LOC_BOTH		2
char *mark_loc_strs[] = {
	[MARK_LOC_BBT]	= "bbt",
	[MARK_LOC_BBM]	= "bbm",
	[MARK_LOC_BOTH]	= "both",
};
MAKE_str2xxx(mark_loc, mark_loc_strs)

#define BLOCK_MARK_GOOD		0
#define BLOCK_MARK_BAD		1
char *block_mark_strs[] = {
	[BLOCK_MARK_GOOD]	= "good",
	[BLOCK_MARK_BAD]	= "bad",
};
MAKE_str2xxx(block_mark, block_mark_strs)

int main(int argc, char *argv[])
{
	const char *short_options = "e:b:l:m:o:n:xy:vh";
	const struct option long_options[] = {
		{"ecc-mode", 1, 0, 'e'},
		{"bbt-type", 1, 0, 'b'},
		{"location", 1, 0, 'l'},
		{"mark", 1, 0, 'm'},
		{"offset", 1, 0, 'o'},
		{"block-num", 1, 0, 'n'},
		{"no-spans-master", 0, 0, 'x'},
		{"debugfs", 1, 0, 'y'},
		{"verbose", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	char *endptr;
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
	int mark_loc = MARK_LOC_BBT;
	int block_mark = BLOCK_MARK_BAD;
	int offs_set = 0;
	int block_set = 0;
	uint64_t offs;
	int block;

	struct nand_bbt_stats stats;
	uint8_t *bbt;
	uint64_t bbt_pair_offs[2];
	uint8_t bbt_pair_vers[2];

	int bad;
	uint8_t *page, *oob;

	struct nand_info info;
	void *iohandle;

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
		case 'l':
			mark_loc = str2mark_loc(optarg);
			if (mark_loc == -1) {
				eprintf("invalid 'location' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'm':
			block_mark = str2block_mark(optarg);
			if (block_mark == -1) {
				eprintf("invalid 'mark' argument [%s]\n",
					optarg);
				return 1;
			}
			break;
		case 'o':
			offs = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'offset' argument [%s]\n",
					optarg);
				return 1;
			}
			offs_set = 1;
			break;
		case 'n':
			block = strtoul(optarg, &endptr, 0);
			if (*endptr != '\0') {
				eprintf("invalid 'block-num' argument [%s]\n",
					optarg);
				return 1;
			}
			block_set = 1;
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

	if (block_set == offs_set) {
		eprintf("specify '--offset' *OR* '--block_num'\n");
		return -1;
	}

	if (verbose) {
		printf("Arguments:\n");
		printf("\tmtddev       : %s\n", ioconfig.mtddev);
		printf("\tecc-mode     : %s\n", nand_ecc_strs[ecc_mode]);
		printf("\tbbt-type     : %s\n", nand_bbt_strs[bbt_type]);
		printf("\tlocation     : %s\n", mark_loc_strs[mark_loc]);
		printf("\tblock-mark   : %s\n", block_mark_strs[block_mark]);
		if (block_set)
			printf("\tblock-num    : %d\n", block);
		else
			printf("\toffset       : 0x%012llx\n", offs);
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

	if (block_set)
		offs = (uint64_t)block << info.block_shift;
	else
		block = offs >> info.block_shift;


	if (block >= info.block_count) {
		eprintf("specified block [%04u @ 0x%012llx] is beyond end of device\n",
			block, offs);
		ret = 1;
		goto out1;
	}

	if (offs & (info.block_size - 1)) {
		eprintf("specified offset [0x%012llx] is not block-aligned\n",
			offs);
		ret = 1;
		goto out1;
	}

	vbprintf("Marking block %s [%04u @ 0x%012llx]:\n",
		 block_mark_strs[block_mark], block, offs);

	if (mark_loc == MARK_LOC_BBM || mark_loc == MARK_LOC_BOTH) {
		page = xxmalloc(info.page_size + info.oob_size);
		oob = page + info.page_size;

		if (block_mark == BLOCK_MARK_BAD) {
			vbprintf("\tWriting BBM: ");
			if (nandio_bbt_scan_bbms_block(iohandle, &info, offs,
						       page, oob,
						       NAND_BBT_SCAN_ECC) == 0) {
				ret = nandio_write_bbm(iohandle, &info, page,
						       oob, offs);
				if (ret != 0) {
					eprintf("failed to write BBM to 0x%012llx\n",
						offs);
				} else {
					vbprintf("done\n");
				}
			} else {
				vbprintf("already marked bad!\n");
			}
		} else {
			vbprintf("\tErasing BBM (and block): ");
			if (nandio_bbt_scan_bbms_block(iohandle, &info, offs,
						       page, oob,
						       NAND_BBT_SCAN_BBM) == 1) {
				ret = nandio_erase_block(iohandle, &info, offs);
				if (ret != 0) {
					eprintf("failed to erase BBM at 0x%012llx\n",
						offs);
				} else {
					vbprintf("done\n");
				}
			} else {
				vbprintf("no BBM to erase\n");
			}
		}

		free(page);
	}

	if (mark_loc == MARK_LOC_BBT || mark_loc == MARK_LOC_BOTH) {
		vbprintf("\tLoading BBT from Flash\n");
		bbt = nandio_bbt_load(iohandle, &info, ecc_mode, bbt_type, &stats);
		if (!bbt) {
			eprintf("Failed to load BBT from Flash\n");
			ret = 1;
			goto out1;
		}

		if (verbose)
			nand_bbt_dump_stats(&stats);

		bad = nand_bbt_isbad(bbt, block);
		if ((bad && block_mark == BLOCK_MARK_BAD) ||
		    (!bad && block_mark == BLOCK_MARK_GOOD)) {
			vbprintf("\tBlock already marked %s in BBT\n",
				 block_mark_strs[block_mark]);
		} else {
			vbprintf("\tSyncing Primary/Mirror BBTs\n");
			if (nandio_bbt_sync_pair(iohandle, &info, bbt, &stats,
						 bbt_pair_offs,
						 bbt_pair_vers) != 0) {
				eprintf("Failed to sync Primary/Mirror BBTs\n");
				ret = 1;
				free(bbt);
				goto out1;
			}
			vbprintf("\tUpdating BBT entry\n");
			nand_bbt_set_mark(bbt, block,
					  (block_mark == BLOCK_MARK_BAD) ?
					  BBT_MARK_BAD_WEAR : BBT_MARK_GOOD);
			vbprintf("\tWriting BBTs to Flash\n");
			if (nandio_bbt_update(iohandle, &info, bbt,
					      stats.ecc, stats.bbt,
					      bbt_pair_vers[0] + 1,
					      NAND_BBT_UPDATE_BOTH,
					      bbt_pair_offs, bbt_pair_vers) != 0) {
				eprintf("Failed to write BBTs\n");
				ret = 1;
				free(bbt);
				goto out1;
			}
		}

		free(bbt);
	}

	if (verbose) {
		printf("Updated BBTs and/or BBM\n");
		printf("*** Reboot for changes to be picked up by kernel ***\n");
	}


 out1:
	nand_io_exit(iohandle);

	return ret;
}
