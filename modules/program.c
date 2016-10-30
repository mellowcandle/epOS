
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

	char buffer[1024];
	
	memset(buffer,0,1024);
	
	//	printf("Hello from user space");
here:
	__asm(".intel_syntax noprefix");

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
	return 0;
}
