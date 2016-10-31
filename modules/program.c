
int printf(const char *format, ...);

int main(int argc, char *argv[])
{

	printf("Hello from user space\r\n");
here:
	__asm(".intel_syntax noprefix");

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
	return 0;
}
