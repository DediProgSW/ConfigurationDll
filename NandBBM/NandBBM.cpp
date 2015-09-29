// NandBBM_t.cpp : Defines the exported functions for the DLL application.
//

// operation.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <tchar.h>
#include <time.h>
#include <assert.h>
#include <Windows.h>
#include "iphlpapi.h"
#include "../common/prog_info.h"
#include "zstring.h"
#include "../Log/log.h"
#include "../common/assist.h"
#include "../path/path.h"

#include <xprintf.h>
#include <stmnand.h>

#define MAX_FN_LEN	1024
#define MAX_LINE_LEN	64
#define NANDBLOCKSIZE 276480

// NandBBMInit.cpp : Defines the exported functions for the DLL application.
//

typedef int(register_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          reg_addr,
        unsigned short          *data,
        const void              *param
        );

typedef int(sram_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          address,
        unsigned int           *data,
        unsigned int            data_size,
        const void              *param
        );

typedef int(fifo_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned char           *cvpData,
        unsigned int            uiLength,
        const void              *param
        );

#define Read 0
#define Write 1

#define TOUT 6000

#define Err_USB -100
#define Err_FIFO -101
#define Err_ScanBB -1
#define Err_Read -2
#define Err_Write -3


#include "stdafx.h"
#include <vector>

static int verbose = 0;


static int fifo_wr(
	unsigned int len,
	unsigned char *buf,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned short fifolen;
	unsigned int tout;
	unsigned int remainLen;
	unsigned int i;

	unsigned int translen;
	remainLen = len;



	while (remainLen > 0)
	{
		CACL_TAKES_TIME_BEGIN(fifowr);
		do
		{
			reg_func(Read, 0x35, &fifolen, func_param);
			fifolen &= 0xFFF;
			tout = CACL_TAKES_TIME_END(fifowr);
		} while ((fifolen != 0) && (tout < 200));

		if (tout == 200)
			return Err_FIFO;

		translen = (remainLen < 1024) ? remainLen : 1024;
		if (fifo_func(Write, buf + (len - remainLen), translen, func_param))
			return Err_USB;

		remainLen -= translen;
	}

	return 0;

}

static int fifo_rd(
	unsigned int len,
	unsigned char *buf,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned short fifolen;
	unsigned int tout;
	unsigned int remainLen;
	unsigned int translen;

	remainLen = len;
	while (remainLen > 0)
	{
		CACL_TAKES_TIME_BEGIN(fiford);
		do
		{
			reg_func(Read, 0x36, &fifolen, func_param);
			fifolen &= 0xFFF;
			//Sleep(1);
			tout = CACL_TAKES_TIME_END(fiford);
		} while ((fifolen != (remainLen/2))&&(fifolen < 512) && (tout < 200));

		if (tout >= 200)
			return Err_FIFO;

		translen = (remainLen < 512) ? remainLen : 512;

		if (fifo_func(Read, buf + (len - remainLen), translen, func_param))
			return Err_USB;

		remainLen -= translen;
	}

	return 0;

}


static int WritePage(const struct chip_info_c       *chip,
	unsigned int PageAddress,
	unsigned int PageCount,
	unsigned char *buf,
	sram_func_t	*sram_func,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned int sramtemp[512];
	memset(sramtemp, 0, 512);
	sramtemp[0] = 0x80;
	sram_func(Write, 16, sramtemp, 4 * sizeof(unsigned int), func_param);

	memset(sramtemp, 0, 512);
	sramtemp[1] = chip->partition_info[0].program_parameter;
	sramtemp[2] = PageAddress;
	sramtemp[3] = PageCount;
	sramtemp[4] = chip->partition_info[0].page_size_in_byte;
	sramtemp[7] = chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte;
	sramtemp[8] = 0x07;

	log_add("Write page paramters: %x %x %x %x %x %x %x %x ", sramtemp[0], sramtemp[1], sramtemp[2], sramtemp[3], sramtemp[4], sramtemp[5], sramtemp[6], sramtemp[7]);
	sram_func(Write, 0, sramtemp, 16 * sizeof(unsigned int), func_param);

	fifo_wr(PageCount*chip->partition_info[0].page_size_in_byte, buf,reg_func, fifo_func, func_param);

	unsigned int timeout = 0;

	CACL_TAKES_TIME_BEGIN(pageread);

	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(pageread);
		if (timeout > TOUT)
			return Err_Write;
	}

	if (sramtemp[8] & 0x4000)
		return Err_Write;

	return 0;
}

