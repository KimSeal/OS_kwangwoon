#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>

#include <asm/syscall_wrapper.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init_task.h>

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

pid_t process_tracer(const struct pt_regs *regs){//hijack function
	int pid0 = regs->di;
	struct task_struct *task1;		//to search input process
	struct task_struct *task;		//take input process

	struct task_struct *sibling_task;	//sibling process take
	struct task_struct *child_task;		//children process take
	struct task_struct *parent_task;	//parent process take
	struct list_head * list;		//save list of sibling & children
	struct list_head * list2;
	
	for_each_process(task1){		//find input process
		if((int)task1->pid == pid0){	//same pid search
			task = task1;		//take process
		}
	}
	int count=0;				//process count(sibling, children)
	if(task == NULL){			//if no correct information
		return -1;
	}

	printk("[os2018202074] ##### TASK INFORMATION of ''[%d] %s '' #####\n", pid0, task->comm);//process id and name
	
	int statecheck = task->state;	//state check
	if(statecheck == 0){		//and print current state
		printk("[os2018202074] - task state : Running or ready\n");
	}
	else if(statecheck == 2){
		printk("[os2018202074] - task state : Wait with ignoring all signals\n");
	}
	else if(statecheck == 1){
		printk("[os2018202074] - task state : Wait\n");
	}
	else if(statecheck == 4){
		printk("[os2018202074] - task state : Stopped\n");
	}
	else if(statecheck == 32){
		printk("[os2018202074] - task state : Zombie process\n");
	}
	else if(statecheck == 16){
		printk("[os2018202074] - task state : Dead\n");
	}
	else{
		printk("[os2018202074] - task state : etc\n");
	}
	//print group leader
	printk("[os2018202074] - Process Group Leader : [%d] %s\n", task->group_leader->pid, task->group_leader->comm);
	//print context switch number
	printk("[os2018202074] - Number of context switches : %ld\n", task->nivcsw);
	//print fork number(use sched & fork file)
	printk("[os2018202074] - Number of calling fork() : %d\n", task->fork_count);
	//print about parent process
	printk("[os2018202074] - it's parent process(es) : [%d] %s\n", task->parent->pid, task->parent->comm);
	//print about sibling
	printk("[os2018202074] - it's sibling process(es) :\n");
	//make list for sibling
	list = &(task->sibling);	
	//check all sibling nodes
	list_for_each(list, &task->sibling){
		sibling_task = list_entry(list, struct task_struct, sibling);
		if(sibling_task->pid != 0){//delete self case
			printk("[os2018202074]    > [%d] %s\n", sibling_task->pid, sibling_task->comm);
			count++;
		}
	}

	if(count == 0){	//if no sibling
		printk("[os2018202074]    > It has no sibling.\n");	
	}
	else if(count > 0){//if have sibling, print them
		printk("[os2018202074]    > This process has %d sibling process(es)\n", count);	
	}
	printk("[os2018202074] - it's child process(es) :\n");
	// initialize count to check number of children
	count =0;
	//make list for children
	list2 = &(task->children);
	//check all children nodes
	list_for_each(list2, &task->children){
		child_task = list_entry(list2, struct task_struct, sibling);
		if(child_task->pid != 0){//delete self case
			printk("[os2018202074]    > [%d] %s\n", child_task->pid, child_task->comm);
			count++;
		}
	}
	
	if(count == 0){//if no children
		printk("[os2018202074]    > It has no child.\n");	
	}
	else if(count > 0){//if have children, print them
		printk("[os2018202074]    > This process has %d child process(es)\n", count);	
	}

	printk("[os2018202074] ##### END OF INFORMATION #####\n");
	//return pid
	return pid0;
}
static int __init hooking_init(void)	//take and save original system call
{
	syscall_table = (void **) kallsyms_lookup_name("sys_call_table");
	//save system call table
	make_rw(syscall_table);
	real_ftrace = syscall_table[__NR_ftrace];//save original function
	syscall_table[__NR_ftrace] = process_tracer;
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
