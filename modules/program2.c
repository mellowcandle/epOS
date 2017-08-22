
#include <syscall.h>

int printf(const char *format, ...);

int __stdin;
int __stdout;
int __stderr;

void stdlib_init()
{
	__stdin = syscall_open("/dev/keyboard");
	__stdout = syscall_open("/dev/console");
	__stderr = syscall_open("/dev/console");
}


//int main(int argc, char *argv[])
int main()
{
	while (1)
	{
		printf("(2) Hello world from user space\r\n");
	}

	return 0;
}
