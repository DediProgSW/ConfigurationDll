#ifndef	_CMD_PROTOCOL_H
#define	_CMD_PROTOCOL_H
#include <stdlib.h>
#include "../Dediprog_bg/svr_config.h"
#include "loadfile_info.h"
#include "prog_info.h"
#include "chip_op_name.h"

//command
#define CTRL_CMD_BASE					10000
#define CTRL_CMD_GET_DEVICE_CNT				CTRL_CMD_BASE + 1
#define CTRL_CMD_GET_DEVICE_LIST			CTRL_CMD_BASE + 2
#define CTRL_CMD_GET_CHIP_INFO				CTRL_CMD_BASE + 3
#define CTRL_CMD_GET_CHIP_ID				CTRL_CMD_BASE + 4
#define CTRL_CMD_GET_CHIP_DETAIL_SETTING		CTRL_CMD_BASE + 5
#define CTRL_CMD_GET_STATUS				CTRL_CMD_BASE + 6
#define CTRL_CMD_SET_CHIP_INFO				CTRL_CMD_BASE + 7
#define CTRL_CMD_LOAD_FILE				CTRL_CMD_BASE + 8
#define CTRL_CMD_SET_ERASE				CTRL_CMD_BASE + 9
#define CTRL_CMD_SET_BLANK 				CTRL_CMD_BASE + 10
#define CTRL_CMD_SET_PROGRAM				CTRL_CMD_BASE + 11
#define CTRL_CMD_SET_VERIFY				CTRL_CMD_BASE + 12
#define CTRL_CMD_SET_BATCH_CHIP				CTRL_CMD_BASE + 13
#define CTRL_CMD_SET_BLINK				CTRL_CMD_BASE + 14
#define CTRL_CMD_GET_DEVICE_INFO			CTRL_CMD_BASE + 15
#define CTRL_CMD_GET_CHIP_MANUFCT_CNT			CTRL_CMD_BASE + 16
#define CTRL_CMD_GET_CHIP_MANUFCT_LIST			CTRL_CMD_BASE + 17
#define CTRL_CMD_GET_CHIP_PARTNUM_CNT			CTRL_CMD_BASE + 18
#define CTRL_CMD_GET_CHIP_PARTNUM_LIST			CTRL_CMD_BASE + 19
#define CTRL_CMD_GET_FILE_CNT				CTRL_CMD_BASE + 20
#define CTRL_CMD_GET_FILE_LIST				CTRL_CMD_BASE + 21
#define CTRL_CMD_SET_FILE				CTRL_CMD_BASE + 22
#define CTRL_CMD_SET_BATCH_DEVICE			CTRL_CMD_BASE + 23
#define	CTRL_CMD_GET_DIR_CNT				CTRL_CMD_BASE + 24
#define	CTRL_CMD_GET_DIR_CONTENT			CTRL_CMD_BASE + 25
#define	CTRL_CMD_CREATE_PROJECT				CTRL_CMD_BASE + 26
#define	CTRL_CMD_FIRMWARE_UPDATE			CTRL_CMD_BASE + 27
#define CTRL_CMD_CLIENT_LOGIN				CTRL_CMD_BASE + 50





/*------old command param, if have time, refactor it to that:
 *		one command correspondence one param structure
 */

#pragma pack(push, 1)

struct index_param {
	unsigned short order_index;
	unsigned short site_index;
};

/* read id */
struct readid_param {
	unsigned short order_index;
	unsigned short site_index;
};

struct readid_res {
	/* 1 --> correct id
	 * 0 --> incorrect id
	 */
	unsigned char state;
	/* id count */
	unsigned char id_cnt;
	/* id buffer */
	unsigned char id_code[8];
	/* time takes unit(0.1s) */
	unsigned int takes;
};

/* erase */
struct erase_param {
	unsigned short order_index;
	unsigned short site_index;
};

/* progress info */
struct progress_info {
	unsigned short order_index;
	unsigned short site_index;
	unsigned char op_type;
	unsigned int current;
	unsigned int total;
	unsigned int take_time;	/* takes time */
	unsigned int result;
};

/* firmware update */
struct fw_update_param {
	int	order_index;
	wchar_t file_path[_MAX_PATH];
};

struct dev_info {
	prog_type_t type;	//
	unsigned short order_index;
	unsigned char ver[3];
	unsigned char reserved[8];
    unsigned char sn[16];
};

struct data_file_info {
	wchar_t path[_MAX_PATH];
};
//typedef struct 
//{
//      unsigned char           state;
//      unsigned char           id_num;
//      unsigned char           id_code[8];
//}chip_id_info;

struct chip_info_param {
	wchar_t type[16];
	wchar_t mfg[30];
	wchar_t part_nr[50];
	int	free_loadfile;
};

struct chip_info_res {
	wchar_t type[16];
	wchar_t mfg[30];
	unsigned long chipsize_low;
	unsigned long chipsize_high;
	unsigned long flash_id_nr;
	unsigned char flash_id[8];
};

struct chip_type {
	wchar_t type[16];
};

typedef struct {
	wchar_t mfg[30];
} chip_mft;

typedef struct {
	wchar_t type[16];
	wchar_t mfg[30];
} chip_search;

