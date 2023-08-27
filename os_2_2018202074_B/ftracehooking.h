#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init_task.h>

#define __NR_read 0		//write system call table number
#define __NR_write 1
#define __NR_open 2
#define __NR_close 3
#define __NR_lseek 8
#define __NR_ftrace 336


