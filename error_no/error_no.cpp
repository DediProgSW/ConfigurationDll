// error_no.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include <vector>
#include "error_no.h"
#include "../common/assist.h"

struct slot_err {
	int		usb_index;
	int		site_nr;
	int		err_code;
};

static HANDLE                          hMutex;
std::vector<struct slot_err> err_code;


int err_init()
{

        hMutex = CreateMutex(NULL, FALSE, NULL);



        return 0;
}

int set_last_err(int usb_index, int site_nr, int err)
{
	int i;
	struct slot_err		slot_status;


        WaitForSingleObject(hMutex, 20000);        /* timeout 10s */

	for (i = 0; i < (int)err_code.size(); i++) {
		if (err_code.at(i).usb_index == usb_index && err_code.at(i).site_nr == site_nr)
			break;
	}

	if (i < (int)err_code.size()) {
		err_code.at(i).err_code = err;
	} else {
		slot_status.err_code = err;
		slot_status.site_nr = site_nr;
		slot_status.usb_index = usb_index;
		err_code.push_back(slot_status);
	}

        ReleaseMutex(hMutex);


	return err;
}

int get_last_err(int usb_index, int site_nr)
{
	int i;

	for (i = 0; i < (int)err_code.size(); i++) {
		if (err_code.at(i).usb_index == usb_index && err_code.at(i).site_nr == site_nr)
			break;
	}

	if (i < (int)err_code.size())
		return err_code.at(i).err_code;
	else
		return 0;
}


