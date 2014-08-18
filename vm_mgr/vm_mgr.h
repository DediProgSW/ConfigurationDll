#ifndef _VM_MGR_H
#define _VM_MGR_H

#ifdef VM_MGR_EXPORTS
#define VM_MGR_API __declspec(dllexport)
#else
#define VM_MGR_API 
#endif

typedef unsigned long long      vm_ptr_t;

VM_MGR_API      vm_ptr_t vm_alloc(unsigned long long size);
VM_MGR_API      int vm_write(vm_ptr_t pointer, const unsigned char *buff, unsigned long long size);
VM_MGR_API      int vm_read(unsigned char *buff, vm_ptr_t ptr, unsigned long long size);

VM_MGR_API	unsigned char *vm_pointer(vm_ptr_t ptr, unsigned long size);
VM_MGR_API	void vm_pointer_release(vm_ptr_t ptr, unsigned long size);
VM_MGR_API	vm_ptr_t vm_set(vm_ptr_t ptr, unsigned long long size, unsigned char *fill, int fill_len);

VM_MGR_API      int vm_free(vm_ptr_t ptr);


#endif // !_VM_MGR_H
