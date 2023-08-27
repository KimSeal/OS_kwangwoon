#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>

#include <asm/syscall_wrapper.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/init_task.h>

#include <linux/sched/mm.h>
#include <linux/string.h>

#define __NR_ftrace 336

void ** syscall_table;	//save system call table
void *real_ftrace;	//save original ftrace function

void make_rw(void *addr) //function to read and write
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);
	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}

void make_ro(void *addr)//function to cancel (read and write)
{
	unsigned int level;
	pte_t * pte = lookup_address((u64)addr, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
}

__SYSCALL_DEFINEx(1,file_varea, pid_t, pid){//Hooking function(name: file_varea)
	int i;
	struct task_struct *task1;	//take task_struct based pid
	task1 = pid_task(find_vpid(pid), PIDTYPE_PID);

	struct mm_struct *mm;	//mm_struct to data/code/heap address of process
	mm= get_task_mm(task1);

	struct vm_area_struct *vm;	//process name and memory address
	vm = mm->mmap;

	unsigned long mem_start;	//variable to save address of memory, data, code, heap
	unsigned long mem_end;

	unsigned long data_start;	
	unsigned long data_end;
	data_start = mm->start_data;
	data_end = mm->end_data;
	
	unsigned long code_start;
	unsigned long code_end;
	code_start = mm->start_code;
	code_end = mm->end_code;

	unsigned long heap_start;
	unsigned long heap_end;
	heap_start = mm->start_brk;
	heap_end = mm->brk;

	int map_count;
	map_count = mm->map_count;
	
	char * path;
	printk("######## Loaded files of a process '%s(%d)' in VM ########\n", task1->comm,pid);
	//print process name & pid
	for(i =0; i < map_count && vm != NULL; i++){	//check all them
		path = vm->vm_file->f_path.dentry->d_iname;	//take path of file
		mem_start = vm->vm_start;			//memory address check
		mem_end = vm->vm_end;

		if(vm->vm_file != NULL){	//print information
			printk("mem(%lx~%lx) code(%lx~%lx) data(%lx~%lx) heap(%lx~%lx) %s\n",
			mem_start, mem_end, code_start, code_end, data_start, data_end, heap_start, heap_end, path);
		}
		vm = vm->vm_next;		//go to next
	}
	printk("###########################################################\n");
	return 0;
}

static int __init hooking_init(void)	//take and save original system call
{
	syscall_table = (void **) kallsyms_lookup_name("sys_call_table");
	//save system call table
	make_rw(syscall_table);
	real_ftrace = syscall_table[__NR_ftrace];//save original function
	syscall_table[__NR_ftrace] = __x64_sysfile_varea;
	return 0;
}
static void __exit hooking_exit(void)	//return to original function
{
	syscall_table[__NR_ftrace] = real_ftrace;//return original function
	make_ro(syscall_table);
}
module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
