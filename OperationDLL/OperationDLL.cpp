// OperationDLL.cpp : Defines the exported functions for the DLL application.
// OperationDLL Template
//

#include "stdafx.h"
#include <assert.h>
#include "../common/prog_info.h"
#include "../common/OperationDLL.h"

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
        struct _st_icinfo *info;
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
	};
        
        if (!buff)
                return sizeof(struct _st_icinfo);
        if (size < sizeof(struct _st_icinfo))
                return sizeof(struct _st_icinfo);

        info = (struct _st_icinfo *)buff;
               
        info->uiBusWidth = (unsigned int)chip->bus_width;
        info->uiPageSize = (unsigned int)chip->partition_info[0].page_size_in_byte;
        info->uiSectorSize = (unsigned int)chip->partition_info[0].sector_size_in_byte;
        info->uiChipSize = (unsigned int)chip->partition_info[0].size_in_bytes;
        memcpy(info->ausId, chip->flash_id, sizeof(info->ausId));
        info->usIdLength = (unsigned int)chip->flash_id_nr;
        memcpy(info->ausPinMask, chip->chip_contact[0].pin_mask, sizeof(info->ausPinMask));
        info->usPinLength = 0;
        info->uiMaxVoltage = (unsigned int)chip->chip_contact[0].pin_max_volt;
        info->uiMinVoltage = (unsigned int)chip->chip_contact[0].pin_min_volt;

        info->uiInitCommand = (unsigned int)chip->partition_info[0].init_parameter;
        info->uiChipEraseCommand = (unsigned int)chip->partition_info[0].chip_erase_parameter;
        info->uiSectorEraseCommand = (unsigned int)chip->partition_info[0].sector_erase_parameter;
        info->uiProgramCommand = (unsigned int)chip->partition_info[0].program_parameter;
        info->uiReadCommand = (unsigned int)chip->partition_info[0].read_parameter;

	return 0;
}

/* @chip the  chip attribute from config-file function
 * @reg_func  direct operate the FPGA register function
 * @sram_func direct operate the Firmware Sram function
 * @fifo_func direct operate the fifo function
 * @func_param the three function param
 */
extern "C" __declspec(dllexport)
int prepare_init(
                const struct chip_info_c       *chip,
                register_func_t                 *reg_func,
		sram_func_t                     *sram_func,
		fifo_func_t                     *fifo_func,
		const void                      *func_param
		)
{
	assert(chip);
        assert(reg_func);
        assert(sram_func);
        assert(fifo_func);

        return 0;
}

extern "C" __declspec(dllexport)
int prepare_erase(
                const struct chip_info_c        *chip,
                register_func_t                 *reg_func,
		sram_func_t                     *sram_func,
		fifo_func_t                     *fifo_func,
		const void                      *func_param
		)
{
	assert(chip);
        assert(reg_func);
        assert(sram_func);
        assert(fifo_func);


        return 0;
}

extern "C" __declspec(dllexport)
int prepare_blankcheck(
                const struct chip_info_c        *chip,
                register_func_t                 *reg_func,
		sram_func_t                     *sram_func,
		fifo_func_t                     *fifo_func,
		const void                      *func_param
		)
{
	assert(chip);
        assert(reg_func);
        assert(sram_func);
        assert(fifo_func);


        return 0;
}

extern "C" __declspec(dllexport)
int prepare_program(
                const struct chip_info_c        *chip,
                register_func_t                 *reg_func,
		sram_func_t                     *sram_func,
		fifo_func_t                     *fifo_func,
		const void                      *func_param
		)
{
        assert(chip);
        assert(reg_func);
        assert(sram_func);
        assert(fifo_func);


	return 0;
}

extern "C" __declspec(dllexport)
int prepare_verify(
                const struct chip_info_c        *chip,
                register_func_t                 *reg_func,
		sram_func_t                     *sram_func,
		fifo_func_t                     *fifo_func,
		const void                      *func_param
		)
{
	assert(chip);
        assert(reg_func);
        assert(sram_func);
        assert(fifo_func);

        return 0;
}

