
#include <syscall.h>

int printf(const char *format, ...);

int stdin;
int stdout;
int stderr;

//int main(int argc, char *argv[])
int main()
{
	int ret;
	stdin = syscall_open("/dev/console");
	ret = syscall_close(stdin);
//	stdout = syscall_open("/dev/console");
//	stderr = syscall_open("/dev/console");


//	unsigned int i=0;
	while (1) ;
	{

		//		__asm volatile("hlt");
	//__asm(".intel_syntax noprefix");
	__asm("int $0x80");

//		__asm volatile("mov eax, 0xdeadbeef");
//		printf("1 - %d\r\n",i++);
	}

#if 0
here:

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
#endif
	return 0;
}
