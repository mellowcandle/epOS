
int printf(const char *format, ...);

//int main(int argc, char *argv[])
int main()
{
	unsigned int i=0;
	while (1)
	{
		printf("2 - %d\r\n",i++);
	}

#if 0
here:
	__asm(".intel_syntax noprefix");

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
#endif
	return 0;
}