typedef struct {
	wchar_t filename[64];
} bin_file_info;



struct folder_info{
        unsigned long long size;
	wchar_t file_name[64];
	int is_dir;
};

struct get_dir_param {
	wchar_t path[_MAX_PATH];
	int cnt;
};

struct check_sum {
	unsigned long sum;
};

struct batch_param_info {
	unsigned short order_index;
	unsigned short site_index;
	unsigned char config[4];
};

struct operate_result {
	unsigned short usbindex;
	unsigned short chipindex;
	unsigned char optype;
	unsigned int result;
        unsigned long takes;
};

struct contact_info {
	unsigned short pin_mask[4];
	unsigned long pin_max_volt;
	unsigned long pin_min_volt;
};

struct memory_info {
	wchar_t display_name[16];
	wchar_t type[20];
	unsigned long long start_program_addr;
	unsigned long long size_in_bytes;
	unsigned long page_size_in_byte;
	unsigned long block_size_in_byte;
	unsigned long sector_size_in_byte;
	unsigned long erase_parameter;
	unsigned long read_parameter;
	unsigned long program_parameter;
	unsigned long unprotect_parameter;
	unsigned long protect_parameter;
	unsigned long option_cmd1;
	unsigned long option_cmd2;
	unsigned long timing;
	unsigned long rev[4];
	wchar_t rev_str[4][20];
};

struct chip_info_detail {
	wchar_t part_name[50];
	wchar_t type[16];
	wchar_t manufacturer[30];
	wchar_t description[20];
	wchar_t socket_adpator[40];
	wchar_t ordering_info[16];
	unsigned long chip_vcc;
	unsigned long buffer_vcc;
	unsigned long chip_vpp;
	unsigned long bus_width;
	unsigned short power_pin[2];
	unsigned long flash_id_nr;
	unsigned char flash_id[8];
	unsigned long contact_test_nr;
	contact_info chip_contact[5];

	wchar_t fpga_fw_x4[30];
	wchar_t fpga_fw_x8[30];
	wchar_t fpga_fw_starprog[30];
	wchar_t fpga_universal_cartridge_x4[30];
	wchar_t fpga_universal_cartridge_x8a[30];
	wchar_t fpga_universal_cartridge_x8b[30];
	wchar_t vector_file[30];
	wchar_t ram_function_file[30];
	wchar_t ram_function_transfer_dll[30];
	unsigned long memory_cnt;
	memory_info memory_info[8];
};

#pragma pack( pop )

/*--------------------New SW Framework Command-----------------------------------*/

/*----------Progmaster Order exchage, only the length need to call ntohs---------*/
enum {
        SOCKET_CMD_BASE = 1000,

        SOCKET_CMD_PROG_ORDER_RES,	/* init order */
        SOCKET_CMD_PROG_ORDER_EXCHANGE, /* handle the map from usb index to order index */

        SOCKET_CMD_GET_SERVER_VER,      /* get server version */
	SOCKET_CMD_CHECK_SOCKET,	/* get socket status */

        SOCKET_CMD_GET_MFG_CNT = 1100,         /* get mfg cnt */
        SOCKET_CMD_GET_MFG_TBL,         /* get mfg tbl */
        SOCKET_CMD_GET_CONFIGFILE_VER,  /* get config file version, create time */


        SOCKET_CMD_GET_PARTITION_CNT = 1200,   /* get the memory count */
        SOCKET_CMD_GET_PARTITION_TBL,   /* get the memory table */

        SOCKET_CMD_GET_SAU_CNT = 1300,         /* get spare area usage cnt */
        SOCKET_CMD_GET_SAU_TBL,         /* get the spare area usage table */

        SOCKET_CMD_GET_BBM_CNT = 1400,         /* get the bad block management cnt */
        SOCKET_CMD_GET_BBM_TBL,         /* get the bad block management table */
        SOCKET_CMD_SET_BBM_METHOD,      /* set the bbm method to every image/file */

        SOCKET_CMD_LOAD_FILE_TBL = 1500,       /* load the file table  to server */
        SOCKET_CMD_GET_FILE_TBL_CNT,    /* get the file tble count from server */
        SOCKET_CMD_GET_FILE_TBL,        /* get the file table from server */
        SOCKET_CMD_GET_FCS_CNT,         /* get the file checksum counter */
        SOCKET_CMD_GET_FCS_TBL,         /* get the file checksum table */
	/* new loadfile command */
	SOCKET_CMD_IMG_SET,		/* set the image tbl */
	SOCKET_CMD_IMG_GET_SIZE,	/* get the image tbl size */
	SOCKET_CMD_IMG_GET,		/* get the image tbl */
	SOCKET_CMD_REGISTER_SET,	/* loadfile register set */
	SOCKET_CMD_REGISTER_SIZE,	/* loadfile register size */
	SOCKET_CMD_REGISTER_GET,	/* loadfile register get */
        SOCKET_CMD_SET_OP_PARAM,        /* set default operation parameter */
        SOCKET_CMD_SET_IMG_PARAM,       /* set the image parameter */
        SOCKET_CMD_SET_IMAGE_PARAM,     /* set the image pararmeter (new loadfile command) */
        SOCKET_CMD_GET_IMAGE_PARAM_SIZE,/* get the image parameter size(new loadfile command) */
        SOCKET_CMD_GET_IMAGE_PARAM,     /* get the image parameter */
	SOCKET_CMD_GET_FILE_ADDRINFO_SIZE,	/* get the file address info */
	SOCKET_CMD_GET_FILE_ADDRINFO,	/* get the file address info */
	SOCKET_CMD_GET_FILE_FORMAT_CNT,	/* get the file format table */
	SOCKET_CMD_GET_FILE_FORMAT_TBL,	/* get the file format table */
	SOCKET_CMD_GET_ECC_METHOD_CNT,	/* get the ecc method table */
	SOCKET_CMD_GET_ECC_METHOD_TBL,	/* get the ecc method table */

