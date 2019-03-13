#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd;

	/* Create stdin, stdout and stderr */
	fd = open("/dev/console", O_RDWR);
	dup(0);
	dup(0);

	while (1);
}
