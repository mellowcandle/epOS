#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHELL_CMD "/bin/sh"

static char * argv_rc[] = { SHELL_CMD, NULL };
static char * envp_rc[] = { "HOME=/home/", NULL ,NULL };

int main(int argc, char *argv[])
{
	int fd;
	int pid;
	int i;

	/* Create stdin, stdout and stderr */
	fd = open("/dev/console", O_RDWR);
	dup(0);
	dup(0);

	printf("Initializing init\n");

	while (1) {
		pid = fork();
		if (!pid) {
			printf("Executing shell...\n");
			execve(SHELL_CMD, argv_rc, envp_rc);
			_exit(2);
		}
	while (pid != wait(&i))
		;

	printf("shell returned...\n");

	}
}