	SOCKET_CMD_GET_OP_PARAM_SIZE,	/* get operation param size */
	SOCKET_CMD_GET_OP_PARAM,	/* get operation param size */
	SOCKET_CMD_GET_IMG_PARAM_SIZE,	/* image param */
	SOCKET_CMD_GET_IMG_PARAM,	/* image param */
	SOCKET_CMD_SAVE_LOADFILE_SLN,	/* save loadfile sln */
	SOCKET_CMD_READ_LOADFILE_SLN,	/* read loadfile sln */
	SOCKET_CMD_LOADFILE_SLN_SIZE,	/* load file size */

        SOCKET_CMD_GET_IMG_CHIP_CHECKSUM,   /* get chip checksum */

        SOCKET_CMD_LOAD_PARTITION_TBL = 1600,   /* load the partition table */
        SOCKET_CMD_SAVE_PARTITION_TBL,          /* save the partition table */

        SOCKET_CMD_GET_PRJ_CNT = 1700,  /* get the project count in sdcard */
        SOCKET_CMD_GET_PRJ_TBL,         /* get the project name table */
        SOCKET_CMD_SELECT_PRJ,          /* select one project in sdcard*/
        SOCKET_CMD_SAVE_PRJ,            /* save the project file */
        SOCKET_CMD_DOWNLOAD_PRJ,        /* download project */
        SOCKET_CMD_RUN_PRJ,             /* run the project */
        SOCKET_CMD_STOP_PRJ,            /* stop the project*/
        SOCKET_CMD_PRJ_START_SITE,      /* start one site */
        SOCKET_CMD_PRJ_START_DEV,       /* project start device */
        SOCKET_CMD_GET_RPJ_INFO,        /* get project information */
        SOCKET_CMD_GET_PRJ_STATUS,      /* get project run status */
        SOCKET_CMD_GET_PRJ_SITE_STATUS,  /* get project site status */

        SOCKET_CMD_SET_BATCH_SETTING = 1800,   /* set the batch setting */
        SOCKET_CMD_GET_BATCH_CNT,       /* get the batch setting */
        SOCKET_CMD_GET_BATCH_SETTING,       /* get the batch setting cnt */
        SOCKET_CMD_BATCH_SITE,          /* batch site */

        SOCKET_CMD_GET_IC_SETTING_DLG = 1900,  /* get the ic setting dll name */
        SOCKET_CMD_EMMC_EXTCSD_WRITE,   /* send the extcsd in emmc to server */
        SOCKET_CMD_GET_REG_VALUE,       /* get register value */

        SOCKET_CMD_CHIP_READID = 2000,  /* read id */
        SOCKET_CMD_CHIP_ERASE,          /* erase */
        SOCKET_CMD_CHIP_BLANK,          /* balnk check */
        SOCKET_CMD_CHIP_PROGRAM,        /* program */
        SOCKET_CMD_CHIP_VERIFY,         /* verify */
        SOCKET_CMD_PARTITION_READ,      /* read one partition */
        SOCKET_CMD_PARTT_READ_FILE,     /* editor read partition buff-file */
        SOCKET_CMD_PARTT_READ_CHIP,     /* editor read partition chip-file */
        SOCKET_CMD_SAVE_PTN,            /* save partition */
        SOCKET_CMD_GET_OPERATION_STATUS,/* chip operation status */
	SOCKET_CMD_STOP_OP,		/* stop operation */


        SOCKET_CMD_SELECT_CHIP = 2100,         /* select chip */
        SOCKET_CMD_GET_CHIP_INFO,       /* get chip inforamtion */

        SOCKET_CMD_USB_REG_READ = 2200, /* usb read fpga register */
        SOCKET_CMD_USB_REG_WRITE,       /* usb write fpga register */
        SOCKET_CMD_USB_FIFO_READ,       /* usb read fpga fifo */
        SOCKET_CMD_USB_FIFO_WRITE,      /* usb read fpga write */
        SOCKET_CMD_USB_SRAM_READ,       /* usb read sram */
        SOCKET_CMD_USB_SRAM_WRITE,      /* usb write sram */
        SOCKET_CMD_RAMFUNC_TRIG,        /* ram function trig */
        SOCKET_CMD_RAMFUNC_CHECK,       /* ram function check */
        SOCKET_CMD_POWER_OPERATION,     /* power operation */
        SOCKET_CMD_USB_CONTACT,         /* usb contact */
        SOCKET_CMD_USB_LED,             /* usb led control */

