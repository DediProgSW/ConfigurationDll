// OperationDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include <vector>
#include "../vm_mgr/vm_mgr.h"
#include "../common/loadfile_info.h"


extern "C" __declspec(dllexport)
int du_to_reg(
const struct chip_info_c                *chip,                  /* chip info structure */
std::vector<struct loadfile_du>         &du_header,             /* du header */
vm_ptr_t                                buff,
loadfile_item                           **reg,
unsigned long                           *chip_checksum,         /* loadfile --> chipchecksum */
std::vector<struct loadfile_fcs>        &file_checksum,          /* file checksum */
const struct loadfile_funcs             *funcs
) {
	assert(chip);
	assert(reg);
	assert(chip_checksum);
	
	/* 如果没有Register partion，ICSetting Dilog, 默认Load file要更改buffer，就把下面的示例代码写道这里 */

	printf("\\du_to_reg _indicaterds\n");

	return 0;
}

extern "C" __declspec(dllexport)
int final_du(
const struct chip_info_c                *chip,
std::vector<struct loadfile_du>         &du_header,
vm_ptr_t                                buff,
loadfile_item                           **reg,
unsigned long                           *chip_checksum,
std::vector<struct loadfile_fcs>        &file_checksum,
const struct loadfile_funcs             *funcs) {
	assert(chip);
	assert(reg);
	assert(chip_checksum);

	/*确保要修改的buffer 在范围内*/
	if (du_header.size() <= 0)
		return 0;
	if (du_header.at(0).ptn_addr > 0x0000 || du_header.at(0).ptn_addr + du_header.at(0).program_length < 0X8310)
		return 0;

	/*示例*/
	if ((*reg)->buff[0]) { /*reg-> buff 是ICSetting Dilog 传递过来的 */
		unsigned int tempBufferAddr = 0x0000 - du_header.at(0).ptn_addr;
		unsigned int tempBufferLen = du_header.at(0).image_length;
		unsigned char *tempBuffer = (unsigned char *)malloc(tempBufferLen * sizeof(char));
		if (tempBuffer == 0)
			return 0;
		/*读取buffer*/
		vm_read(tempBuffer, buff + du_header.at(0).data_offset_in_buff, tempBufferLen);

		/* 转换Buffer 修改长度*/

		/*0x8300~0x8303 and 0x830D~0x8310 = 0x7FF5~0x7FF8*/
		for (unsigned int i = 0x00; i < 4; i++) {
			tempBuffer[tempBufferAddr + 0x8300 + i] = tempBuffer[tempBufferAddr + 0x7F55 + i];
			tempBuffer[tempBufferAddr + 0x830D + i] = tempBuffer[tempBufferAddr + 0x7F55 + i];
		}
		/*写回去*/
		vm_write(buff + du_header.at(0).data_offset_in_buff, tempBuffer, tempBufferLen);

		/* 计算 check sum，注意长度 */
		du_header.at(0).data_crc_value = funcs->du_crc(tempBuffer, du_header.at(0).program_length);
		du_header.at(0).data_checksum_value = funcs->du_checksum(tempBuffer, du_header.at(0).program_length);
		du_header.at(0).du_crc = funcs->du_crc(tempBuffer, du_header.at(0).image_length);
		free(tempBuffer);
	}
	printf("\\final_du\n");
	return 0;
}