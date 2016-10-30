int main(int argc, char *argv[])
{

here:
	__asm(".intel_syntax noprefix");

	__asm volatile("mov eax, 0xdeadbeef");
	goto here;
	return 0;
}
