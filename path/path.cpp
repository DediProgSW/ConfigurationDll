// path.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include "path.h"

static int get_current_path_w(wchar_t path[MAX_PATH])
{
        int len;
        len = GetModuleFileNameW(NULL, path, MAX_PATH);
	while (len--) {
		if (path[len] == L'\\')
			break;
	}
	path[len] = L'\0';
	wcscat(path, L"\\ICSupport\\");

        return 0;
}

int path_configfile_w(const wchar_t *file_name, wchar_t path[MAX_PATH])
{
        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, file_name);

        return 0;
}

int path_fpga_w(const wchar_t *file_name, wchar_t path[MAX_PATH])
{

        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, L"FPGA\\");
        wcscat(path, file_name);
        
        return 0;
}

int path_vector_w(const wchar_t *file_name,  wchar_t path[MAX_PATH])
{
        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, L"Binary\\");
        wcscat(path, file_name);
       
        return 0;
}

int path_ramfunction_w(const wchar_t *file_name,  wchar_t path[MAX_PATH])
{
        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, L"Binary\\");
        wcscat(path, file_name);
        
        return 0;
}

int path_dll_w(const wchar_t *file_name, wchar_t path[MAX_PATH])
{
        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, L"Binary\\");
        wcscat(path, file_name);
        
        return 0;
}

int path_datafile_w(const wchar_t *file_name, wchar_t path[MAX_PATH])
{
        assert(file_name);
        assert(path);

        get_current_path_w(path);
        wcscat(path, L"DataFiles\\");
        wcscat(path, file_name);
 
        return 0;
}
