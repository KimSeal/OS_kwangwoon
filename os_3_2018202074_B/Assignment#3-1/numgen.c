#include <stdio.h>

#define MAX_PROCESSES 64

int main()
{
	FILE *f_write = fopen("temp.txt", "w"); //open file to write
	for(int i=0;i<MAX_PROCESSES*2;i++)		//loop to print
	{
		fprintf(f_write, "%d\n", i+1);	//write in file
	}
	fclose(f_write);			//close file
	return 0;
}
