// vm_mgr.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <Windows.h>
#include <vector>
#include <assert.h>
#include <tchar.h>
#include "../common/assist.h"
#include"../error_no/error_no.h"
#include "../Log/log.h"
#include "vm_mgr.h"

#define VM_ADDR_GEN(addr, index)                (assert(index < 16), (((index & 0xfULL) << 60) | addr))
#define VM_START_ADDR                           0x100000000ULL
#define VM_WINDOW_SIZE                          (64 * 1024 * 1024UL)
#define VM_ALIGN_SIZE                           (64 * 1024)

struct vm_window {
        void                    *p;
        vm_ptr_t                start;
        unsigned long           size;
};

struct vm_item {
        int                 addr_index;
        HANDLE              mutext;    
        HANDLE              hFile;
        HANDLE              hMap;
        vm_ptr_t            vm_start_ptr;
        unsigned long long  vm_size;
        struct vm_window    window;
        TCHAR               file_path[MAX_PATH];
};

struct {
    std::vector<struct vm_item>    tbl;
} vm;

static int find_empty_index()
{
        int addr_index;
        unsigned int i;

        for (addr_index = 0; ; addr_index++) {
                for (i = 0; i < vm.tbl.size(); i++) {
                        if (vm.tbl.at(i).addr_index == addr_index)
                                break;
                }
                assert(addr_index <= 16);
                if (i >= vm.tbl.size())
                        return addr_index;
        }
}

vm_ptr_t vm_alloc(unsigned long long size)
{
        int             len;
        TCHAR           drive[4];
        TCHAR           tmp_path[MAX_PATH];
        TCHAR           tmp_file_path[MAX_PATH];
        ULARGE_INTEGER  total, free;
        struct vm_item  vm_item;

        len = GetModuleFileNameW(NULL, tmp_file_path, MAX_PATH);
	while (len--) {
		if (tmp_file_path[len] == L'\\')
			break;
	}
	tmp_file_path[len] = L'\0';
        _tcscat(tmp_file_path, L"\\server.ini");

        GetPrivateProfileString(_T("TempFolder"), _T("PATH"), NULL, tmp_path, ARRAY_SIZE(tmp_path), tmp_file_path);

        if (tmp_path[0] == 0)
                GetTempPath(ARRAY_SIZE(tmp_path), tmp_path);

        drive[0] = tmp_path[0];
        drive[1] = tmp_path[1];
        drive[2] = 0;
        
        GetDiskFreeSpaceEx(drive, &free, &total, NULL);
        if (free.QuadPart <= size) {
		log_add(__FUNCTION__":disk free:%lld byte, total %lld byte", free.QuadPart, total.QuadPart);
                errno = E_NO_STORAGE_SPACE;
                return NULL;
        }

        GetTempFileName(tmp_path, _T("DediProg"), 0, tmp_file_path);

        HANDLE hFile = CreateFile(
                                tmp_file_path,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL
                                );
        if (hFile == NULL) {
                return NULL; 
        }

        vm_item.addr_index = find_empty_index();
        if (vm_item.addr_index >= 16) {
		CloseHandle(hFile);
                log_debug_add(__FUNCTION__":find no empty index");
                errno = E_ALLOC_MEMORY_FAIL;
                return NULL;
        }

        vm_item.hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, size >> 32, size & 0xffffffff, NULL);
        if (vm_item.hMap == NULL) {
                errno = E_ALLOC_MEMORY_FAIL;
		CloseHandle(hFile);
                return NULL;
        }

        vm_item.vm_start_ptr = (vm_ptr_t)MapViewOfFile(
                vm_item.hMap,
                FILE_MAP_ALL_ACCESS,
                0ULL >> 32,
                0ULL & 0xffffffff,
                (SIZE_T)min(size, VM_WINDOW_SIZE)
                );
 
        
        vm_item.vm_start_ptr = VM_ADDR_GEN(vm_item.vm_start_ptr, vm_item.addr_index);

        assert(vm_item.vm_start_ptr < ((-1) >> 4));

        vm_item.hFile = hFile;
        vm_item.vm_size = size;
        vm_item.window.size = (unsigned long)min(VM_WINDOW_SIZE, size);
        vm_item.window.start = vm_item.vm_start_ptr;
        vm_item.window.p = (unsigned char *)vm_item.vm_start_ptr;
        vm_item.mutext = CreateMutex(NULL, FALSE, NULL);
        _tcsncpy(vm_item.file_path, tmp_file_path, ARRAY_SIZE(vm_item.file_path));
        vm.tbl.push_back(vm_item);

        log_debug_add(__FUNCTION__":index:%d, ptr = 0x%llx", vm_item.addr_index, vm_item.vm_start_ptr);

        return vm_item.vm_start_ptr;
}