static int ReadPage(const struct chip_info_c       *chip,
	unsigned int PageAddress,
	unsigned int PageCount,
	unsigned char *buf,
	sram_func_t	*sram_func,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned int sramtemp[512];
	memset(sramtemp, 0, 512);
	sramtemp[0] = 0x80;
	sram_func(Write, 16, sramtemp, 4 * sizeof(unsigned int), func_param);

	memset(sramtemp, 0, 512);
	sramtemp[1] = chip->partition_info[0].read_parameter;
	sramtemp[2] = PageAddress;
	sramtemp[3] = PageCount;
	sramtemp[4] = chip->partition_info[0].page_size_in_byte;
	sramtemp[7] = chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte;
	sramtemp[8] = 0x09;

	sram_func(Write, 0, sramtemp, 16 * sizeof(unsigned int), func_param);

	fifo_rd(PageCount*chip->partition_info[0].page_size_in_byte, buf, reg_func, fifo_func, func_param);

	unsigned int timeout = 0;

	CACL_TAKES_TIME_BEGIN(pagewrite);

	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(pagewrite);
		if (timeout > TOUT)
			return Err_Read;
	}

	if (sramtemp[8] & 0x4000)
		return Err_Read;

	return 0;
}

static int CheckBB(const struct chip_info_c       *chip,
	sram_func_t	*sram_func,
	const void *func_param,
	unsigned int *BBTable)
{

	unsigned int sramtemp[512];

	memset(sramtemp, 0, 512);
	sram_func(Read, 31, BBTable, 129 * sizeof(unsigned int), func_param);
	if (BBTable[0] != 0xFFFF)
	{
		log_add("Bad Block Table exist");
		return 0;
	}
		

	memset(sramtemp, 0, 512);
	sramtemp[0] = 0x01;
	sramtemp[2] = chip->partition_info[0].page_size_in_byte & 0xFFFFFF00;
	sram_func(Write, 16, sramtemp, 4*sizeof(unsigned int), func_param);

	memset(sramtemp, 0, 512);
	sramtemp[1] = chip->partition_info[0].init_parameter;
	sramtemp[3] = chip->partition_info[0].size_in_bytes / chip->partition_info[0].block_size_in_byte;
	sramtemp[4] = chip->partition_info[0].page_size_in_byte;
	sramtemp[7] = chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte;
	sramtemp[8] = 0x0c;

	log_add("Scan BB paramters: %x %x %x %x %x %x %x %x ", sramtemp[0], sramtemp[1], sramtemp[2], sramtemp[3], sramtemp[4], sramtemp[5], sramtemp[6], sramtemp[7]);
	sram_func(Write, 0, sramtemp, 16 * sizeof(unsigned int), func_param);

	unsigned int timeout = 0;

	CACL_TAKES_TIME_BEGIN(cardrst);

	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(cardrst);
		if (timeout > TOUT)
			return Err_ScanBB;
	}

	if (sramtemp[8] & 0x4000)
		return Err_ScanBB;

	log_add("Scan BB result: %x", sramtemp[8]);

	sram_func(Read, 31, BBTable, 129 * sizeof(unsigned int), func_param);


	return 0;
}


