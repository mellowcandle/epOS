
#include <syscall.h>
#include <stdio.h>
#include <string.h>



//int main(int argc, char *argv[])
int main(int argc, int argv)
{
	char * ramon = "My name is Ramon";
	int i=strlen(ramon);
	memcmp("earl", ramon, 4);
	printf("Ramon\n");
	i = i+1;
	return 13;
}
