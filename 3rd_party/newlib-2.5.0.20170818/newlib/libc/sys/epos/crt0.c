#include <fcntl.h>
#include <stdlib.h>
#include <reent.h>
static struct _reent ___reent = {0};

extern void exit(int code);
extern int main (int argc, int argv);
extern char **environ;

char *__env[1] = { 0 };
void _start(int argc, int argv, char **env) {
	if ((int) env == 0) {
		environ = __env;
	}
	else
	{
		environ = env;
	}
    int ex = main(argc, argv);
    exit(ex);
}

struct _reent * __libc_getreent (void)
{
	return &___reent;
}