        SOCKET_CMD_GET_USR_LIMIT = 2300,       /* user limit count */
        SOCKET_CMD_SET_USR_LIMIT,
        SOCKET_CMD_GET_SKT_LIMIT,

        SOCKET_CMD_GET_SN = 2400,       /* get progmaster sn */
        SOCKET_CMD_SET_SN,              /* set progmaster sn */

        SOCKET_CMD_GET_DLGDLL_SIZE = 2500, /* get ui dll size */
        SOCKET_CMD_GET_DLGDLL,		/* cmd get dll content */

	SOCKET_CMD_CHKSUM_METHOD_CNT = 2600,	/* checksum method tbl count */
	SOCKET_CMD_CHKSUM_METHOD_TBL,		/* checksum method tbl */

        SOCKET_CMD_GET_SPEC_PATH = 2700,     /* get desktop path */
};

#pragma warning(push)
#pragma warning(disable:4200)

/*---------------------------------------------------------*/
//SOCKET_CMD_PROG_ORDER_INIT

struct prog_order_init_param {
        /* NULL */
};

struct prog_order_init_res {
        /* NULL */
};

//SOCKET_CMD_PROG_ORDER_EXCHANGE

#pragma pack(push, 1)
struct prog_order_exchange_param {
	int	exchange1;
	int	exchange2;
};

struct prog_order_exchange_result {

};

#pragma pack( pop )

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_SERVER_VER
#pragma pack(push, 1)
struct get_server_ver_param {

};

