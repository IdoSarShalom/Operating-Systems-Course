#include<stdio.h> 
#include <string.h>
#include<fcntl.h> 
#include<stdlib.h> 
#include<sys/types.h>
#include<sys/stat.h>
#define GODEL 5 

int main(int argc, char * argv[])
{
	/* file descriptor */
	int fd1;           
	int fd2;		   
	
	/* how many characters were actually read */
	int camar1;			
	int camar2;			
		
	char buf1[GODEL + 1];
	char buf2[GODEL + 1];

	memset(buf1, 0, GODEL + 1);
	memset(buf2, 0, GODEL + 1);
	
	fd1 = open(argv[1], O_RDONLY);
	fd2 = open(argv[2], O_RDONLY);

	camar1 = read(fd1, buf1, GODEL);
	camar2 = read(fd2, buf2, GODEL);

	int val = strcmp(buf1, buf2);

	if (val != 0)
		 exit(1);

	exit(2);
}