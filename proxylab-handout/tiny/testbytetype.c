#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
 //long b[11000];
void *b =(void *)malloc(11000);
	int fd=open("godzilla.gif",O_RDONLY,0);
 read(fd,b,11000);
 int fd2=open("god2.gif",O_TRUNC|O_WRONLY|O_CREAT,0);
 write(fd2,b,11000);
close(fd);
close(fd2);
free(b);
}
