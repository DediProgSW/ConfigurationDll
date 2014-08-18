#ifndef _ERROR_NO_H
#define	_ERROR_NO_H

typedef enum {
	OP_ERR,
	BASIC_ERR,
} err_type_t;

#define	E_BASIC_BASE(a)				(a)

/* no error */
#define	E_OK					0
/* contact test fail */
#define	E_CONTACT_FAIL				E_BASIC_BASE(1)
/* the id of chip is incorrect */
#define	E_INVALID_IC				E_BASIC_BASE(2)
/* the encryption or counter is not avalible */
#define	E_INVALID_SOCKET			E_BASIC_BASE(3)
/* the device is not the in {PROG_MASTER_X4, PROG_MASTER_X8, PROG_STATR} */
#define	E_INVALID_DEVICE			E_BASIC_BASE(4)
/* the device is plug out from the pc */
#define	E_DEVICE_PLUG_OUT			E_BASIC_BASE(5)
/* malloc memory failed */
#define	E_ALLOC_MEMORY_FAIL			E_BASIC_BASE(6)
/* the socket net occurs error */
#define	E_NET_ERROR				E_BASIC_BASE(7)
/* there is no programmer device */
#define	E_NO_DEVICE				E_BASIC_BASE(8)
/* file can't be finded */
#define	E_FILE_NO_EXIST				E_BASIC_BASE(9)
/* config power fail */
#define	E_CONFIG_POWER_FAIL			E_BASIC_BASE(10)
/* reset the bus fail */
#define	E_RESET_BUS_FAIL			E_BASIC_BASE(11)
/* ram function is still running */
#define E_RAM_FUNC_RUNNING			E_BASIC_BASE(12)
/* config file has Grammar mistakes */
#define	E_XML_GRAMMAR_FAIL			E_BASIC_BASE(13)
/* program error */
#define	E_CHIP_PROGRAM				E_BASIC_BASE(14)
/* the chip is not blank */
#define	E_CHIP_NOT_BLANK			E_BASIC_BASE(15)
/* verify error */
#define	E_CHIP_VERIFY				E_BASIC_BASE(16)
/* erase error */
#define	E_CHIP_ERASE				E_BASIC_BASE(17)
/* current operator is running */
#define	E_CURR_OP_RUNNING			E_BASIC_BASE(18)
/* can't load ram func dll */
#define	E_LOAD_RAM_FUNC_FAIL			E_BASIC_BASE(19)
/* usb communication fail */
#define	E_USB_COMMUNICATION_FAIL		E_BASIC_BASE(20)
/* down load base unit fail */
#define	E_DOWN_BASEUNIT_FAIL			E_BASIC_BASE(21)
/* down load cartridge fail */
#define	E_DOWN_CARTRIDGE_FAIL			E_BASIC_BASE(22)
/* down vector fail */
#define	E_DOWN_VECTOR_FAIL			E_BASIC_BASE(23)
/* down ram function fail */
#define E_DWON_RAMFUNC_FAIL			E_BASIC_BASE(24)
/* not support device type */
#define	E_NOT_SUPPORT_DEV_TYPE			E_BASIC_BASE(25)
/* can't open base unit file */
#define	E_BASEUNIT_FILE_NOT_EXIST		E_BASIC_BASE(26)
/* can't open cartridge file */
#define	E_CARTRIDGE_FILE_NOT_EXIST		E_BASIC_BASE(27)
/* can't open vector file */
#define	E_VECTOR_FILE_NOT_EXIST			E_BASIC_BASE(28)
/* can't open ram function file */
#define	E_RAM_FILE_NOT_EXIST			E_BASIC_BASE(29)
/* down ic information to ramfunction file */
#define	E_DOWN_ICINFO_FAIL			E_BASIC_BASE(30)
/* load file is more larger */
#define	E_LOAD_FILE_SIZE_LARGER			E_BASIC_BASE(31)
/* create the thread fail */
#define	E_CREATE_THREAD_ERR			E_BASIC_BASE(32)
/* get device tbl fail */
#define	E_GET_DERIVE_TBL_FAIL			E_BASIC_BASE(33)
/* have not log in */
#define	E_HAVENOT_LOG_IN			E_BASIC_BASE(34)
/* download ic information error */
#define	E_DOWN_IC_INFO_ERR			E_BASIC_BASE(35)
/* config file not founded */
#define E_CONFIG_FILE_NOT_EXIST                 E_BASIC_BASE(36)
/* it is not a nand flash, it don't need spare area usage */
#define E_NEED_NO_SAU                           E_BASIC_BASE(37)
/* it is not a nand flash, it does not need bbm method */
#define E_NEED_NO_BBM                           E_BASIC_BASE(38)
/* operation timeout */
#define E_TIME_OUT                              E_BASIC_BASE(39)
/* have not selected the chip */
#define E_SELECT_NO_CHIP                        E_BASIC_BASE(40)
/* have not load the file */
#define E_NO_LOAD_FILE                          E_BASIC_BASE(41)
/* out of the counter */
#define E_OUT_OF_SKT_COUNTER                    E_BASIC_BASE(42)
/* initilalize fail */
#define E_INIT_FAIL                             E_BASIC_BASE(43)
/* no ic in socket */
#define E_NO_IC_IN_SOCKET                       E_BASIC_BASE(45)
/* crc cacluate fail*/
#define E_CRC_FAIL                              E_BASIC_BASE(46)
/* nre operate faile */
#define E_NRE_OPERATE_FAIL                      E_BASIC_BASE(47)
/* unlock fail */
#define E_UNLOCK_FAIL                           E_BASIC_BASE(48)
/* lock fail */
#define E_LOCK_FAIL                             E_BASIC_BASE(49)
/* ignore the start single */
#define E_IGNORE_START_SIGNAL                   E_BASIC_BASE(50)
/* the project has be terminated */
#define E_PRJ_TERMINATED                        E_BASIC_BASE(51)
/* over current */
#define E_OVER_CURRENT                          E_BASIC_BASE(52)
/* to many bad block */
#define E_TOO_MANY_BAD_BLOCK                    E_BASIC_BASE(53)
/* invalid status */
#define E_INVALID_STATUS                        E_BASIC_BASE(54)
/* invalid param */
#define E_INVALID_PARAM                         E_BASIC_BASE(55)
/* project is running */
#define E_PRJ_RUNNING                           E_BASIC_BASE(56)
/* no batch setting */
#define E_NO_BATCH_SETTING                      E_BASIC_BASE(57)
/* can't create project */
#define E_CREATE_PRJ_FAIL                       E_BASIC_BASE(58)
/* file analyse fail */
#define E_FILE_ANALYSE_FILE                     E_BASIC_BASE(59)
/* have not read memory */
#define E_NOT_READ_CHIP                         E_BASIC_BASE(60)
/* no select project */
#define E_NO_SELECT_PRJ                         E_BASIC_BASE(61)
/* no run project */
#define E_NO_RUN_PRJ                            E_BASIC_BASE(62)
/* no set register value */
#define E_NO_REG_SET                            E_BASIC_BASE(63)
/* erase prohibit */
#define E_PROHIBIT_ERASE                        E_BASIC_BASE(64)
/* blank prohibit */
#define E_PROHIBIT_BLANK                        E_BASIC_BASE(65)
/* program prohibit */
#define E_PROHIBIT_PROGRAM                      E_BASIC_BASE(66)
/* verify prohibit */
#define E_PROHIBIT_VERIFY                       E_BASIC_BASE(67)
/* can't find operation dll */
#define E_NO_OPERATION_DLL                      E_BASIC_BASE(68)
/* usb initialize fail */
#define E_USB_INIT_FALI                         E_BASIC_BASE(69)
/* create file fail */
#define E_CREATE_FILE_FAIL                      E_BASIC_BASE(70)
/* no enough storage space */
#define E_NO_STORAGE_SPACE                      E_BASIC_BASE(71)
/* sdcard not plug in */
#define E_NO_SDCARD                             E_BASIC_BASE(72)
/* no socket */
#define E_NO_SOCKET                             E_BASIC_BASE(73)
/* out of counter */
#define E_OUT_OF_USR_COUNTER                    E_BASIC_BASE(74)
/* Initlize Timeout */
#define E_INIT_TIMEOUT                          E_BASIC_BASE(75)
/* Erase Timeout */
#define E_ERASE_TIMEOUT                         E_BASIC_BASE(76)
/* BlankCheck Timeout */
#define E_BLANKCHECK_TIMEOUT                    E_BASIC_BASE(77)
/* Program Timeout */
#define E_PROGRAM_TIMEOUT                       E_BASIC_BASE(78)
/* Verify Timeout */
#define E_VERIFY_TIMEOUT                        E_BASIC_BASE(79)
/* NRE Timeout */
#define E_NRE_TIMEOUT                           E_BASIC_BASE(80)
/* Unlock Timeout */
#define E_UNLOCK_TIMEOUT                        E_BASIC_BASE(81)
/* Lock Timeout */
#define E_LOCK_TIMEOUT                          E_BASIC_BASE(82)
/* invalid serial no */
#define E_INVALID_SN                            E_BASIC_BASE(83)
/* Serial no conflict */
#define E_CONFLICT_SN                           E_BASIC_BASE(84)
/* prog rebuild order */
#define E_PROG_ORDER_REBUILD                    E_BASIC_BASE(85)
/* invalid project */
#define E_INVALID_PRJ                           E_BASIC_BASE(86)
/* find chip fail*/
#define E_FIND_CHIP_FAIL                        E_BASIC_BASE(87)
/* read file fail */
#define E_READ_FILE_FAIL                        E_BASIC_BASE(88)
/* chip property error */
#define	E_CHIP_PROPERTY_ERR			E_BASIC_BASE(89)
/* no operation param */
#define	E_NO_OP_PARAM				E_BASIC_BASE(90)
/* no img param */
#define	E_NO_IMG_PARAM				E_BASIC_BASE(91)
/* Operation Canceled */
#define	E_OP_CANCELED				E_BASIC_BASE(92)
/* Load Operation dll failed*/
#define E_LOAD_OPERATIONDLL_FAIL		E_BASIC_BASE(93)
/* load checksum dll fail */
#define	E_LOAD_CHECKSUMDLL_FAIL			E_BASIC_BASE(94)
/* load crc dll fail */
#define	E_LOAD_CRCDLL_FAIL			E_BASIC_BASE(95)
/* load img to du fail */
#define	E_LOAD_IMAGETODUDLL_FAIL		E_BASIC_BASE(96)
/* load file format dll fail */
#define E_LOAD_FILEFORMATDLL_FAIL		E_BASIC_BASE(97)
/* load file ecc dll fail */
#define	E_LOAD_ECCDLL_FAIL			E_BASIC_BASE(98)
/* write file fail */
#define E_WRITE_FILE_FAIL                       E_BASIC_BASE(99)
/* open file fail*/
#define E_OPEN_FILE_FAIL                        E_BASIC_BASE(100)
/* Need newer firmware */
#define E_NEED_NEW_FW                           E_BASIC_BASE(101)
/* Server Is not ready */
#define E_SERVER_NOT_RDY                        E_BASIC_BASE(102)
/* FW unexpected error */
#define E_UNEXPECTED_ERR                        E_BASIC_BASE(103)

extern "C" __declspec(dllexport)
int err_init(void);

extern "C" __declspec(dllexport)  
int set_last_err(int usb_index, int site_nr, int err);

extern "C" __declspec(dllexport) 
int get_last_err(int usb_index, int site_nr);

#ifdef  UNICODE 

#define	err_str(a) err_str_unicode(a)

extern "C" __declspec(dllexport) 
const wchar_t *err_str_unicode(int err);

extern "C" __declspec(dllexport) 
const char *err_str_ascii(int err);


#else

#define	err_str(a) err_str_ascii(a)

extern "C" __declspec(dllexport) 
const char *err_str_ascii(int err);

#endif






#endif // !_ERROR_NO_H
