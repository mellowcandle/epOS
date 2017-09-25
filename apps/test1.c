
#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


//int main(int argc, char *argv[])
int main(int argc, int argv)
{
	int pid = getpid();
	int ret;
	printf("%d - Ramon\n", pid);
	ret = fork();
	if (ret == 0)
	{
		printf("Father, I'm here\r\n");
	}
	else
	{
		printf("Luke %d, I'm your father\r\n", ret);
	}

	while(1);

	return 13;
}