static __inline int in_region(vm_ptr_t ptr, unsigned long long size)
{
        //static int dbg = 0;
        int i;
 
        for (i = 0; i < (int)vm.tbl.size(); i++) {
                if ((ptr + size) <= (vm.tbl.at(i).vm_start_ptr + vm.tbl.at(i).vm_size) && (ptr >= vm.tbl.at(i).vm_start_ptr))
                        return i;
                //if (ptr + size > vm.tbl.at(i).vm_start_ptr + vm.tbl.at(i).vm_size)
                //        i = 'DEAD';
                //if (ptr < vm.tbl.at(i).vm_start_ptr)
                //        i = 'INVA';
        }
        
        LOG_BEGIN {
                log_add("----------------------------------------------");
                log_add("|             Virtual Memory layout          |");
                log_add("|--------------------------------------------|");
                for (i = 0; i < (int)vm.tbl.size(); i++)
                log_add("|window.start_addr:%llx | window.size:%llx   |", vm.tbl.at(i).window.start, vm.tbl.at(i).window.size);
                log_add("|--------------------------------------------|");
                log_add("|user.ptr:%08llx  | user.size:%llx     |", ptr, size);
                log_add("----------------------------------------------");
        } LOG_END;
        //dbg++;
        log_emerge_add(__FUNCTION__":invlid parameter:0x%llx, 0x%llx", ptr, size);
        assert(!"Invalid Memory");
        return -1;
}

static __inline unsigned char *scroll_window(int vm_index, vm_ptr_t ptr, unsigned long size)
{
        unsigned char           *p;
        vm_ptr_t            win_start;
        unsigned long           win_size;
        unsigned long long      file_offset;
        unsigned long long      tmp_offset;


        win_start = vm.tbl.at(vm_index).window.start;
        win_size = vm.tbl.at(vm_index).window.size;

        
        UnmapViewOfFile(vm.tbl.at(vm_index).window.p);

        win_size = (unsigned long)min(VM_WINDOW_SIZE, size);

        assert(ptr >= vm.tbl.at(vm_index).vm_start_ptr);
        file_offset = ptr - vm.tbl.at(vm_index).vm_start_ptr;
        
        tmp_offset = file_offset / VM_ALIGN_SIZE * VM_ALIGN_SIZE;
        tmp_offset = file_offset - tmp_offset;
        
        file_offset = file_offset / VM_ALIGN_SIZE * VM_ALIGN_SIZE;

        size += (unsigned long)tmp_offset;

        p = (unsigned char *)MapViewOfFile(
                vm.tbl.at(vm_index).hMap,
                FILE_MAP_ALL_ACCESS,
                file_offset >> 32,
                file_offset & 0xffffffff,
                size
                );

        if (p) {
                vm.tbl.at(vm_index).window.start = ptr;
                vm.tbl.at(vm_index).window.size = size;
                vm.tbl.at(vm_index).window.p = p;

                return p + tmp_offset;
        }

        return NULL;
}

int vm_write(vm_ptr_t pointer, const unsigned char *buff, unsigned long long size)
{
        int             ret;
        unsigned char       *p;
        unsigned long       len;
        int index = in_region(pointer, size);

        ret = 0;

        if (index == -1) {
                ret = -1;
                return ret;
        }
        ret = WaitForSingleObject(vm.tbl.at(index).mutext, 20000);        /* timeout 20s */
        if (ret == WAIT_TIMEOUT) {
                log_emerge_add(__FUNCTION__": wait mutex timeout");
                return -1;
        }

        while (size) {
                len = (unsigned long)min(size, VM_WINDOW_SIZE);
                p = scroll_window(index, pointer, len);
                if (!p) {
                        log_emerge_add(__FUNCTION__": scroll window fail, 0x%llx", pointer);
                        ret = -1;
                        goto end;
                }

                memcpy(p, buff, len);

                pointer += len;
                buff += len;
                size -= len;
        }
end:
        ReleaseMutex(vm.tbl.at(index).mutext);
        return ret;
}


int vm_read(unsigned char *buff, vm_ptr_t ptr, unsigned long long size)
{
        int ret;
        unsigned char   *p;
        unsigned long   len;
        int index;

        assert(buff);

        index = in_region(ptr, size);

        log_debug_add(__FUNCTION__":vm_ptr:%llp, size:%llp, err: %d", ptr, size, 0);
        if (index == -1) {
                log_debug_add(__FUNCTION__":vm_ptr:%lp, size:%lp, err: ivalid region", ptr, size);
                return -E_ALLOC_MEMORY_FAIL;
        }

        if (WAIT_TIMEOUT == WaitForSingleObject(vm.tbl.at(index).mutext, 20000)) {
                log_debug_add(__FUNCTION__":vm_ptr:%lp, size:%lp, err: wati for mutex timeout", ptr, size);
		return -E_ALLOC_MEMORY_FAIL;        /* timeout 20s */
        }

        ret = 0;
        while (size) {
                len = (unsigned long)min(size, VM_WINDOW_SIZE);
                p = scroll_window(index, ptr, len);
                if (!p) {
                        log_emerge_add(__FUNCTION__": scroll window fail, 0x%llx", ptr);
                        ret = -E_ALLOC_MEMORY_FAIL;
                        goto end;
                }

                memcpy(buff, p, len);

                ptr += len;
                buff += len;
                size -= len;
        }
end:
        ReleaseMutex(vm.tbl.at(index).mutext);

        return ret;
}

