#ifndef _PATH_H
#define _PATH_H

#define PATH_API        extern "C" __declspec(dllexport)

#ifdef  UNICODE 

#define path_configfile(file_name, path)        path_configfile_w(file_name, path)
#define path_fpga(file_name, path)              path_fpga_w(file_name, path)
#define path_vector(file_name, path)            path_vector_w(file_name, path)
#define path_ramfunction(file_name, path)       path_ramfunction_w(file_name, path)
#define path_dll(file_name, path)               path_dll_w(file_name, path)
#define path_datafile(file_name, path)          path_datafile_w(file_name, path)

PATH_API int path_configfile_w(const wchar_t *file_name, wchar_t path[MAX_PATH]);
PATH_API int path_fpga_w(const wchar_t *file_name, wchar_t path[MAX_PATH]);
PATH_API int path_vector_w(const wchar_t *file_name,  wchar_t path[MAX_PATH]);
PATH_API int path_ramfunction_w(const wchar_t *file_name,  wchar_t path[MAX_PATH]);
PATH_API int path_dll_w(const wchar_t *file_name, wchar_t path[MAX_PATH]);
PATH_API int path_datafile_w(const wchar_t *file_name, wchar_t path[MAX_PATH]);

#else

#define path_configfile(file_name, path)        assert(0)
#define path_fpga(file_name, path)              assert(0)
#define path_vector(file_name, path)            assert(0)
#define path_ramfcuntion(file_name, path)       assert(0)
#define path_dll(file_name, path)               assert(0)
#define path_data(file_name, path)              assert(0)



#endif

#endif // !_PATH_H
