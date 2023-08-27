#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

int main()
{
	syscall(336, getpid()); 	//call system calls
	int fd = 0;			
	char buf[50];			
	fd = open("abc.txt", O_RDWR);	//open check
	for (int i = 1; i <= 5; ++i)	
	{		
		read(fd, buf, 5);	//read check
		lseek(fd, 0, SEEK_END);	//lseek check
		write(fd, buf, 5);	//write check
		lseek(fd, i*6, SEEK_SET);	//lseek check
	}
	lseek(fd, 0, SEEK_END);		//lseek
	write(fd, "HELLO", 3);		//write check
	close(fd);			//close check
	syscall(336,0);			//system call end
	return 0;

}

