#ifndef	_COMM_TYPE_H
#define	_COMM_TYPE_H

#include "loadfile_info.h"
#include "../vm_mgr/vm_mgr.h"

typedef unsigned char	u8;
typedef signed char	s8;

typedef unsigned short	u16;
typedef signed short	s16;

typedef unsigned long	u32;
typedef signed long	s32;

/* progress function prototype */
typedef int (progress_func_t) (int usb_index, int site_nr, int total, int current, int op_type, int err_code, unsigned int take_time);
typedef int (res_t) (int usb_index, int site_nr, int op_type, int err_code, unsigned long takes);
typedef int (ram_down_t) (int usb_index, const unsigned char *p, unsigned long size, void *param);
typedef int (ram_config_t) (const struct chip_info_c *chip, unsigned char *buff, unsigned long size);
/* operation function prototype */
typedef int(usb_reg_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          reg_addr,
        unsigned short          *data,
        void                    *param
        );

typedef int(usb_sram_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          address,
        unsigned char           *data,
        unsigned long           data_size,
        void                    *param
        );

typedef int(usb_fifo_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned char           *data,
        unsigned long           len,
        void                    *param
        );


typedef int (prepare_init_t)(
                const struct chip_info_c       *chip,
                usb_reg_func_t         *reg_func,
                usb_sram_func_t        *sram_func,
                usb_fifo_func_t        *fifo_func,
                const void             *func_param
                );

typedef int (read_reg_t)(
                const struct chip_info_c        *chip,
                struct loadfile_item            *reg,
                int (*read) (
                        const struct chip_info_c *chip,
                        unsigned char *buff,
                        int len,
                        void *param
                        ),
                void                            *read_param
                );

typedef int (du_to_reg_t)(
                const struct chip_info_c                *chip,                  /* chip info structure */
                std::vector<struct loadfile_du>         &du_header,             /* du header */
                vm_ptr_t                                buff,
                struct loadfile_item                    **reg,
                unsigned long                           *chip_checksum,         /* loadfile --> chipchecksum */
                std::vector<struct loadfile_fcs>        &file_checksum,          /* file checksum */
                const struct loadfile_funcs             *funcs
                );
                
typedef int (final_du_t)(
                const struct chip_info_c                *chip,
                std::vector<struct loadfile_du>         &du_header,
                vm_ptr_t                                buff,
                loadfile_item                           **reg,
                unsigned long                           *chip_checksum,
                std::vector<struct loadfile_fcs>        &file_checksum,
                const struct loadfile_funcs             *funcs
                );
 
/* unique key function type */
typedef int (uniquekey_pv_t)(int usb_index, int site_index, int ptn_index, unsigned long page_size, unsigned long long start_addr, const unsigned char *buff,  unsigned long len);
typedef int (uniquekey_curr_t)(int usb_index, int site_index, const wchar_t *data);
typedef int (uniquekey_erase_data_t)(const wchar_t *ptn_name, unsigned long long start_addr, unsigned long len);

#endif
