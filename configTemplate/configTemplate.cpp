// configTemplate.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "configTemplate.h"
#include <assert.h>
#include "../common/prog_info.h"


// This is an example of an exported variable
extern "C" __declspec(dllexport)
int ramfunc_config(
	const struct chip_info_c *chip,
	unsigned char *buff,
	unsigned long size
)
{
#pragma pack(1)
	typedef struct _st_icinfo {
		unsigned char aucId[8];
		unsigned int  uiIdLength;
		unsigned int uiPageSizeInByte;
		unsigned int uiSectorSizeByte;
		unsigned int uiBlockSizeByte;
		unsigned int uiChipSizeInPage;

		unsigned int uiReadIdParameter;
		unsigned int uiEraseParameter;
		unsigned int uiProgramParameter;
		unsigned int uiReadParameter;
		unsigned int uiUnlockParameter;
		unsigned int uiLockParameter;
		unsigned int uiBusWidth;
	} TEMPLATE_IC_INFO;
#pragma pack()
	TEMPLATE_IC_INFO info = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		chip->flash_id_nr,
		chip->partition_info[0].page_size_in_byte,
		chip->partition_info[0].sector_size_in_byte,
		chip->partition_info[0].block_size_in_byte,
		chip->partition_info[0].size_in_bytes / chip->partition_info[0].page_size_in_byte,

		chip->partition_info[0].init_parameter,
		chip->partition_info[0].chip_erase_parameter,
		chip->partition_info[0].program_parameter,
		chip->partition_info[0].read_parameter,
		chip->partition_info[0].unprotect_parameter,
		chip->partition_info[0].protect_parameter,
		chip->bus_width
	};
	memcpy(&info.aucId, chip->flash_id, chip->flash_id_nr);

	if (!buff)
		return sizeof(struct _st_icinfo);
	if (size < sizeof(struct _st_icinfo))
		return sizeof(struct _st_icinfo);

	assert(buff);
	memcpy(buff, &info, sizeof(info));

	return 0;
}
