#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/mman.h>

#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>

#include <sys/time.h>	//calculate time
#include <time.h>
uint8_t* Operation;
uint8_t* compiled_code;

void sharedmem_init(); //shared memory initialize
void sharedmem_exit();	// shared memory free
void drecompile_init(); // to compiled_code initialize
void drecompile_exit(); // to compiled_code free
void* drecompile(uint8_t *func); //func file make(asm array)

uint8_t * shared_memory;//take place to save shared memory

int len=0;		//length of asm array	

int pagesize;	//page size
int k=0;	//save Operation place
int f=0;	//save compiled_code place

int main(void)
{
	int (*func)(int a);
	int i;

	sharedmem_init();
	drecompile_init(shared_memory);

	func = (int (*)(int a))drecompile(Operation);
	
	struct timespec startTime, endTime;	//variable to save time
	long diffTime=0;				//total run time of func(1)
	long subdiffTime=0;			//each run time of func(1)

	int result=0;
	int subresult=0;
	
	//for(int l=0;l<50;l++){
		system("sync");
		system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
		clock_gettime(CLOCK_MONOTONIC, &startTime);//start of time
		result=func(1);
		clock_gettime(CLOCK_MONOTONIC, &endTime);//end of time
		subdiffTime=1000000000 * (endTime.tv_sec - startTime.tv_sec)+(endTime.tv_nsec-startTime.tv_nsec);
	//	printf("%d(th) run time: %lf\n", l,(double)subdiffTime);//print time(nano-sec)
	//	diffTime += subdiffTime;
	//}
	//printf("\ntotal run time: %lf\n", (double)diffTime);//print time(nano-sec)
	//printf("average run time: %lf\n", (double)diffTime/50);//print time(nano-sec)
	//printf("result: %d\n", result);				//check asm right

	printf("total execution time: %.8lf sec\n", (double)subdiffTime/1000000000);//print time(nano-sec)
	
	drecompile_exit();
	sharedmem_exit();
	return 0;
}

void sharedmem_init()
{
	int id_seg = shmget(1234, PAGE_SIZE, 0);	//take ID to shared memory connect
	shared_memory = (uint8_t*)shmat(id_seg, NULL, 0);//connected array
}

void sharedmem_exit()
{
	shmdt(shared_memory);			//unconnect shared memory
}

void drecompile_init(uint8_t *func)
{
	do{
	}while(shared_memory[len++] != 0xC3);	//take length of shared memory
	//fd = open("./hello.txt", O_RDWR|O_CREAT, 0777);	
	//char fun_val[5];
	pagesize = getpagesize();		//take page size
	compiled_code = (uint8_t *)mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS |MAP_SHARED, -1,0);
	//compiled_code take memory by using mmap
	k=0;
	do{ 	compiled_code[k] = shared_memory[k];
		//compiled code take shared memory's data 
	}while(k++<len);

	msync(compiled_code, pagesize, MS_ASYNC);
	//map synchronize
}

void drecompile_exit()
{
	munmap(compiled_code, pagesize);	//free mmaped variable
	munmap(Operation, len);
	//free mapped Operation
}