const char *err_str_ascii(int err)
{
	const static char *err_tbl[] = {
		//#define	E_OK					E_BASIC_BASE(0)
		"No Error",
		//"reserved",
		//#define	E_CONTACT_FAIL				E_BASIC_BASE(1)
		"Contact Test Fail",
		//#define	E_INVALID_IC				E_BASIC_BASE(2)
		"Invalid chip ID",
		//#define	E_INVALID_SOCKET			E_BASIC_BASE(3)
		"Invalid Socket",
		//#define	E_INVALID_DEVICE			E_BASIC_BASE(4)
		"Invalid Device",
		//#define	E_DEVICE_PLUG_OUT			E_BASIC_BASE(5)
		"Devcie has plugged out",
		//#define	E_ALLOC_MEMORY_FAIL			E_BASIC_BASE(6)
		"Alloc memory fail",
		//#define	E_NET_ERROR				E_BASIC_BASE(7)
		"Net socket error",
		//#define	E_NO_DEVICE				E_BASIC_BASE(8)
		"No Device",
		//#define	E_FILE_NO_EXIST				E_BASIC_BASE(9)
		"File is not exist",
		//#define	E_CONFIG_POWER_FAIL			E_BASIC_BASE(10)
		"Config power fail",
		//#define	E_RESET_BUS_FAIL			E_BASIC_BASE(11)
		"Erase bus fail",
		//#define E_RAM_FUNC_RUNNING				E_BASIC_BASE(12)
		"Ram function is still running",
		//#define	E_XML_GRAMMAR_FAIL			E_BASIC_BASE(13)
		"XML grammar is wrong",
		//#define	E_CHIP_PROGRAM				E_BASIC_BASE(14)
		"Program fail",
		//#define	E_CHIP_NOT_BLANK			E_BASIC_BASE(15)
		"Chip is not blank",
		//#define	E_CHIP_VERIFY				E_BASIC_BASE(16)
		"Verify error",
		//#define	E_CHIP_ERASE				E_BASIC_BASE(17)
		"Erase error",
		//#define	E_CURR_OP_RUNNING			E_BASIC_BASE(18)
		"Current operate is running",
		//#define	E_LOAD_RAM_FUNC_FAIL			E_BASIC_BASE(19)
		"load ram func dll fail",
		//#define	E_USB_COMMUNICATION_FAIL		E_BASIC_BASE(20)
		"usb communication fail",
		//#define	E_DOWN_BASEUNIT_FAIL			E_BASIC_BASE(21)
		"down load base unit fail",
		//#define	E_DOWN_CARTRIDGE_FAIL			E_BASIC_BASE(22)
		"down load cartridge fail",
		//#define	E_DOWN_VECTOR_FAIL			E_BASIC_BASE(23)
		"down vector fail",
		//#define	E_DWON_RAMFUNC_FAIL			E_BASIC_BASE(24)
		"down ram function fail",
		//#define	E_NOT_SUPPORT_DEV_TYPE			E_BASIC_BASE(25)
		"not support device type",
		//#define	E_BASEUNIT_FILE_NOT_EXIST		E_BASIC_BASE(26)
		"base unit file is not exist",
		//#define	E_CARTRIDGE_FILE_NOT_EXIST		E_BASIC_BASE(27)
		"cartridge file is not exist",
		//#define	E_VECTOR_FILE_NOT_EXIST			E_BASIC_BASE(28)
		"vector file is not exist",
		//#define	E_RAM_FILE_NOT_EXIST			E_BASIC_BASE(29)
		"ram function file is not exist",
		//#define	E_DOWN_ICINFO_FAIL			E_BASIC_BASE(30)
		"down ic information to ramfunction fail",
		//#define	E_LOAD_FILE_SIZE_LARGER			E_BASIC_BASE(31)
		"load file is more larger",
		//#define	E_CREATE_THREAD_ERR			E_BASIC_BASE(32)
		"create the thread fail",
		//#define	E_GET_DERIVE_TBL_FAIL			E_BASIC_BASE(33)
		"get device tbl fail",
		//#define	E_HAVENOT_LOG_IN			E_BASIC_BASE(34)
		"have not log in",
                //#define	E_DOWN_IC_INFO_ERR			E_BASIC_BASE(35)
                "download ic information error",
                //#define E_CONFIG_FILE_NOT_EXIST                       E_BASIC_BASE(36)
                "config file not founded",
                //#define E_NEED_NO_SAU                                 E_BASIC_BASE(37)
                "need no spare area usage",
                //#define E_NEED_NO_BBM                                 E_BASIC_BASE(38)
                "need no bad block management",
                //#define E_TIME_OUT                                    E_BASIC_BASE(39)
                "operation timeout",
                //#define E_SELECT_NO_CHIP                              E_BASIC_BASE(40)
                "select no chip",
                //#define E_NO_LOAD_FILE                                E_BASIC_BASE(41)
                "no select file",
                //#define E_OUT_OF_COUNTER                              E_BASIC_BASE(42)
                "out of the counter",
                //#define E_INIT_FAIL                             E_BASIC_BASE(43)
                "initilalize fail",
                //NULL
                "",
                //#define E_NO_IC_IN_SOCKET                       E_BASIC_BASE(45)
                "no ic in socket",
                //#define E_CRC_FAIL                              E_BASIC_BASE(46)
                "crc cacluate fail",
                //#define E_NRE_OPERATE_FAIL                      E_BASIC_BASE(47)
                "nre operate faile",
                //#define E_UNLOCK_FAIL                           E_BASIC_BASE(48)
                "unlock fail",
                //#define E_LOCK_FAIL                             E_BASIC_BASE(49)
                "lock fail",
                //#define E_IGNORE_START_SINGLE                   E_BASIC_BASE(50)
                "ignore the start single",
                //#define E_PRJ_TERMINATED                        E_BASIC_BASE(51)
                "the project has be terminated",
                //#define E_OVER_CURRENT                          E_BASIC_BASE(52)
                "over current",
                //#define E_TOO_MANY_BAD_BLOCK                    E_BASIC_BASE(53)
                "to many bad block",
                //#define E_INVALID_STATUS                        E_BASIC_BASE(54)
                "invalid status",
                //#define E_INVALID_PARAM                         E_BASIC_BASE(55)
                "invalid param",
                //#define E_PRJ_RUNNING                           E_BASIC_BASE(56)
                "project is running",
                //#define E_NO_BATCH_SETTING                      E_BASIC_BASE(57)
                "no batch setting",
                //#define E_CREATE_PRJ_FAIL                       E_BASIC_BASE(58)
                "can't create project",
                //#define E_FILE_ANALYSE_FILE                     E_BASIC_BASE(59)
                "file analyse fail",
                //#define E_NOT_READ_CHIP                         E_BASIC_BASE(60)
                "have not read memory",
                //#define E_NO_SELECT_PRJ                         E_BASIC_BASE(61)
                "no select project",
                //#define E_NO_RUN_PRJ                            E_BASIC_BASE(62)
                "no run project",
                //#define E_NO_REG_SET                            E_BASIC_BASE(63)
                "no set the register value",
                //#define E_PROHIBIT_ERASE                        E_BASIC_BASE(64)
                "erase is prohibitted",
                //#define E_PROHIBIT_BLANK                        E_BASIC_BASE(65)
                "blank is prohibitted",
                //#define E_PROHIBIT_PROGRAM                      E_BASIC_BASE(66)
                "program is prohibitted",
                //#define E_PROHIBIT_VERIFY                       E_BASIC_BASE(67)
                "verify is prohibitted",
                //#define E_NO_OPERATION_DLL                      E_BASIC_BASE(68)
                "can't find operation dll",
                //#define E_USB_INIT_FALI                         E_BASIC_BASE(69)
                "usb initialize fail",
                //#define E_CREATE_FILE_FAIL                      E_BASIC_BASE(70)
                "create file fail",
                //#define E_NO_STORAGE_SPACE                      E_BASIC_BASE(71)
                "no enough storage space",
                //#define E_NO_SDCARD                             E_BASIC_BASE(72)
                "sdcard not plug in",
                //#define E_NO_SOCKET                             E_BASIC_BASE(73)
                "no socket",
                //#define E_OUT_OF_USR_COUNTER                    E_BASIC_BASE(74)
                "out of user counter",
                //#define E_INIT_TIMEOUT                          E_BASIC_BASE(75)
                "Initlize Timeout",
                //#define E_ERASE_TIMEOUT                         E_BASIC_BASE(76)
                "Erase Timeout",
                //#define E_BLANKCHECK_TIMEOUT                    E_BASIC_BASE(77)
                "BlankCheck Timeout",
                //#define E_PROGRAM_TIMEOUT                       E_BASIC_BASE(78)
                "Program Timeout",
                //#define E_VERIFY_TIMEOUT                        E_BASIC_BASE(79)
                "Verify Timeout",
                //#define E_NRE_TIMEOUT                           E_BASIC_BASE(80)
                "NRE Timeout",
                //#define E_UNLOCK_TIMEOUT                        E_BASIC_BASE(81)
                "Unlock Timeout",
                //#define E_LOCK_TIMEOUT                          E_BASIC_BASE(82)
                "Lock Timeout",
                //#define E_INVALID_SN                            E_BASIC_BASE(83)
                "invalid serial no",
                //#define E_CONFLICT_SN                           E_BASIC_BASE(84)
                "Serial no conflict",
                //#define E_PROG_ORDER_REBUILD                    E_BASIC_BASE(85)
                "prog rebuild order",
		//#define E_INVALID_PRJ                           E_BASIC_BASE(86)
		"invalid project",
		//#define E_FIND_CHIP_FAIL                        E_BASIC_BASE(87)
		"find chip fail",
		//#define E_READ_FILE_FAIL                        E_BASIC_BASE(88)
		"read file fail",
		//#define	E_CHIP_PROPERTY_ERR		  E_BASIC_BASE(89)
		"chip property error",
		//#define	E_NO_OP_PARAM			  E_BASIC_BASE(90)
		"no operation param",
		//#define	E_NO_IMG_PARAM			  E_BASIC_BASE(91)
		"no img param",
		//#define	E_OP_CANCELED			  E_BASIC_BASE(92)
		"Operation Canceled",
		//#define E_LOAD_OPERATIONDLL_FAIL    E_BASIC_BASE(93)
		"Load Operation DLL fail",
		//#define	E_LOAD_CHECKSUMDLL_FAIL			E_BASIC_BASE(94)
		"Load Checksum DLL fail",
		//#define	E_LOAD_CRCDLL_FAIL			E_BASIC_BASE(95)
		"Load CRC DLL fail",
		//#define	E_LOAD_IMAGETODUDLL_FAIL		E_BASIC_BASE(96)
		"Load img to du fail",
		//#define E_LOAD_FILEFORMATDLL_FAIL		E_BASIC_BASE(97)
		"load file format dll fail",
		//#define	E_LOAD_ECCDLL_FAIL			E_BASIC_BASE(98)
		"load file ecc dll fail",
		//#define E_WRITE_FILE_FAIL                E_BASIC_BASE(99)
		"Write file fail",
		//#define E_OPEN_FILE_FAIL                  E_BASIC_BASE(100)
		"Open file fail",
                //#define E_NEED_NEW_FW                           E_BASIC_BASE(101)
                "Need newer firmware",
                //#define E_SERVER_NOT_RDY                        E_BASIC_BASE(102)
                "Server Is not ready",
                //#define E_UNEXPECTED_ERR                        E_BASIC_BASE(103)
                "unexpected error",
		};

	assert(err >= 0);
	assert(err < ARRAY_SIZE(err_tbl));

	return err_tbl[err];
}