unsigned char *vm_pointer(vm_ptr_t ptr, unsigned long size)
{
	unsigned char *p;

	int index = in_region(ptr, size);

	if (index == -1) {
		errno = E_ALLOC_MEMORY_FAIL;
		return NULL;
	}

        if (WAIT_TIMEOUT == WaitForSingleObject(vm.tbl.at(index).mutext, 20000)) {
		errno = E_ALLOC_MEMORY_FAIL;
		return NULL;        /* timeout 20s */
	}

	p = scroll_window(index, ptr, size);
	if (!p) {
		errno = E_ALLOC_MEMORY_FAIL;
                ReleaseMutex(vm.tbl.at(index).mutext);
        }
	return p;
}

void vm_pointer_release(vm_ptr_t ptr, unsigned long size)
{
	int index = in_region(ptr, size);
	if (index == -1)
		return ;

        ReleaseMutex(vm.tbl.at(index).mutext);
}

static void set_value(unsigned char *buff, unsigned long len, unsigned char *fill, unsigned long fill_len)
{
	int tmp;
	int cnt;
	int last;
	unsigned long	*b4;
	unsigned short	*b2;

	assert(buff);
	assert(fill);
	assert(len >= fill_len);

	if (fill_len == 1) {
		memset(buff, *fill, len);
		return ;
	} else if (fill_len == 4) {
		last = len % fill_len;
		cnt = len / fill_len;

		b4 = (unsigned long *)buff;
		while (cnt--)
			*b4++ = ((unsigned long *)fill)[0];
		
		memcpy(b4, fill, last);
	} else if (fill_len == 2) {
		last = len % fill_len;
		cnt = len / fill_len;

		b2 = (unsigned short *)buff;
		while (cnt--)
			*b2++ = ((unsigned short *)fill)[0];

		memcpy(b2, fill, last);
	} else {
		while (len) {
			tmp = min(fill_len, len);
			memcpy(buff, fill, tmp);

			buff += tmp;
			len -= tmp;
		}
	}
}

vm_ptr_t vm_set(vm_ptr_t ptr, unsigned long long size, unsigned char *fill, int fill_len)
{
	unsigned char   *p;
        unsigned long   len;
        int index;
	vm_ptr_t	ret;

	ret = ptr;

        index = in_region(ptr, size);
        if (index == -1) {
                assert(!"invalid pointer");
                errno = E_ALLOC_MEMORY_FAIL;
                return NULL;
        }

	if (WAIT_TIMEOUT == WaitForSingleObject(vm.tbl.at(index).mutext, 20000)) {
		errno = E_ALLOC_MEMORY_FAIL;
		return NULL;        /* timeout 10s */
	}

        while (size) {
                len = (unsigned long)min(size, VM_WINDOW_SIZE);
                p = scroll_window(index, ptr, len);
                if (!p) {
			ret = NULL;
			errno = E_ALLOC_MEMORY_FAIL;
			goto end;
		}
                set_value(p, len, fill, fill_len);

                ptr += len;
                size -= len;
        }
end:
        ReleaseMutex(vm.tbl.at(index).mutext);
	return ret;
}





int vm_free(vm_ptr_t ptr)
{
    int i;

    LOG_BEGIN {
                log_add("-----------------------------------------------------------------------");
                log_add("|                        Virtual Memory layout                        |");
                log_add("|---------------------------------------------------------------------|");
                for (i = 0; i < (int)vm.tbl.size(); i++)
                log_add("|window.start_addr:%llx                  | window.size:%llx           |", vm.tbl.at(i).window.start, vm.tbl.at(i).window.size);
                log_add("-----------------------------------------------------------------------");
    } LOG_END;

    for (i = 0; i < (int)vm.tbl.size(); i++) {
        if (vm.tbl.at(i).vm_start_ptr == ptr) {
            assert(vm.tbl.at(i).hMap);
                
            UnmapViewOfFile(vm.tbl.at(i).window.p);
            CloseHandle(vm.tbl.at(i).hMap);
            CloseHandle(vm.tbl.at(i).hFile);
            vm.tbl.erase(vm.tbl.begin() + i);

            log_debug_add(__FUNCTION__": success");

            return 0;
        }
    }
 
    log_debug_add(__FUNCTION__": free fail");

    assert(!"Invalid pointer");

    return -1;
}
