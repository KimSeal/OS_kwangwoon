#include "ftracehooking.h"

void ** syscall_table;      // system call table
extern char file_name[512]; // file name
extern int cur_pid; 	
extern int opennum, readnum,  writenum, lseeknum, closenum; //number of call
extern size_t readlen, writelen;	//save read write byte

static asmlinkage long (*real_open)(const struct pt_regs* regs);
//save original open system call function
static asmlinkage long ftrace_open(const struct pt_regs* regs){//hijack open
	if(get_current()->pid == cur_pid){
		char __user * fileread = (char*)regs->di;
		opennum +=1;	//open num check
		strncpy_from_user(file_name, fileread, sizeof(file_name));
	} 
	return real_open(regs);
}

static asmlinkage long (*real_read)(const struct pt_regs* regs);
//save original read system call function
static asmlinkage long ftrace_read(const struct pt_regs* regs){//hijack read
	if(get_current()->pid == cur_pid){
		readnum +=1;			//call number check
		readlen += (size_t)regs->dx;	//byte check
	} 
	return real_read(regs);
}

static asmlinkage long (*real_write)(const struct pt_regs* regs);
//save original write system call function
static asmlinkage long ftrace_write(const struct pt_regs* regs){//hijack write
	if(get_current()->pid == cur_pid){
		writenum +=1;			//call number check
		writelen += (size_t)regs->dx;	//byte check
	} 
	return real_write(regs);
}

static asmlinkage long (*real_lseek)(const struct pt_regs* regs);
//save original lseek system call function
static asmlinkage long ftrace_lseek(const struct pt_regs* regs){//hijack lseek
	if(get_current()->pid == cur_pid){
		lseeknum +=1;	//lseek num check
	} 
	return real_lseek(regs);
}

static asmlinkage long (*real_close)(const struct pt_regs* regs);
//save original close system call function
static asmlinkage long ftrace_close(const struct pt_regs* regs){//hijack close
	if(get_current()->pid == cur_pid){
		closenum +=1;//close num check
	} 
	return real_close(regs);
}

static int __init io_hooking_init(void){//hooking init
	syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
	//take system call init
	real_open = syscall_table[__NR_open];	//take original function
	syscall_table[__NR_open] = ftrace_open;	//and replace that
	real_read = syscall_table[__NR_read];
	syscall_table[__NR_read] = ftrace_read;
	real_write = syscall_table[__NR_write];
	syscall_table[__NR_write] = ftrace_write;
	real_lseek = syscall_table[__NR_lseek];
	syscall_table[__NR_lseek] = ftrace_lseek;
	real_close = syscall_table[__NR_close];
	syscall_table[__NR_close] = ftrace_close;
	return 0;
}
static void __exit io_hooking_exit(void){
	syscall_table[__NR_open] = real_open;//return to original function
	syscall_table[__NR_read] = real_read;
	syscall_table[__NR_write] = real_write;
	syscall_table[__NR_lseek] = real_lseek;
	syscall_table[__NR_close] = real_close;
}
module_init(io_hooking_init);
module_exit(io_hooking_exit);
MODULE_LICENSE("GPL");

