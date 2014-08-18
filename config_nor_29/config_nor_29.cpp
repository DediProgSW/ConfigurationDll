// config_nor_29.cpp : Defines the exported functions for the DLL application.
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
		//16, 0x20, 0x2000, 0x400000, 
		//{ 0, 0, 0, 0 },0x08, 
		//{ 0, 0, 0, 0 }, 0, 2200, 300,
		//0, 0, 1, 1, 0,
	struct _st_icinfo {
		unsigned int uiBusWidth;
		unsigned int uiPageSize;
		unsigned int uiSectorSize;
		unsigned int uiChipSize;
		unsigned short ausId[4];
		unsigned short usIdLength;
		unsigned short ausPinMask[4];
		unsigned short usPinLength;
		unsigned int uiMaxVoltage;
		unsigned int uiMinVoltage;

		unsigned int uiInitCommand;
		unsigned int uiChipEraseCommand;
		unsigned int uiSectorEraseCommand;
		unsigned int uiProgramCommand;
		unsigned int uiReadCommand;
	} info = {
		(unsigned int)chip->bus_width, 
		(unsigned int)chip->partition_info[0].page_size_in_byte, 
		(unsigned int)chip->partition_info[0].sector_size_in_byte,
		(unsigned int)chip->partition_info[0].size_in_bytes,
		{0, 0, 0, 0}, (unsigned int)chip->flash_id_nr, 
		{0, 0, 0, 0}, 0,
		(unsigned int)chip->chip_contact[0].pin_max_volt,
		(unsigned int)chip->chip_contact[0].pin_min_volt, 
		(unsigned int)chip->partition_info[0].init_parameter,
		(unsigned int)chip->partition_info[0].chip_erase_parameter,
		(unsigned int)chip->partition_info[0].sector_erase_parameter,
		(unsigned int)chip->partition_info[0].program_parameter,
		(unsigned int)chip->partition_info[0].read_parameter,
	};

	memcpy(info.ausId, chip->flash_id, chip->flash_id_nr);

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
	struct _st_icinfo {
		unsigned int uiBusWidth;
		unsigned int uiPageSize;
		unsigned int uiSectorSize;
		unsigned int uiChipSize;
		unsigned short ausId[4];
		unsigned short usIdLength;
		unsigned short ausPinMask[4];
		unsigned short usPinLength;
		unsigned int uiMaxVoltage;
		unsigned int uiMinVoltage;

		unsigned int uiInitCommand;
		unsigned int uiChipEraseCommand;
		unsigned int uiSectorEraseCommand;
		unsigned int uiProgramCommand;
		unsigned int uiReadCommand;
	} info = {
		(unsigned int)chip->bus_width, 
		(unsigned int)chip->partition_info[0].page_size_in_byte, 
		(unsigned int)chip->partition_info[0].sector_size_in_byte,
		(unsigned int)chip->partition_info[0].size_in_bytes,
		{0, 0, 0, 0}, (unsigned int)chip->flash_id_nr, 
		{0, 0, 0, 0}, 0,
		(unsigned int)chip->chip_contact[0].pin_max_volt,
		(unsigned int)chip->chip_contact[0].pin_min_volt, 
		(unsigned int)chip->partition_info[0].init_parameter,
		(unsigned int)chip->partition_info[0].chip_erase_parameter,
		(unsigned int)chip->partition_info[0].sector_erase_parameter,
		(unsigned int)chip->partition_info[0].program_parameter,
		(unsigned int)chip->partition_info[0].read_parameter,
	};

	memcpy(info.ausId, chip->flash_id, chip->flash_id_nr);     

        if (!buff)
                return sizeof(struct _st_icinfo);
        if (size < sizeof(struct _st_icinfo))
                return sizeof(struct _st_icinfo);
 
        assert(buff);
        memcpy(buff, &info, sizeof(info));

        return 0;
}
