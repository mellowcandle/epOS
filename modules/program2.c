
int printf(const char *format, ...);

//int main(int argc, char *argv[])
int main()
{

	while (1)
	{
		printf("Hello from user space task 2\r\n");
	}

#if 0
here:
	__asm(".intel_syntax noprefix");

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
#endif
	return 0;
}
