// config_nand.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include "../common/prog_info.h"

#if 0
extern "C" __declspec(dllexport)  
int RamFuncConfigFile(int usb_index, const struct chip_info_c *chip,
			int (*down)(int usb_index, const unsigned  char *p, unsigned long size, void *param),
			void *param)
{
#pragma pack(1)
	typedef struct _st_icinfo {
		unsigned int uiPageSize;
		unsigned int uiSectorSize;
		unsigned int uiChipSize;
		unsigned char uiJedecId[8];
		unsigned short usIdLength;
		unsigned short ausPinMask[4];
		unsigned short usPinLength;
		unsigned int uiMaxVoltage;
		unsigned int uiMinVoltage;

		unsigned short usReadIdCommand;
		unsigned short usChipEraseCommand;
		unsigned short usSectorEraseCommand;
		unsigned short usProgramCommand;
		unsigned short usReadCommand;
		unsigned short usReadStatusCommand;
		unsigned short usUnlockCommand;
		unsigned short usLockCommand;
		unsigned short usRfu;
	} PNOR_IC_INFO;
#pragma pack()
	PNOR_IC_INFO info = {
		chip->partition_info[0].page_size_in_byte,
		chip->partition_info[0].sector_size_in_byte,
		(unsigned int)chip->partition_info[0].size_in_bytes,
                {0, 0, 0, 0, 0, 0, 0, 0},
		(unsigned short)chip->flash_id_nr,
		{0, 0, 0, 0}, 0, 2200, 300,

		(unsigned short)chip->partition_info[0].init_parameter,
		(unsigned short)chip->partition_info[0].chip_erase_parameter,
		(unsigned short)chip->partition_info[0].sector_erase_parameter,
		(unsigned short)chip->partition_info[0].program_parameter,
		(unsigned short)chip->partition_info[0].read_parameter, 0,
		(unsigned short)chip->partition_info[0].unprotect_parameter,
		(unsigned short)chip->partition_info[0].protect_parameter, 0
	};
	memcpy(&info.uiJedecId, chip->flash_id, chip->flash_id_nr);

	return down(usb_index, (unsigned char *)&info, sizeof(info), param);
}

#endif

/* @chip the chip attribute from config-file
 * @buff receive the data of the ramproject structure
 * @size the buff size
 * @note
 *      if (buff == NULL)
 *              return sizeof(struct ramproject structure)
 *      if (size < sizeof(struct ramproject structure)
 *              return sizeof(struct ramproject structure)
 */

extern "C" __declspec(dllexport)  
int ramfunc_config(
                        const struct chip_info_c *chip,
                        unsigned char *buff,
                        unsigned long size
                  )
{
#pragma pack(1)
	typedef struct _st_icinfo {
		unsigned int uiPageSize;
		unsigned int uiSectorSize;
		unsigned int uiChipSizeInPage;
		unsigned char uiJedecId[8];
		unsigned short usIdLength;
		unsigned short ausPinMask[4];
		unsigned short usPinLength;
		unsigned int uiMaxVoltage;
		unsigned int uiMinVoltage;

		unsigned short usReadIdCommand;
		unsigned short usChipEraseCommand;
		unsigned short usSectorEraseCommand;
		unsigned short usProgramCommand;
		unsigned short usReadCommand;
		unsigned short usReadStatusCommand;
		unsigned short usUnlockCommand;
		unsigned short usLockCommand;
		unsigned short usRfu;
	} PNOR_IC_INFO;
#pragma pack()
	PNOR_IC_INFO info = {
		chip->partition_info[0].page_size_in_byte,
		chip->partition_info[0].sector_size_in_byte,
		(unsigned int)(chip->partition_info[0].size_in_bytes / chip->partition_info[0].page_size_in_byte),
                {0, 0, 0, 0, 0, 0, 0, 0},
		(unsigned short)chip->flash_id_nr,
		{0, 0, 0, 0}, 0, 2200, 300,

		(unsigned short)chip->partition_info[0].init_parameter,
		(unsigned short)chip->partition_info[0].chip_erase_parameter,
		(unsigned short)chip->partition_info[0].sector_erase_parameter,
		(unsigned short)chip->partition_info[0].program_parameter,
		(unsigned short)chip->partition_info[0].read_parameter, 0,
		(unsigned short)chip->partition_info[0].unprotect_parameter,
		(unsigned short)chip->partition_info[0].protect_parameter, 0
	};
	memcpy(&info.uiJedecId, chip->flash_id, chip->flash_id_nr);     


        if (!buff)
                return sizeof(struct _st_icinfo);
        if (size < sizeof(struct _st_icinfo))
                return sizeof(struct _st_icinfo);
 
        assert(buff);
        memcpy(buff, &info, sizeof(info));

        return 0;
}