int bbt_gen(const struct chip_info_c *chip,unsigned int *bb_list,unsigned int bb_cnt,unsigned char *bbt_primary,unsigned char *bbt_mirror)
{

	char *endptr;
	int option_index;
	int c;

	char fn_out[MAX_FN_LEN];
	struct nand_info info;

	
	info.page_size = chip->partition_info[0].page_size_in_byte&(~0xFF);
	info.oob_size = chip->partition_info[0].page_size_in_byte&(0xFF);
	//info.block_size = chip->partition_info[0].block_size_in_byte;

	int ecc_mode = STMNAND_ECC_BCH30;
	int bbt_type = STMNAND_BBT_INBAND;
	int block_count = chip->partition_info[0].size_in_bytes / chip->partition_info[0].block_size_in_byte;

	info.block_size = info.page_size*(chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte);

	uint8_t bbt_vers = 1;
	int bch_no_remap = 0;
	

	int bbt_size;
	uint8_t *bbt, *b;
	int n_pages, buf_size;
	uint8_t *buf, *page, *oob;

	uint32_t chunk_size, total_size;
	int first_page;

	int block;
	int bak;

	int ret = 0;


	if (ecc_mode == STMNAND_ECC_AFM || ecc_mode == STMNAND_ECC_FLEX)
		bbt_type = STMNAND_BBT_LINUX;
	else
		bbt_type = STMNAND_BBT_INBAND;

	if (bbt_type == STMNAND_BBT_INBAND && info.block_size == -1) {
		eprintf("Required argument missing [block-size]\n");
		return 1;
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


	int i;
	for (i = 0; i < bb_cnt;i++)
		nand_bbt_set_mark(bbt, bb_list[i], BBT_MARK_BAD_FACTORY);

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

		if (bak == 0)
			memcpy(bbt_primary, buf, chip->partition_info[0].block_size_in_byte);
		else
			memcpy(bbt_mirror, buf, chip->partition_info[0].block_size_in_byte);

		/* Write BBT data to file */
		
		_snprintf(fn_out, MAX_FN_LEN, "%s_%s", "NandBBT",
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

int  bbt_check(
	const struct chip_info_c  *chip,
	unsigned int *BBTable,
	unsigned char *read_buf
	)
{
	char *endptr;
	int option_index;
	int c;

	char fn_out[MAX_FN_LEN];
	struct nand_info info;


	info.page_size = chip->partition_info[0].page_size_in_byte&(~0xFF);
	info.oob_size = chip->partition_info[0].page_size_in_byte&(0xFF);
	info.block_size = info.page_size*(chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte);

	int ecc_mode = STMNAND_ECC_BCH30;
	int bbt_type = STMNAND_BBT_INBAND;
	int block_count = chip->partition_info[0].size_in_bytes / chip->partition_info[0].block_size_in_byte;

	

	uint8_t bbt_vers = 1;
	int bch_no_remap = 0;


	int bbt_size;
	uint8_t *bbt, *b;
	int n_pages, buf_size;
	uint8_t *buf, *page, *oob;

	uint32_t chunk_size, total_size;
	int first_page;

	int block;
	int bak;

	int ret = 0;

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

	unsigned int i=0;
	int remain_bbt_size= bbt_size;
	while (remain_bbt_size > 0)
	{
		memcpy(bbt + 1024 * i, read_buf + i*(1024 + 54), (remain_bbt_size > 1024) ? 1024 : remain_bbt_size);
		remain_bbt_size -= 1024;
	}


	/* Generate Primary  BBT data */
	//for (bak = 0; bak < 2; bak++) {
	bak = 0;	//just compare primary
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

	//}

	BBTable[0] = 0;
	ret = memcmp(read_buf, buf, chip->partition_info[0].block_size_in_byte);
	if (ret == 0)
	{

		for ( i = 0; i < block_count; i++)
		{
			if (nand_bbt_get_mark(bbt, i) != 0x3)
			{
				log_add("Found bad block in bbt: %x", i);
				BBTable[0]++;
				BBTable[BBTable[0]] = i*(chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte);
			}
		}

	}

out1:
	free(buf);
	free(bbt);

	return ret;

}



extern "C" __declspec(dllexport)
int prepare_init(
                const struct chip_info_c       *chip,
                register_func_t         *reg_func,
		sram_func_t             *sram_func,
		fifo_func_t             *fifo_func,
		const void              *func_param
				 )
{
  
	unsigned int BBTable[256];
	unsigned char bbt_primary[NANDBLOCKSIZE];
	unsigned char bbt_mirror[NANDBLOCKSIZE];
	unsigned char read_buf[NANDBLOCKSIZE];
	unsigned int i;
	unsigned int ret;
	unsigned int result;
    log_add("Nand BBM Init");

	unsigned int block_cnt = chip->partition_info[0].size_in_bytes / chip->partition_info[0].block_size_in_byte;
	unsigned int page_per_block = chip->partition_info[0].block_size_in_byte / chip->partition_info[0].page_size_in_byte;

	log_add("Chip total blocks: %d, Page number per block: %d", block_cnt, page_per_block);


	unsigned short regvalue = 1;
	reg_func(Write, 0x30, &regvalue, func_param);


	//check if there's valid bbt
	result = ReadPage(chip, (block_cnt - 1)*page_per_block, page_per_block, read_buf, sram_func, reg_func, fifo_func, func_param);
	if (result)
	{
		log_add("Read Nand flash fail");
		goto end;
	}

	if (bbt_check(chip, BBTable, read_buf))
	{

		if (CheckBB(chip, sram_func, func_param, BBTable))
		{
			log_add("Scan Bad block fail");
			result = -1;
			goto end;
		}

		if (BBTable[0] >= 126)
		{
			log_add("Too many bad blocks!");
			result = -1;
			goto end;
		}


		log_add("Total bad block count: %d, They are:", BBTable[0]);
		for (i = 0; i < BBTable[0]; i++)
		{
			BBTable[i + 1] /= page_per_block;
			log_add("%d  ", BBTable[i + 1]);
			if ((BBTable[i + 1] == (block_cnt - 1)) || (BBTable[i + 1] == (block_cnt - 2)))
			{
				log_add("Block for  bbt is bad");
				result = -1;
				goto end;
			}
		}

		log_add("Generate BCH30 BBT");
		bbt_gen(chip, &BBTable[1], BBTable[0], bbt_primary, bbt_mirror);

		log_add("Write BCH30 BBT to memory:");
		result = WritePage(chip, (block_cnt - 1)*page_per_block, page_per_block, bbt_primary, sram_func, reg_func, fifo_func, func_param);
		if (result)
		{
			log_add("Write primary bbt fail");
			goto end;
		}

		result = WritePage(chip, (block_cnt - 2)*page_per_block, page_per_block, bbt_mirror, sram_func, reg_func, fifo_func, func_param);
		if (result)
		{
			log_add("Write mirror bbt fail");
			goto end;
		}

		//verify
		result = ReadPage(chip, (block_cnt - 1)*page_per_block, page_per_block, read_buf, sram_func, reg_func, fifo_func, func_param);
		if (result)
		{
			log_add("Read primary fail");
			goto end;
		}
		result = memcmp(read_buf, bbt_primary, chip->partition_info[0].block_size_in_byte);
		if (result)
		{
			log_add("Verify primary fail");
			goto end;
		}

		result = ReadPage(chip, (block_cnt - 2)*page_per_block, page_per_block, read_buf, sram_func, reg_func, fifo_func, func_param);
		if (result)
		{
			log_add("Read mirror fail");
			goto end;
		}
		result = memcmp(read_buf, bbt_mirror, chip->partition_info[0].block_size_in_byte);
		if (result)
		{
			log_add("Verify mirror fail");
			goto end;
		}


		//Add last 2 blocks into bad block list to avoid erasing
		for (i = 0; i < BBTable[0]; i++)
			BBTable[i + 1] *= page_per_block;

		BBTable[BBTable[0] + 1] = (block_cnt - 2)*page_per_block;
		BBTable[BBTable[0] + 2] = (block_cnt - 1)*page_per_block;
		BBTable[0] = BBTable[0] + 2;

		sram_func(Write, 31, BBTable, 129 * sizeof(unsigned int), func_param);
		result = 0;

		result = 0;

		

		goto end;
	}
	else
	{
		//Add last 2 blocks into bad block list to avoid erasing
		BBTable[BBTable[0] + 1] = (block_cnt - 2)*page_per_block;
		BBTable[BBTable[0] + 2] = (block_cnt - 1)*page_per_block;
		BBTable[0] = BBTable[0] + 2;

		log_add("There's valid bbt in the flash");
		sram_func(Write, 31, BBTable, 129 * sizeof(unsigned int), func_param);
		result = 0;	

		goto end;
	}

	

end:
	regvalue = 0;
	reg_func(Write, 0x30, &regvalue, func_param);
	return result;

}
