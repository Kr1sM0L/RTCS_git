#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	int fd1, fd2, fd3;
	fd1=open("text.txt", O_CREAT|O_RDWR|O_TRUNC); 
	printf("fd1=%d\n", fd1); 
	write(fd1, "what's",6); 
	fd2=dup(fd1); 
	printf("fd2=%d\n", fd2); 
	write(fd2," up",3); 
	close(0); 
	fd3=dup(fd1); 
	printf("fd3=%d\n", fd3); 
	write(0," doc",4);
	dup2(0,2);
	write(2, "/?\n", 2);
}