const wchar_t *err_str_unicode(int err)
{
		const static wchar_t *err_tbl[] = {
		//#define	E_OK					E_BASIC_BASE(0)
		L"No Error",
		//L"reserved",
		//#define	E_CONTACT_FAIL				E_BASIC_BASE(1)
		L"Contact Test Fail",
		//#define	E_INVALID_IC				E_BASIC_BASE(2)
		L"Invalid chip ID",
		//#define	E_INVALID_SOCKET			E_BASIC_BASE(3)
		L"Invalid Socket",
		//#define	E_INVALID_DEVICE			E_BASIC_BASE(4)
		L"Invalid Device",
		//#define	E_DEVICE_PLUG_OUT			E_BASIC_BASE(5)
		L"Devcie has plugged out",
		//#define	E_ALLOC_MEMORY_FAIL			E_BASIC_BASE(6)
		L"Alloc memory fail",
		//#define	E_NET_ERROR				E_BASIC_BASE(7)
		L"Net socket error",
		//#define	E_NO_DEVICE				E_BASIC_BASE(8)
		L"No Device",
		//#define	E_FILE_NO_EXIST				E_BASIC_BASE(9)
		L"File is not exist",
		//#define	E_CONFIG_POWER_FAIL			E_BASIC_BASE(10)
		L"Config power fail",
		//#define	E_RESET_BUS_FAIL			E_BASIC_BASE(11)
		L"Erase bus fail",
		//#define E_RAM_FUNC_RUNNING				E_BASIC_BASE(12)
		L"Ram function is still running",
		//#define	E_XML_GRAMMAR_FAIL			E_BASIC_BASE(13)
		L"XML grammar is wrong",
		//#define	E_CHIP_PROGRAM				E_BASIC_BASE(14)
		L"Program fail",
		//#define	E_CHIP_NOT_BLANK			E_BASIC_BASE(15)
		L"Chip is not blank",
		//#define	E_CHIP_VERIFY				E_BASIC_BASE(16)
		L"Verify error",
		//#define	E_CHIP_ERASE				E_BASIC_BASE(17)
		L"Erase error",
		//#define	E_CURR_OP_RUNNING			E_BASIC_BASE(18)
		L"Current operate is running",
		//#define	E_LOAD_RAM_FUNC_FAIL			E_BASIC_BASE(19)
		L"load ram func dll fail",
		//#define	E_USB_COMMUNICATION_FAIL		E_BASIC_BASE(20)
		L"usb communication fail",
		//#define	E_DOWN_BASEUNIT_FAIL			E_BASIC_BASE(21)
		L"down load base unit fail",
		//#define	E_DOWN_CARTRIDGE_FAIL			E_BASIC_BASE(22)
		L"down load cartridge fail",
		//#define	E_DOWN_VECTOR_FAIL			E_BASIC_BASE(22)
		L"down vector fail",
		//#define	E_DWON_RAMFUNC_FAIL			E_BASIC_BASE(24)
		L"down ram function fail",
		//#define	E_NOT_SUPPORT_DEV_TYPE			E_BASIC_BASE(25)
		L"not support device type",
		//#define	E_BASEUNIT_FILE_NOT_EXIST		E_BASIC_BASE(26)
		L"base unit file is not exist",
		//#define	E_CARTRIDGE_FILE_NOT_EXIST		E_BASIC_BASE(27)
		L"cartridge file is not exist",
		//#define	E_VECTOR_FILE_NOT_EXIST			E_BASIC_BASE(28)
		L"vector file is not exist",
		//#define	E_RAM_FILE_NOT_EXIST			E_BASIC_BASE(29)
		L"ram function file is not exist",
		//#define	E_DOWN_ICINFO_FAIL			E_BASIC_BASE(30)
		L"down ic information to ramfunction fail",
		//#define	E_LOAD_FILE_SIZE_LARGER			E_BASIC_BASE(31)
		L"load file is more larger",
		//#define	E_CREATE_THREAD_ERR			E_BASIC_BASE(32)
		L"create the thread fail",
		//#define	E_GET_DERIVE_TBL_FAIL			E_BASIC_BASE(33)
		L"get device tbl fail",
		//#define	E_HAVENOT_LOG_IN			E_BASIC_BASE(34)
		L"have not log in",
                //#define	E_DOWN_IC_INFO_ERR			E_BASIC_BASE(35)
                L"download ic information error",
                //#define E_CONFIG_FILE_NOT_EXIST                       E_BASIC_BASE(36)
                L"config file not founded",
                //#define E_NEED_NO_SAU                                 E_BASIC_BASE(37)
                L"need no spare area usage",
                //#define E_NEED_NO_BBM                                 E_BASIC_BASE(38)
                L"need no bad block management",
                //#define E_TIME_OUT                                    E_BASIC_BASE(39)
                L"operation timeout",
                //#define E_SELECT_NO_CHIP                              E_BASIC_BASE(40)
                L"select no chip",
                //#define E_NO_LOAD_FILE                                E_BASIC_BASE(41)
                L"no select file",
                //#define E_OUT_OF_COUNTER                              E_BASIC_BASE(42)
                L"out of the counter",
                //#define E_INIT_FAIL                             E_BASIC_BASE(43)
                L"initilalize fail",
                //NULL
                L"",
                //#define E_NO_IC_IN_SOCKET                       E_BASIC_BASE(45)
                L"no ic in socket",
                //#define E_CRC_FAIL                              E_BASIC_BASE(46)
                L"crc cacluate fail",
                //#define E_NRE_OPERATE_FAIL                      E_BASIC_BASE(47)
                L"nre operate faile",
                //#define E_UNLOCK_FAIL                           E_BASIC_BASE(48)
                L"unlock fail",
                //#define E_LOCK_FAIL                             E_BASIC_BASE(49)
                L"lock fail",
                //#define E_IGNORE_START_SINGLE                   E_BASIC_BASE(50)
                L"ignore the start single",
                //#define E_PRJ_TERMINATED                        E_BASIC_BASE(51)
                L"the project has be terminated",
                //#define E_OVER_CURRENT                          E_BASIC_BASE(52)
                L"over current",
                //#define E_TOO_MANY_BAD_BLOCK                    E_BASIC_BASE(53)
                L"to many bad block",
                //#define E_INVALID_STATUS                        E_BASIC_BASE(54)
                L"invalid status",
                //#define E_INVALID_PARAM                         E_BASIC_BASE(55)
                L"invalid param",
                //#define E_PRJ_RUNNING                           E_BASIC_BASE(56)
                L"project is running",
                //#define E_NO_BATCH_SETTING                      E_BASIC_BASE(57)
                L"no batch setting",
                //#define E_CREATE_PRJ_FAIL                       E_BASIC_BASE(58)
                L"can't create project",
                //#define E_FILE_ANALYSE_FILE                     E_BASIC_BASE(59)
                L"file analyse fail",
                //#define E_NOT_READ_CHIP                         E_BASIC_BASE(60)
                L"have not read memory",
                //#define E_NO_SELECT_PRJ                         E_BASIC_BASE(61)
                L"no select project",
                //#define E_NO_RUN_PRJ                            E_BASIC_BASE(62)
                L"no run project",
                 //#define E_NO_REG_SET                           E_BASIC_BASE(63)
                L"no set the register value",
                //#define E_PROHIBIT_ERASE                        E_BASIC_BASE(64)
                L"erase is prohibitted",
                //#define E_PROHIBIT_BLANK                        E_BASIC_BASE(65)
                L"blank is prohibitted",
                //#define E_PROHIBIT_PROGRAM                      E_BASIC_BASE(66)
                L"program is prohibitted",
                //#define E_PROHIBIT_VERIFY                       E_BASIC_BASE(67)
                L"verify is prohibitted",
                //#define E_NO_OPERATION_DLL                      E_BASIC_BASE(68)
                L"can't find operation dll",
                //#define E_USB_INIT_FALI                         E_BASIC_BASE(69)
                L"usb initialize fail",
                //#define E_CREATE_FILE_FAIL                      E_BASIC_BASE(70)
                L"create file fail",
                //#define E_NO_STORAGE_SPACE                      E_BASIC_BASE(71)
                L"no enough storage space",
                //#define E_NO_SDCARD                             E_BASIC_BASE(72)
                L"sdcard not plug in",
                //#define E_NO_SOCKET                             E_BASIC_BASE(73)
                L"no socket",
                //#define E_OUT_OF_USR_COUNTER                    E_BASIC_BASE(74)
                L"out of user counter",
                //#define E_INIT_TIMEOUT                          E_BASIC_BASE(75)
                L"Initlize Timeout",
                //#define E_ERASE_TIMEOUT                         E_BASIC_BASE(76)
                L"Erase Timeout",
                //#define E_BLANKCHECK_TIMEOUT                    E_BASIC_BASE(77)
                L"BlankCheck Timeout",
                //#define E_PROGRAM_TIMEOUT                       E_BASIC_BASE(78)
                L"Program Timeout",
                //#define E_VERIFY_TIMEOUT                        E_BASIC_BASE(79)
                L"Verify Timeout",
                //#define E_NRE_TIMEOUT                           E_BASIC_BASE(80)
                L"NRE Timeout",
                //#define E_UNLOCK_TIMEOUT                        E_BASIC_BASE(81)
                L"Unlock Timeout",
                //#define E_LOCK_TIMEOUT                          E_BASIC_BASE(82)
                L"Lock Timeout",
                //#define E_INVALID_SN                            E_BASIC_BASE(83)
                L"invalid serial no",
                //#define E_CONFLICT_SN                           E_BASIC_BASE(84)
                L"Serial no conflict",
                //#define E_PROG_ORDER_REBUILD                    E_BASIC_BASE(85)
                L"prog rebuild order",
		//#define E_INVALID_PRJ                           E_BASIC_BASE(86)
		L"invalid project",
		//#define E_FIND_CHIP_FAIL                        E_BASIC_BASE(87)
		L"find chip fail",
		//#define E_READ_FILE_FAIL                        E_BASIC_BASE(88)
		L"read file fail",
		//#define	E_CHIP_PROPERTY_ERR		  E_BASIC_BASE(89)
		L"chip property error",
		//#define	E_NO_OP_PARAM			  E_BASIC_BASE(90)
		L"no operation param",
		//#define	E_NO_IMG_PARAM			  E_BASIC_BASE(91)
		L"no img param",
		//#define	E_OP_CANCELED			  E_BASIC_BASE(92)
		L"Operation Canceled",
		//#define E_LOAD_OPERATIONDLL_FAIL    E_BASIC_BASE(93)
		L"Load Operation DLL fail",
		//#define	E_LOAD_CHECKSUMDLL_FAIL			E_BASIC_BASE(94)
		L"Load Checksum DLL fail",
		//#define	E_LOAD_CRCDLL_FAIL			E_BASIC_BASE(95)
		L"Load CRC DLL fail",
		//#define	E_LOAD_IMAGETODUDLL_FAIL		E_BASIC_BASE(96)
		L"Load img to du fail",
		//#define E_LOAD_FILEFORMATDLL_FAIL		E_BASIC_BASE(97)
		L"load file format dll fail",
		//#define	E_LOAD_ECCDLL_FAIL			E_BASIC_BASE(98)
		L"load file ecc dll fail",
                //#define E_WRITE_FILE_FAIL                 E_BASIC_BASE(99)
		L"Write file fail",
                //#define E_OPEN_FILE_FAIL                  E_BASIC_BASE(100)
		L"Open file fail",
                //#define E_NEED_NEW_FW                           E_BASIC_BASE(101)
                L"Need newer firmware",
                //#define E_SERVER_NOT_RDY                        E_BASIC_BASE(102)
                L"Server Is not ready",
                //#define E_UNEXPECTED_ERR                        E_BASIC_BASE(103)
                L"unexpected error",
	};

	assert(err >= 0);
	assert(err < ARRAY_SIZE(err_tbl));

	return err_tbl[err];
}