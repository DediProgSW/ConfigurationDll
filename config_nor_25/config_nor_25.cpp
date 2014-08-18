// config_nor_25.cpp : Defines the exported functions for the DLL application.
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
		unsigned int uiJedecId;
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
		chip->partition_info[0].size_in_bytes,
		0,
		chip->flash_id_nr,
		{0, 0, 0, 0}, 0, 2200, 300,

		chip->partition_info[0].init_parameter,
		chip->partition_info[0].chip_erase_parameter,
		chip->partition_info[0].sector_erase_parameter,
		chip->partition_info[0].program_parameter,
		chip->partition_info[0].read_parameter, 0,
		chip->partition_info[0].unprotect_parameter,
		chip->partition_info[0].protect_parameter, 0
	};
	memcpy(&info.uiJedecId, chip->flash_id, chip->flash_id_nr);

	return down(usb_index, (unsigned char *)&info, sizeof(info), param);
}

#endif         

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
		unsigned int uiChipSize;
		unsigned int uiJedecId;
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
		chip->partition_info[0].size_in_bytes,
		0,
		chip->flash_id_nr,
		{0, 0, 0, 0}, 0, 2200, 300,

		chip->partition_info[0].init_parameter,
		chip->partition_info[0].chip_erase_parameter,
		chip->partition_info[0].sector_erase_parameter,
		chip->partition_info[0].program_parameter,
		chip->partition_info[0].read_parameter, 0,
		chip->partition_info[0].unprotect_parameter,
		chip->partition_info[0].protect_parameter, 0
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
