#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <time.h> //use clock_gettime

#define MAX_PROCESSES 64

int main()
{
	FILE* f_read = fopen("temp.txt", "r");
	//open txt file to read
	struct timespec start, end;
	//variable to save time
	pid_t pid;
	
	int result =1;	//variable to save result
	int dep = 1;	//variable to save node's depth
	
	system("rm -rf tmp");
	system("sync");
	system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
	//initalize
	
	clock_gettime(CLOCK_MONOTONIC, &start);//time to start
	pid =fork();				//make child process
	if(pid==0){				//in child process
		while(1){			//for loop
			int left, right;
			if(MAX_PROCESSES < dep*2){	//when node in tree
				if(EOF==fscanf(f_read, "%d", &left)){
				left =0;
				}
				if(EOF==fscanf(f_read, "%d", &right)){
					right =0;
				}
				//take txt's values
				result = left+right;
				exit(result);
				//add and take result
			}
			if(!fork()){ // for left node
				dep*=2;
				continue;
			}

			wait(&left);	//take left result

			if(!fork()){ //for right node 
				dep*=2;
				continue;
			}
			wait(&right);	//take right result
			result = (left>>8)+(right>>8);//return add result
			exit(result);
		}
	}
	waitpid(pid, &result, 0);	//wait final result
	clock_gettime(CLOCK_MONOTONIC, &end);// time end
	
	if(result < 1<<8){		//
		printf("value of fork : %d\n", result);//print final fork
	}
	else{
		printf("value of fork : %d\n", result>>8);
	}
	double time = (end.tv_sec- start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1000000000.0);
	printf("%lf\n", time); //print time
	fclose(f_read);				  //file close
	return 0;
}

	