struct get_server_ver_res {
        wchar_t ver[64];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_CHECK_SOCKET
#pragma pack(push, 1)
struct check_socket_param{
	int order_index;
	int site_index;
};

struct check_socket_res {
	/* NULL */
};


#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_CONFIGFILE_VER
#pragma pack(push, 1)
struct get_config_ver_param {
};

struct get_config_ver_res {
        wchar_t ver[32];
        wchar_t create_time[64];
};

#pragma pack(pop)


/*---------------------------------------------------------*/
//SOCKET_CMD_GET_PARTITION_TBL

#pragma pack(push, 1)

struct socket_partition_info {
        wchar_t         name[64];
        wchar_t         type[30]; 
        unsigned long   page_size;
        unsigned long   sector_size;
        unsigned long   block_size;
        long long       memory_addr;
        long long       memory_length;
};

struct get_memory_tbl_param {
        /* it need nothing */
};

struct get_memory_tbl_res {
        int                                partition_cnt;
        struct socket_partition_info       partition_info[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_SAU_TBL

#pragma pack(push, 1)

struct socket_sau_item {                /* spare area usage */
        wchar_t name[30];
};

struct get_sau_tbl_param {
        wchar_t type[30];               /* NAND, NOR, MCU */
};

struct get_sau_tbl_res {
        int                             sau_cnt;
        struct socket_sau_item          sau_item[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_BBM_TBL
#pragma pack(push, 1)
struct socket_bbm_item {                /* bad block management*/
        wchar_t name[30];
};

struct get_bbm_tbl_param {
                                        /* NULL */
};

struct get_bbm_tbl_res {
        int                             bbm_cnt;
        struct socket_bbm_item          bbm_item[0];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_LOAD_FILE_TBL

#pragma pack(push, 1)
struct socket_file_info {
        wchar_t         name[30];           /* memory name */
        wchar_t         type[30];           /* NAND, NOR, MCU */
		wchar_t         cks_name[30];       /* Check sum alg name*/
		wchar_t         file_format[30];    /* analys file as format*/
        long long       memory_addr;
        long long       file_offset;
        long long       program_length;
        wchar_t         file_path[_MAX_PATH];
        wchar_t         sau_name[30];
        unsigned long   is_skip_blank;
        struct {
                int                     byte_nr;
                union {
                        unsigned long           u32;                /* unused value,only care when mymory_type is not nand & eMMC */
                        unsigned short          u16;
                        unsigned char           u8;
                }value;
        }unused_value;
	//unsigned char   is_fill_unused;     /*not care for Nand&eMMC ,if 1 fill with unused value,0 not fill*/
	//unsigned char   unused_value[2];    /* unused value */
};

struct load_file_tbl_param {
        int                             partition_cnt;
        struct socket_file_info         partition_info[];
};

struct load_file_tbl_res {
        /* this command doesn't need any result data */
};


//SOCKET_CMD_GET_FILE_TBL
struct get_file_tbl_param {
        /* this command doesn't need any param data */
};

struct get_file_tbl_res {
        int                             img_cnt;
        struct socket_file_info         img_info[];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_FCS_TBL
#pragma pack(push, 1)

struct get_fcs_tbl_param {
        int     cnt;
};

struct socket_fcs_item {
        wchar_t path[_MAX_PATH];
        unsigned long cs;
};

struct get_fcs_tbl_res {
        int cnt;
        struct socket_fcs_item	tbl[]; 
};

#pragma pack(pop)


/*---------------------------------------------------------*/
//SOCKET_CMD_SET_BBM_METHOD

#pragma pack(push, 1)
struct set_bbm_method_item {
        wchar_t         name[30];           /* memory name */
        wchar_t         type[30];           /* NAND, NOR, MCU */
        long long       memory_addr;        /* memory start program address */
        long long       program_length;     /* program length */
        wchar_t         bbm_method[30];     /* bbm method */
};
struct set_bbm_method_param {
        int                             item_cnt;
        struct set_bbm_method_item      item[];
};

struct set_bbm_method_res {
        /* this command does not need any result data */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_SAVE_PRJ
#pragma pack(push, 1)

struct save_prj_param {
        wchar_t         path[_MAX_PATH];
};

struct save_prj_res {
        /* this command does not need any result data */
};


#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_PRJ_CNT,         /* get the project count in sdcard */
//SOCKET_CMD_GET_PRJ_TBL,         /* get the project name table */
#pragma pack(push, 1)

struct get_prj_tbl_param{
        /* this command doesn't need any param data */
        int     prj_cnt;
};

struct get_prj_tbl_res {
       wchar_t name_tbl[][64];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_SELECT_PRJ         /* select one project in sdcard*/
#pragma pack(push, 1)

struct select_prj_param {
        wchar_t    prj_name[64];
};

struct select_prj_res {
        /* this command doesn't need any result data */
};


#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_DOWNLOAD_PRJ,                /* download project */
#pragma pack(push, 1)

struct download_prj_param {
        wchar_t prj_name[_MAX_PATH];
};

struct download_prj_res {
        /* this command doesn't need any result data */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_RUN_PRJ,             /* run the project */
#pragma pack(push, 1)

struct run_prj_param {
        int     order_index;      /* -1 --> all device */
};

struct run_prj_res {
        /* this command doesn't need any result data*/
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_STOP_PRJ,             /* stop the project */
#pragma pack(push, 1)

struct stop_prj_param {
	int     order_index;      /* -1 --> all device */
};

struct stop_prj_res {
	/* this command doesn't need any result data*/
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_PRJ_START_SITE,      /* start one site */
#pragma pack(push, 1)

struct start_site_param {
	int     order_index;      /* must >= 0 */
        int     site_index;
};

struct start_site_res {
	/* this command doesn't need any result data*/
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_PRJ_START_DEV
#pragma pack(push, 1)
struct start_dev_param {
        int     order_index;
};

struct start_dev_res {
	/* this command doesn't need any result data*/
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_PRJ_INFO
#pragma pack(push, 1)
struct get_prj_info_param {
        /*      this command doesn't need any param */
};

struct get_prj_info_res {
        wchar_t         file_name[128];
        unsigned long   size;
        unsigned long   check_sum;
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_PRJ_STATUS

enum socket_prj_status {
        SOCKET_PRJ_IDLE,
        SOCKET_PRJ_SELECTED,
        SOCKET_PRJ_RUNNING,
};

#pragma pack(push, 1)
struct get_prj_status_param {
        int     order_index;
};

struct get_prj_status_res {
        enum socket_prj_status  status;      
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_PRJ_SITE_STATUS
#include "../Dediprog_bg/project_run.h"

#pragma pack(push, 1)

struct get_prj_site_status_param {
        int order_index;
        int site_index;
};

struct get_prj_site_status_res {
        enum prj_site_status    status;
};

#pragma pack(pop)
//SOCKET_CMD_SET_BATCH_SETTING
#pragma pack(push, 1)

enum start_mode{
	START_FROM_MANUAL_MODE,
	START_FROM_AUTO_DETECTION,
	START_FROM_HANDLER
};
struct socket_op_item {
        wchar_t op[OP_NAME_SIZE];
        wchar_t partition_name[64];
};

struct set_batch_setting_param {
        int                     op_cnt;
        enum start_mode         mode;
        struct socket_op_item   op_tbl[];	
};

struct set_batch_setting_res {
        /* this command doesn't need any result command */
};

//

//SOCKET_CMD_GET_BATCH_SETTING
struct get_batch_setting_param {
        int cnt;
};

struct get_batch_setting_res {
        int                     op_cnt;
        enum  start_mode        mode;
        struct socket_op_item   op_tbl[];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_IC_SETTING_DLG
#pragma pack(push, 1)

struct get_ic_setting_dlg_param {
        /* this command doesn't need any result command */
};

struct get_ic_setting_dlg_res {
        wchar_t dll_name[64];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_EMMC_EXTCSD_WRITE
#pragma pack(push, 1)

struct socket_extcsd_value {
        unsigned long addr;
        unsigned long value;            /* Little end */
};

struct emmc_extcsd_write_param {
        int                             extcsd_cnt;
        struct socket_extcsd_value      value[];
};

struct emmc_extcsd_write_res {
        /* this command does not need any result command */
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_CHIP_ERASE,               /* erase */
#pragma pack(push, 1)

struct chip_erase_param {
        int     order_index;
        int     site_index;
        wchar_t partition_name[64];
};

struct chip_erase_res {
        /* this command does not need any result data */
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_CHIP_BLANK,         /* balnk check */
#pragma pack(push, 1)

struct chip_blank_param {
        int     order_index;
        int     site_index;
        wchar_t partition_name[64];
};

struct chip_blank_res {
        /* this command does not need any result data */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_CHIP_PROGRAM,             /* program */
#pragma pack(push, 1)

struct chip_program_param {
        int     order_index;
        int     site_index;
        wchar_t partition_name[64];
};

struct chip_program_res {
        /* this command does not need any result data */
};

#pragma pack(pop)



/*---------------------------------------------------------*/
//SOCKET_CMD_CHIP_VERIFY,              /* verify */
#pragma pack(push, 1)

struct chip_verify_param {
        int     order_index;
        int     site_index;
        wchar_t partition_name[64];
};

struct chip_verify_res {
        /* this command does not need any result data */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_PARTITION_READ
#pragma pack(push, 1)

struct read_partition_param {
        int                     order_index;              /* must >= 0 */
        int                     site_index;               /* must >= 0 */
};

struct read_partition_res {
	                                                    /* no need */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_PARTT_READ_CHIP
#pragma pack(push, 1)

struct read_partt_chip_param {
        int order_index;
        int site_index;
        unsigned long long      start_addr;
        unsigned long long      read_len;
        wchar_t                 partition_name[64];
};

struct read_partt_chip_res {
	        unsigned long long      data_len;
	        unsigned char           buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_PARTT_READ_FILE
#pragma pack(push, 1)

struct read_partt_file_param {
	        unsigned long long      start_addr;
	        unsigned long long      read_len;
	        wchar_t                 partition_name[64];
};

struct read_partt_file_res {
	        unsigned long long      data_len;
	        unsigned char           buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_SAVE_PTN
#pragma pack(push, 1)

struct save_ptn_param {
        int             order_index;
        int             site_index;
        wchar_t         path[_MAX_PATH];
        wchar_t         ptn_name[64];
};

struct save_ptn_res {
        /* null */
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_OPERATION_STATUS
#pragma pack(push, 1)

struct get_operation_status_param {
        int order_index;
        int site_index;
};

struct get_operation_status_res {
        int order_index;
        int site_index;
        unsigned long total;
        unsigned long current;
        int err;
        int takes;
        char op_name;
        unsigned char is_finish;
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_BATCH_SITE
#pragma pack(push, 1)

struct batch_site_param {
        int     order_index;
        int     site_index;
};

struct batch_site_res {
        /* this command does not need any result data */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_SELECT_CHIP
#pragma pack(push, 1)
struct select_chip_param {
        wchar_t type[16];
	wchar_t mfg[30];
	wchar_t part_nr[50];
};


struct select_chip_res {
        /* this command does not need any result data */
};

#pragma pack(pop)

//SOCKET_CMD_GET_CHIP_INFO
#pragma pack(push, 1)
struct get_chip_info_param {
};

struct get_chip_info_res {
        struct chip_info_c chip_detail;
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_USB_REG_READ = 2200, /* usb read fpga register */
#pragma pack (push, 1)
struct usb_reg_read_param {
        int     order_index;
        int     site_index;
        unsigned short addr;
        unsigned short value;
};

struct usb_reg_read_res {
        unsigned short addr;
        unsigned short value;
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_REG_WRITE,       /* usb write fpga register */
#pragma pack (push, 1)
struct usb_reg_write_param {
        int     order_index;
        int     site_index;
        unsigned short addr;
        unsigned short value;
};

struct usb_reg_write_res {
        /* NULL */
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_FIFO_READ,       /* usb read fpga fifo */
#pragma pack (push, 1)
struct usb_fifo_read_param {
        int     order_index;
        int     site_index;
        unsigned long len;
};

struct usb_fifo_read_res {
        unsigned long len;
        unsigned char buff[];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_FIFO_WRITE,      /* usb read fpga write */
#pragma pack (push, 1)

struct usb_fifo_write_param {
        int     order_index;
        int     site_index;
        unsigned long len;
        unsigned char buff[];
};
struct usb_fifo_write_res {
        /* NULL */
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_SRAM_READ,       /* usb read sram */
#pragma pack (push, 1)
struct usb_sram_read_param {
        int     order_index;
        int     site_index;
        unsigned short addr;
        unsigned long  len;
};

struct usb_sram_read_res {
        unsigned short  addr;
        int             len;
        unsigned char   buff[];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_SRAM_WRITE,      /* usb write sram */
#pragma pack (push, 1)
struct usb_sram_write_param {
        int     order_index;
        int     site_index;
        unsigned short addr;
        unsigned long  len;
        unsigned char buff[];
};

struct usb_sram_write_res {
        /* NULL */
};
#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_RAMFUNC_TRIG,        /* ram function trig */
#pragma pack(push, 1)

struct trig_ramfunc_param {
        int     order_index;
        int     site_index;
        int     trig_len;
        unsigned char trig[128];
        int     param_len;
        unsigned char param[128];
};

struct trig_ramfunc_res {
        /* NULL */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_RAMFUNC_CHECK,       /* ram function check */
#pragma pack(push, 1)
struct check_ramfunc_param {
        int     order_index;
        int     site_index;
	int     status_len;
        int     result_len;
};

struct check_ramfunc_res {
	int     status_len;
        int     result_len;
        unsigned char status[128];
        unsigned char result[2048];
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_USB_CONTACT
#pragma pack(push, 1)
struct usb_contact_param {
        int     order_index;
        int     site_index;
        unsigned short  pin_mask[4];
        unsigned long  max_volt;        /* mv */
        unsigned long  min_volt;        /* mv */
};

struct usb_contact_res {
        /* NULL */
};
#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_USB_LED,             /* usb led control */
#pragma pack(push, 1)
enum socket_led_color{
                USB_LED_NONE,
                USB_LED_ALL,
                USB_LED_BLUE,
                USB_LED_YELLOW,
                USB_LED_RED,
};
struct usb_led_param {
        int     order_index;
        int     site_index;
        enum socket_led_color color;
};

struct usb_led_res {
        /* NULL */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_USR_LIMIT = 2300,       /* user count */
#pragma pack(push, 1)
struct get_usr_limit_param {
        /* NULL */
};

struct get_usr_limit_res {
        unsigned long limit;
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_SET_USR_LIMIT
#pragma pack(push, 1)
struct set_usr_limit_param {
        unsigned long limit;
};

struct set_usr_limit_res {
        /* NULL */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_SKT_LIMIT
#pragma pack(push, 1)
struct get_skt_limit_param {
        int     order_index;
        int     site_index;
};

struct get_skt_limit_res {
        int     pass;
        int     fail;
        int     limit;
};
#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_SN = 2400,       /* get progmaster sn */
#include <time.h>
#pragma pack(push, 1)
struct get_sn_param {
        int     order_index;
};

struct get_sn_res {
        /* NULL */
        unsigned char sn[16];
        struct tm time;
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_SET_SN,              /* set progmaster sn */
#pragma pack(push, 1)

struct set_sn_param {
        int		order_index;
        unsigned char	sn[16];
};

struct set_sn_res {
        /* NULL */
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_POWER_OPERATION,     /* power operation */
#pragma pack(push, 1)
struct usb_power_param {
        int     order_index;
        int     site_index;
        int     power_on;       /* 0 --> off, 1 --> on */
        int     power_type;     /* 0 --> socket vcc, 2 --> socket vpp */
};

struct usb_power_res {
        /* NULL */
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_LOAD_PARTITION_TBL

#pragma pack(push, 1)

/* this command have not support */
typedef struct __st_callback_addon{
	unsigned int(*UsbRegRead)(struct usb_reg_read_param *paramIn, struct usb_reg_read_res *paramOut);
	unsigned int(*UsbRegWrite)(struct usb_reg_write_param *paramIn);
	unsigned int(*UsbFifoRead)(struct usb_fifo_read_param *paramIn, struct usb_fifo_read_res *paramOut);
	unsigned int(*UsbFifoWrite)(struct usb_fifo_write_param *paramIn);
	unsigned int(*UsbSramRead)(struct usb_sram_read_param *paramIn, struct usb_sram_read_res *paramOut);
	unsigned int(*UsbSramWrite)(struct usb_sram_write_param *paramIn);
	unsigned int(*RamFuncTrig)(struct trig_ramfunc_param *paramIn);
	unsigned int(*RamFuncCheck)(struct check_ramfunc_param *paramIn, struct check_ramfunc_res *paramOut);
	unsigned int(*PowerOp)(struct usb_power_param *paramIn);
    unsigned int(*led)(struct usb_led_param *param);
    unsigned int(*contact)(struct usb_contact_param *param);
}CALLBACK_ADDON;

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_IMG_SET,		/* set the image tbl */
#pragma pack(push, 1)
struct img_set_param {
	unsigned long len;
	unsigned char buff[];
};

struct img_set_res {
	/* NULL */
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_IMG_GET,		/* get the image tbl */
#pragma pack(push, 1)
struct img_get_param {
	/* NULL */
};

struct img_get_res {
	unsigned long len;
	unsigned char buff[];
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_REGISTER_SET,	/* loadfile register set */
struct register_set_param {
	unsigned long len;
	unsigned char buff[];
};

struct register_set_res {
	/* NULL */
};
/*---------------------------------------------------------*/
//SOCKET_CMD_REGISTER_GET,	/* loadfile register get */e
struct register_get_param {
	/* NULL */
};

struct register_get_res {
	unsigned long len;
	unsigned char buff[];
};
/*---------------------------------------------------------*/
//SOCKET_CMD_SET_OP_PARAM,                /* set default operation parameter */
#pragma pack(push, 1)
struct set_op_param_param {
        unsigned int    op;     /* E B P V */
		wchar_t         ptn_name[64];
        int             len;
        unsigned char   buff[];
};

struct set_op_param_res {
        /* NULL */
};
#pragma pack(pop)
/*---------------------------------------------------------*/

//SOCKET_CMD_SET_IMAGE_PARAM 
#pragma pack(push, 1)
struct set_image_param_param {
	int	img_index;
        int     len;
        unsigned char buff[];
};

struct set_image_param_res {
        /* NULL */
};
#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_IMAGE_PARAM_SIZE,/* get the image parameter size(new loadfile command) */
//SOCKET_CMD_GET_IMAGE_PARAM,     /* get the image parameter */
#pragma pack(push, 1)
struct get_image_param_param {
        int     img_index;
};

struct get_image_param_res {
        int     len;
        unsigned char buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_FILE_ADDRINFO_CNT,	/* get the file address info */
//SOCKET_CMD_GET_FILE_ADDRINFO,	/* get the file address info */
#pragma pack(push, 1)

struct get_file_addrinfo_param {
	wchar_t	path[_MAX_PATH];
	wchar_t format[LOADFILE_FILEFORMAT_LEN];
};

struct get_file_addrinfo_res {
	int size;
	unsigned char buff[];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_GET_FILE_FORMAT_CNT,	/* get the file format table */
//SOCKET_CMD_GET_FILE_FORMAT_TBL,	/* get the file format table */
#pragma pack(push, 1)

struct get_file_format_tbl_param {
	/* NULL */
};

struct get_file_format_tbl_res {
	int cnt;
	wchar_t tbl[][LOADFILE_FILEFORMAT_LEN];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_ECC_METHOD_CNT,	/* get the ecc method table */
//SOCKET_CMD_GET_ECC_METHOD_TBL,	/* get the ecc method table */

#pragma pack(push, 1)

struct get_ecc_tbl_param {
	/* NULL */
};

struct get_ecc_tbl_res {
	int cnt;
	wchar_t tbl[][LOADFILE_ECC_LEN];
};


#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_SET_IMG_PARAM,               /* set the image parameter */
#pragma pack(push, 1)
struct set_img_param_param {
	wchar_t         name[64];           /* memory name */
        wchar_t         type[30];           /* NAND, NOR, MCU */
        long long       memory_addr;        /* memory start program address */
        long long       program_length;     /* program length */
        int     len;
        unsigned char buff[];
};

struct set_img_param_res {
        /* NULL */
};
#pragma pack(pop)


/*---------------------------------------------------------*/
//SOCKET_CMD_GET_OP_PARAM_SIZE,	/* get operation param size */
//SOCKET_CMD_GET_OP_PARAM,	/* get operation param size */

#pragma pack(push, 1)

struct get_op_param_size_param {
        unsigned int    op;     /* E B P V */
	    wchar_t         ptn_name[64];

};

struct get_op_param_param {
        unsigned int    op;     /* E B P V */
	    wchar_t         ptn_name[64];

};

struct get_op_param_res {
        /* NULL */
        int             len;
        unsigned char   buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_IMG_PARAM_SIZE,	/* image param */
//SOCKET_CMD_GET_IMG_PARAM,	/* image param */
#pragma pack(push, 1)

struct get_img_param_param {
	wchar_t         name[64];           /* memory name */
        wchar_t         type[30];           /* NAND, NOR, MCU */
        long long       memory_addr;        /* memory start program address */
        long long       program_length;     /* program length */
};

struct get_img_param_res {
        int		len;
        unsigned char	buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_LOADFILE_SLN_SIZE,	/* load file size */
#pragma pack(push, 1)
struct loadfile_sln_size_param {
	wchar_t		path[MAX_PATH];
};
struct loadfile_sln_size_res {
	unsigned long	size;
};
#pragma pack(pop)
//SOCKET_CMD_SAVE_LOADFILE_SLN,	/* save loadfile sln */
#pragma pack(push, 1)
struct save_loadfile_sln_param {
	wchar_t		path[MAX_PATH];
	unsigned long	sln_size;
	unsigned char	sln_buff[];
};
struct save_loadfile_sln_res {
	/* NULL */
};
#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_READ_LOADFILE_SLN,	/* read loadfile sln */
#pragma pack(push, 1)
struct read_loadfile_sln_param {
	wchar_t		path[MAX_PATH];
};

struct read_loadfile_sln_res {
	unsigned long sln_size;
	unsigned char sln_buff[];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_IMG_CHIP_CHECKSUM,   /* get chip checksum */
#pragma pack(push, 1)
struct get_img_chip_checksum_param {
        /* NULL */
};

struct get_img_chip_checksum_res {
        unsigned long checksum;
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_DLGDLL = 2500, /* get ui dll size */

//SOCKET_CMD_GET_DLGDLL      = 2600, /* cmd get dll content */         
#pragma pack(push, 1)
struct get_dlgdll_param {
        /* NULL */
        int parent_class;
};

struct get_dlgdll_res {
        int len;
        unsigned long check_sum;
	wchar_t dll_name[128];
        unsigned char buff[];
};
#pragma pack(pop)
/*---------------------------------------------------------*/
//SOCKET_CMD_CHKSUM_METHOD_CNT = 2600,	/* checksum method tbl count */
//SOCKET_CMD_CHKSUM_METHOD_TBL,		/* checksum method tbl */

#pragma pack(push, 1)

struct chksum_method_tbl_res {
	int cnt;
	wchar_t method[][64];
};

#pragma pack(pop)

/*---------------------------------------------------------*/
//SOCKET_CMD_GET_SPEC_PATH
#pragma pack(push, 1)

struct get_desktop_path_param {
        wchar_t name[64];
};

struct get_desktop_path_res {
        wchar_t path[_MAX_PATH];
};

#pragma pack(pop)
/*---------------------------------------------------------*/
#pragma warning(pop)


#endif
