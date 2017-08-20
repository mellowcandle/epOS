
#include <syscall.h>

int printf(const char *format, ...);

static int __stdin;
static int __stdout;
static int __stderr;

void stdlib_init()
{
	__stdin = syscall_open("/dev/keyboard");
	__stdout = syscall_open("/dev/console1");
	__stderr = syscall_open("/dev/console2");
}


//int main(int argc, char *argv[])
int main()
{
	while (1)
	{
	}

	return 0;
}