void* drecompile(uint8_t* func)
{

#ifdef dynamic		//when make dynamic
	pagesize = getpagesize();	//get page size
	Operation = (uint8_t *)mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1,0);
	//map Operation to take asm data(read and write)
	k=0;	//Operation place
	f=0;	//Compiled_code place
	int can_1=0;	//save current function
	int can_2=0;	
	int can_3=0;	//save current value
	do{
		if(can_1 == compiled_code[f] && can_2 ==compiled_code[f+1]){//if instruction same
			if(can_1 == 0xf6 && can_2 == 0xf2){ //div case	
				can_3 *= 2;	//put square of dl value(2) to reduce number of div
				f += 2;		//read control
			}
			else if(can_1 == 0x83 && can_2 == 0xc0){ //add case
				can_3 += compiled_code[f+2];//add value to reduce number of add
				f += 3;
			}
			else if(can_1 == 0x83 && can_2 == 0xe8){ //sub
				can_3 += compiled_code[f+2];//add value to reduce number of sub
				f += 3;
			}
			else if(can_1 == 0x6b && can_2 == 0xc0){ //imul
				can_3 *= compiled_code[f+2]; //multiply value to reduce number of imul
				f += 3;
			}
		}
		else if(compiled_code[f] != 0x83 && compiled_code[f] != 0x6b && compiled_code[f] != 0xf6){
			//no add/sub/imul/div operation
			if(can_1 != 0x00 && can_2 != 0x00){	//if current operation exist
 				if(can_1 == 0xf6 && can_2 == 0xf2){	//div case
					if(can_3 >= 4){			//(when overlap div exist over two time)
						Operation[k] = 0xb2;	//put current value in dl register
						Operation[k+1] = can_3;
						k+=2;			//write control
					}
					Operation[k] = can_1;		//write operation type in Operation array
					Operation[k+1] = can_2;		
					k+=2;
					if(can_3 >= 4){			//change dl register to original value
						Operation[k] = 0xb2;
						Operation[k+1] = 0x02;
						k+=2;
					}
				}				
				else{				//add or sub or imul case
					Operation[k] = can_1;	//put operation and value
					Operation[k+1] = can_2;
					Operation[k+2] = can_3;
					k+=3;
				}
			}

			Operation[k] = compiled_code[f];	//put no add/sub/imul/div value
			k++;
			f++;
			can_1=0;				//empty current function
			can_2=0;
			can_3=0;
		}
		else{				//add/sub/imul/div but that is not current running-function
			if(can_1 != 0x00 && can_2 != 0x00){	//current function is not empty
				if(can_1 == 0xf6 && can_2 == 0xf2){//div case
					if(can_3 >= 4){
						Operation[k] = 0xb2;
						Operation[k+1] = can_3;
						k+=2;
					}
					Operation[k] = can_1;
					Operation[k+1] = can_2;
					k+=2;
					if(can_3 >= 4){
						Operation[k] = 0xb2;
						Operation[k+1] = 0x02;
						k+=2;
					}
				}				
				else{	//add sub imul case
					Operation[k] = can_1;
					Operation[k+1] = can_2;
					Operation[k+2] = can_3;
					k+=3;
				}
			}

			can_1 = compiled_code[f];	//update current function
			can_2 = compiled_code[f+1];
			if(can_1 == 0xf6 && can_2 == 0xf2){//when div, we need to set can_3 to dl register
				can_3=2;
				f+=2;
			}
			else{
				can_3=compiled_code[f+2];
				f+=3;
			}

		}
	}while(compiled_code[f] != 0xC3);	//check asm array's end

	if(can_1 != 0x00 && can_2 != 0x00){	//before asm array is end, we need to update current function
		if(can_1 == 0xf6 && can_2 == 0xf2){//when div case
			if(can_3 >= 4){		//div time is over two
				Operation[k] = 0xb2;	//change dl register value to can_3
				Operation[k+1] = can_3;
				k+=2;
			}
			Operation[k] = can_1;		//do div
			Operation[k+1] = can_2;
			k+=2;
			if(can_3 >= 4){			//change dl register value to original value
				Operation[k] = 0xb2;
				Operation[k+1] = 0x02;
				k+=2;
			}
		}				
		else{				//add sub imul case
			Operation[k] = can_1;	//put Operation and value
			Operation[k+1] = can_2;
			Operation[k+2] = can_3;
			k+=3;
		}
	}
	Operation[k]=compiled_code[f];		//put end of asm code(0xC3)

	mprotect(Operation, pagesize, PROT_NONE);
	mprotect(Operation, pagesize, PROT_READ | PROT_EXEC);//change authority to exec
	msync(Operation, pagesize, MS_ASYNC);	//syncronize map update
	return Operation;			//return to use function

#else		//when make
pagesize = getpagesize();			//make page size

	Operation = (uint8_t *)mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1,0);
	//put memory Operation (read write)
	k=0;
	do{	Operation[k]=compiled_code[k];
		//put all raw asm Operation in Operation array
	}while(k++<len-1);
	mprotect(Operation, pagesize, PROT_NONE);
	mprotect(Operation, pagesize, PROT_READ | PROT_EXEC);//change authority to exec
	msync(Operation, pagesize, MS_ASYNC);	//syncronize map update
	return Operation;//return to use function
		
#endif

}

