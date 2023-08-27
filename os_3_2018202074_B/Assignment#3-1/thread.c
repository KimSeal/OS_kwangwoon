#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <time.h> //use clock_gettime

#define MAX_PROCESSES 64

FILE * f_read;	//to file read

void* thread_sum(void * input)	//function to sum
{
	int dep = *((int*)input);	//tree's depth save
	int left;			//take left node value
	int right;			//take right node value
	static int result;		//save result
	if(MAX_PROCESSES < dep*2){	//no over tree
		if(EOF==fscanf(f_read, "%d", &left)){
			left =0;
		}
		if(EOF==fscanf(f_read, "%d", &right)){
			right =0;
		}
		//take txt file's numbers	
	}
	else				//nodes are not leaves
	{
		int subdep = dep * 2;	//for child node's depth
		pthread_t tid;		//threadid to take child's result
		pthread_t tid2;
		int * sub_left;		//sub node's left right value
		int * sub_right;
		
		pthread_create(&tid, NULL, thread_sum, (void*)&subdep);
		//make thread like left child node
		pthread_join(tid, (void**)&sub_left);
		//take left child node's value
		left = (*sub_left);
		
		pthread_create(&tid2, NULL, thread_sum, (void*)&subdep);
		//make thread like right child node
		pthread_join(tid2, (void**)&sub_right);
		//take right child node's value
		right = (*sub_right);
	}
	result=left+right;
	//add two value
	pthread_exit((void*)&result);
	//return add value
}

int main()
{
	f_read = fopen("temp.txt", "r");
	//open file to read values
	struct timespec start, end;
	//check time
	pthread_t tid;	
	//variable to save thread id
	int *result;
	int dep = 1;	//first depth

	system("rm -rf tmp");
	system("sync");
	system("echo 3 | sudo tee /proc/sys/vm/drop_caches");

	clock_gettime(CLOCK_MONOTONIC, &start);//time to start

	pthread_create(&tid, NULL, thread_sum, (void*)&dep);//call sub-thread
	pthread_join(tid, (void **) &result);//block until sub-thread & return

	clock_gettime(CLOCK_MONOTONIC, &end);//clock end

	printf("value of fork : %d\n", *result);//print fork
	double time = (end.tv_sec- start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1000000000.0);
	printf("%lf\n", time); //print time
	fclose(f_read);		//close file
	return 0;
	
}

