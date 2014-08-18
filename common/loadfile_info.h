#ifndef _LOADFILE_INFO_H
#define _LOADFILE_INFO_H

struct loadfile_du {
        unsigned long           img_index;
	/* memory partition, like Boot0, Boot1, User Area */
	wchar_t                 memory_partition[64];
	/* image file path */
	wchar_t                 image_file_path[MAX_PATH];
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
	wchar_t                 file_path[MAX_PATH];
	unsigned long long      checksum;
};

#endif
