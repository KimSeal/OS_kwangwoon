#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/syscall.h>
#include<time.h>
#include<math.h>

#include<sched.h>	//SCHED header
#include<sys/wait.h>	//to wait each process
#include<sys/mman.h>	//to use mmap function

#define MAX_PROCESSES 10000
#define FORK_MAX_PROCESSES 64

static double *total;	//sum of each processes run time
static int *file_num=0;

void schedtest(int type,int priority,int nice_val)
{
	char f_name[128];//variable to save file name
	int wait_val;	 //variable to wait
	int read_val;	 //variable to read value
	
	//use mmap to match address
	total = mmap(NULL, sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*total = 0;
	
	for(int i=0; i<MAX_PROCESSES; i++){	//make process about MAX_PROCCESSES	
		if(!fork()){	//make child process
			struct sched_param sched_set;
			
			//if standard_RR, we need to nice value
			if(type == SCHED_OTHER){
				sched_set.sched_priority = 0;
				sched_setscheduler(0, type, &sched_set);
				nice(nice_val);	//use nice function
			}

			//if FIFO OR RR, we need to priority
			else{
				sched_set.sched_priority = priority;
				sched_setscheduler(0, type, &sched_set);
			}
			
			struct timespec start,end;//to calculate run time
	
			clock_gettime(CLOCK_MONOTONIC, &start);//start time
						
			//open file and read&print file data, and close file
			FILE *f_read;	
			sprintf(f_name,"./temp/%d",i);

			f_read = fopen(f_name,"r");
			fscanf(f_read,"%d",&read_val);
			fclose(f_read);

			clock_gettime(CLOCK_MONOTONIC, &end);//end time

			double time = (end.tv_sec- start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1000000000.0);//take run time about this process

			*total += time; //add this time to calculate average turn around time
			exit(0);	//finish process
		}
	}

	//wait all process end
	for(int i = 0; i < MAX_PROCESSES; i++){
		wait(&wait_val);
	}

	double average = (*total)/MAX_PROCESSES; //make average turn around time
	munmap(total,sizeof(double));//free mmap

	//print average turn around time
	printf("Average turn around time about files : %lf\n", average);

}

int main()
{
	system("rm -rf tmp");
	system("sync");
	system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
	//initialize

	int type=0;	//variable to save cpu scheduling
	int prior=0;	//variable to save priority
	int nice=0;	//variable to save nice value
	struct timespec start, end;	//variable to calculate time
	
	printf("CPU scheduling(1=standard_RR 2=FIFO 3=RR) : ");//set cpu schduling
	scanf(" %d",&type);

	if(type==1){			//standard_RR case
		type=SCHED_OTHER;
		printf("NICE(-20=high 0=def 19=low) : ");//take nice value
		scanf(" %d",&nice);
	}
	else if(type==2){		//FIFO case
		type=SCHED_FIFO;
		printf("Priority(1=high 2=def 3=low) : ");
		scanf(" %d",&prior);
		if(prior==1){				//set priority value
			prior=sched_get_priority_min(type);
		}
		else if(prior==2){
			prior=(sched_get_priority_min(type)+sched_get_priority_max(type))/2;
		}
		else if(prior==3){
			prior=sched_get_priority_max(type);
		}
	}
	else if(type==3){		//RR case
		type=SCHED_RR;
		printf("Priority(1=high 2=def 3=low) : ");
		scanf(" %d",&prior);
		if(prior==1){				//set priority value
			prior=sched_get_priority_min(type);
		}
		else if(prior==2){
			prior=(sched_get_priority_min(type)+sched_get_priority_max(type))/2;
		}
		else if(prior==3){
			prior=sched_get_priority_max(type);
		}
	}	

	clock_gettime(CLOCK_MONOTONIC, &start); //start time of full processes

	schedtest(type, prior, nice);		//run schedtest function

	clock_gettime(CLOCK_MONOTONIC, &end);   //end time of full processes

	double time = (end.tv_sec- start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1000000000.0);
	printf("Full time of process : %lf\n", time); //print full processes time
	return 0;
}


