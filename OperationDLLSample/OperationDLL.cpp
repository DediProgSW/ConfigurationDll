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
                std::vector<struct loadfile_fcs>        &file_checksum          /* file checksum */
                )
{
        assert(chip);
        assert(reg);
        assert(chip_checksum);

        printf("\\du_to_reg\n");

        return 0;
}
                
extern "C" __declspec(dllexport)  
int final_du(
                const struct chip_info_c                *chip,
                std::vector<struct loadfile_du>         &du_header,
                vm_ptr_t                                buff,
                loadfile_item                           **reg,
                unsigned long                           *chip_checksum,
                std::vector<struct loadfile_fcs>        &file_checksum
                )
{
        assert(chip);
        assert(reg);
        assert(chip_checksum);

        printf("\\final_du\n");

        return 0;
}