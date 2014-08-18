#ifndef _OPERATION_DLL_H
#define _OPERATION_DLL_H

enum op_dll {
        OP_DLL_READ = 0,
        OP_DLL_WRITE,
};

typedef int(register_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          reg_addr,
        unsigned short          *data,
        const void              *func_param
        );

typedef int(sram_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          address,
        unsigned int            *data,
        unsigned int            data_size,
        const void              *func_param
        );

typedef int(fifo_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned char           *cvpData,
        unsigned int            uiLength,
        const void              *func_param
        );


#endif // !_OPERATION_DLL_H
