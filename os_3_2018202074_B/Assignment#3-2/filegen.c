#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_PROCESSES 10000

int main(){
	char name[100];			//to save take file name
	mkdir("./temp", 0777);		//make folder "temp"
	for(int i=0; i < MAX_PROCESSES; i++){	//make MAXPROCESSES number folder
		sprintf(name, "./temp/%d", i);	//to make file name
		FILE * f_write = fopen(name, "w");	//make file
		fprintf(f_write, "%d\n", 1+rand()%9);	//and put random number
		fclose(f_write);			//close file
	}
	return 0;
}
