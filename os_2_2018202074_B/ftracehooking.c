#include "ftracehooking.h"

void ** syscall_table;	//save system call table
void *real_ftrace;	//save original ftrace function
char file_name[512];	//save file name
int pid;		//save process number
int cur_pid = 0;
int opennum, readnum, writenum, lseeknum, closenum = 0; //number of call 
size_t readlen, writelen = 0;	//save read write byte

EXPORT_SYMBOL(cur_pid);		//use iotracehooking.c file function

EXPORT_SYMBOL(file_name);	

EXPORT_SYMBOL(opennum);
EXPORT_SYMBOL(readnum);
EXPORT_SYMBOL(writenum);
EXPORT_SYMBOL(lseeknum);
EXPORT_SYMBOL(closenum);

EXPORT_SYMBOL(readlen);
EXPORT_SYMBOL(writelen);

#define __NR_ftrace 336		//system function call number

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

static asmlinkage int ftrace(const struct pt_regs *regs){//hijack function
	pid = regs->di;	//take current pid
	if(pid !=0){			//init every variable
		cur_pid = (int)pid;	
		printk("OS Assignment 2 ftrace [%d] Start \n", pid);
		opennum = 0;
		readnum = 0;
		writenum = 0;
		lseeknum = 0;
		closenum = 0;
		readlen = 0;
		writelen = 0;
	}
	else{	//print contents and end of function
		printk("[2018202074] /%s file[%s] stats [x] read - %lu / written - %lu ", get_current()->comm, file_name, readlen, writelen);
		printk("open[%d] close[%d] read[%d] write[%d] lseek[%d]\n", opennum, closenum, readnum, writenum, lseeknum);
		printk("OS Assignment 2 ftrace [%d] End\n", cur_pid);
		cur_pid = 0;
	}
	return 0;
}
static int __init hooking_init(void)	//take and save original system call
{
	syscall_table = (void **) kallsyms_lookup_name("sys_call_table");
	//save system call table
	make_rw(syscall_table);
	real_ftrace = syscall_table[__NR_ftrace];//save original function
	syscall_table[__NR_ftrace] = ftrace;
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
