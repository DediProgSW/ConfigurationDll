#ifndef _LOADFILE_INFO_H
#define _LOADFILE_INFO_H

#include <stdlib.h>
#include <vector>
#include "../vm_mgr/vm_mgr.h"

#define LOADFILE_FILECHECKSUM_LEN               64
#define	LOADFILE_DUCRC_LEN			64
#define	LOADFILE_ECC_LEN			64
#define LOADFILE_FILEFORMAT_LEN                 64

struct loadfile_du {
	unsigned long           img_index;
	/* memory partition, like Boot0, Boot1, User Area */
	wchar_t                 ptn_name[64];
	/* image file path */
	wchar_t                 image_file_path[_MAX_PATH];
	unsigned long long      ptn_addr;
	long long               data_offset_in_buff;
	/* image length, it should be aligned image_align_size*/
	long long               image_length;
	/* actual data length */
	long long               program_length;
	/* data crc value */
	long long               data_crc_value;
	/* data check sum value */
	long long               data_checksum_value;
	/* image aligned (image_aligned_size) crc */
	long long               du_crc;
	/* follow the last image */
	int                     follow_last_img;
	/* E B P V NRE */
	unsigned short          operate[2];
	unsigned long           ptn_index;
	struct {
		const unsigned char     *buff;
		unsigned long           len;
	} operation_param;
};

/* loadfile file checksum */
struct loadfile_fcs {
	wchar_t                 file_path[_MAX_PATH];
	unsigned long long	checksum;
};

#pragma warning(push)
#pragma warning(disable:4200)

struct loadfile_item {
	unsigned long len;
	unsigned char buff[0];
};

#pragma warning(pop)

typedef int (loadfile_aynalyze_init_t) (void);
typedef	int (loadfile_aynalyze_file_t) (const wchar_t *path, unsigned long long ptn_addr, unsigned long long file_offset, std::vector<unsigned char> *addr_info);
typedef unsigned long (loadfile_aynalyze_read_t) (unsigned char *buf_out, unsigned long buf_len, unsigned long long ptn_addr, unsigned long long *file_offset);
typedef unsigned long (loadfile_aynalyze_checksum_t)(unsigned long(*checksum_func)(const unsigned char *data, unsigned long len));
typedef int (loadfile_aynalyze_release_t) (void);

struct loadfile_file_analyze_funcs {
	loadfile_aynalyze_init_t	*init;
	loadfile_aynalyze_file_t	*file;
	loadfile_aynalyze_read_t	*read;
	loadfile_aynalyze_checksum_t	*checksum;
	loadfile_aynalyze_release_t	*release;
};

typedef unsigned long (loadfile_file_checksum_t) (const unsigned char *buff, unsigned long size);
typedef unsigned long (loadfile_chip_checksum_t)(const unsigned char *buff, unsigned long size);
typedef unsigned long (loadfile_du_checksum_t) (const unsigned char *buff, unsigned long size);
typedef unsigned long (loadfile_du_crc_t) (const unsigned char *buff, unsigned long len);
typedef unsigned char *(loadfile_ecc_t) (
	const unsigned char *data_in,
	unsigned char *data_out,        /* the image buffer */
	unsigned long page_size,        /* page size */
	unsigned long block_size,       /* block size */
	unsigned long spare_area_size,  /* spare_area_size */
	unsigned long data_in_buff_len,
	unsigned long data_out_buff_len /* the data_out last space, so that if it's space not enough, this function will assert(0)*/
	);
typedef int (loadfile_reg_handler_t) (
	const struct chip_info_c        *chip,
	const unsigned char             *data_in,
	unsigned char                   *data_out,
	unsigned long                   data_out_size
	);

typedef int (loadfile_progress_t)(
	int total,
	int current,
	int res,
	int takes
	);
/**
* @brief
*       analyze the image file info, such as start addr, program length
* @param
*       path --> the image file path
*       buff --> the buffer to receive the image info, if (buff == NULL) return the buff size needed
*       buff_size --> specify the buff size,  if (buff size is smaller than the real need size) return the buff size need
* @return
*       the negative is error code(such as -10, 10 is the error code), zero is success, larger than zero is the buff size it need
*/
typedef int (img_info_t)(const wchar_t *path, unsigned char *buff, unsigned long buff_size);

struct loadfile_funcs {
	int(*get_analyze_func) (const wchar_t *method, struct loadfile_file_analyze_funcs *func);
	loadfile_file_checksum_t        *(*get_file_checksum_func) (const wchar_t *method);
	loadfile_ecc_t                  *(*get_ecc_func) (const wchar_t *method);
	loadfile_du_checksum_t          *du_checksum;
	loadfile_du_crc_t               *du_crc;
	loadfile_reg_handler_t          *reg_handler_func;
	loadfile_progress_t             *progress;
};

typedef vm_ptr_t(loadfile_img_to_du_t) (
	const struct chip_info_c *chp,
	unsigned long max_du_size,
	unsigned long du_align_size,
	unsigned char *img,
	unsigned long img_size,
	const struct loadfile_funcs *funcs,
	std::vector<struct loadfile_du> &du_tbl,
	std::vector<unsigned char > &reg_tbl,
	std::vector<struct loadfile_fcs> &file_cs_tbl,
	unsigned long                   *chip_checksum
	);

#endif
