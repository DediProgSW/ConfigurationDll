#ifndef _PROG_INFO_H
#define	_PROG_INFO_H

/* struct chip_info_c configuration */
#define CHIPINFO_PTN_CNT                16
#define CHIPINFO_TYPE_LEN               20
#define	CHIPINFO_PN_LEN	                50
#define CHIPINFO_PTN_LEN                64
#define CHIPINFO_MFG_LEN                30
#define CHIPINFO_DESC_LEN               20
#define UNIQUEKEY_THIRD_NAME_LEN        64

typedef enum {
	PROG_MASTERU4,
	PROG_MASTERF4,
	PROG_MASTERU8,
	PROG_MASTERF8,
	PROG_STAR_U,
        PROG_STAR_F,
        PROG_STAR_ATE,
	PROG_INVALID,
} prog_type_t;

struct chip_rough_info {
        int     family_index;
	wchar_t ic_type[16];
	wchar_t mfg[30];
	wchar_t part_nr[50];
};

struct prog_dev {
	prog_type_t type;
	int usb_index;
	unsigned char ver[3];
        unsigned char sn[16];
};

struct contact_c {
	unsigned short pin_mask[4];
	unsigned long pin_max_volt;
	unsigned long pin_min_volt;
};

struct socket {
        unsigned long id;
        wchar_t adaptor[64];
};

struct partition_c {
	wchar_t display_name[64];
	wchar_t type[20];
        int           partition_index;
	unsigned long long start_program_addr;
	unsigned long long size_in_bytes;
	unsigned long page_size_in_byte;
	unsigned long block_size_in_byte;
	unsigned long sector_size_in_byte;
	unsigned long spare_area_size_in_byte;
	unsigned long blank_value;
	unsigned long init_parameter;
	unsigned long chip_erase_parameter;
	unsigned long sector_erase_parameter;
	unsigned long read_parameter;
	unsigned long program_parameter;
	unsigned long unprotect_parameter;
	unsigned long protect_parameter;
	unsigned long option_cmd1;
	unsigned long option_cmd2;
	unsigned long timing;
	unsigned long rev[4];
	wchar_t rev_str[4][20];
        unsigned long buff_scale;
};

struct chip_info_c {
	wchar_t part_name[CHIPINFO_PN_LEN];
	wchar_t type[16];
	wchar_t manufacturer[30];
	wchar_t description[20];
	unsigned long socket_info_cnt;
	struct socket socket_info[3]; 
	unsigned long chip_vcc;
	unsigned long buffer_vcc;
	unsigned long chip_vpp;
	unsigned long bus_width;
	unsigned long erase_time_out;
	unsigned long program_time_out;
	unsigned long read_time_out;
	unsigned short power_pin[2];
	unsigned long flash_id_nr;
	unsigned char flash_id[8];
	unsigned long contact_test_nr;
	struct contact_c chip_contact[5];
	wchar_t fpga_fw_x4[30];
	wchar_t fpga_fw_x8[30];
	wchar_t fpga_fw_starprog[30];
	wchar_t fpga_universal_cartridge_x4[30];
	wchar_t fpga_universal_cartridge_x8a[30];
	wchar_t fpga_universal_cartridge_x8b[30];
	wchar_t vector_file[30];
	wchar_t ram_function_file[30];
	wchar_t ram_function_transfer_dll[30];
	wchar_t ic_setting_dlg_dll[30];
	wchar_t operate_dll[30];
	unsigned long partition_cnt;
	struct partition_c partition_info[CHIPINFO_PTN_CNT];
};

#endif // !_PROG_INFO_H
