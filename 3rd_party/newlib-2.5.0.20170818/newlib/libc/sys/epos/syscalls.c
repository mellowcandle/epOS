/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <syscall.h>
char **environ; /* pointer to array of char * strings that define the current environment variables */

DEFN_SYSCALL1(exit, 0, int)
DEFN_SYSCALL1(close, 1,  int)
DEFN_SYSCALL3(execve, 2, char *, char **, char **)
DEFN_SYSCALL0(fork, 3)
DEFN_SYSCALL2(fstat, 4, int, struct stat *)
DEFN_SYSCALL0(getpid, 5)
DEFN_SYSCALL0(isatty, 6)
DEFN_SYSCALL2(kill, 7, int, int)
DEFN_SYSCALL2(link, 8, char *, char *)
DEFN_SYSCALL3(lseek, 9, int, int, int)
DEFN_SYSCALL3(open, 10, char *, int, int)
DEFN_SYSCALL3(read, 11, int, char *, int)
DEFN_SYSCALL1(sbrk, 12, int)
DEFN_SYSCALL2(stat, 13, char *, struct stat *)
DEFN_SYSCALL1(times, 14, struct tms *)
DEFN_SYSCALL1(unlink, 15, char *)
DEFN_SYSCALL1(wait, 16, int *)
DEFN_SYSCALL3(write, 17, int, char *, int)
DEFN_SYSCALL2(gettimeofday, 18, struct timeval *, void *)
DEFN_SYSCALL1(dup, 19, int);
DEFN_SYSCALL2(dup2, 20, int, int);

void _exit(int ret)
{
	syscall_exit(ret);
}

int close(int file)
{
	return syscall_close(file);
}

int execve(char *name, char **argv, char **env)
{
	return syscall_execve(name, argv, env);
}

int fork()
{
	return syscall_fork();
}

int fstat(int file, struct stat *st)
{
	return syscall_fstat(file, st);
}

int getpid()
{
	return syscall_getpid();
}

int isatty(int file)
{
	return syscall_isatty(file);
}

int kill(int pid, int sig)
{
	return syscall_kill(pid, sig);
}

int link(char *old, char *new)
{
	return syscall_link(old, new);
}

int lseek(int file, int ptr, int dir)
{
	return syscall_lseek(file, ptr, dir);
}

int open(const char *name, int flags, ...)
{
	va_list argp;
	int mode;
	int result;
	va_start(argp, flags);
	if (flags & O_CREAT) mode = va_arg(argp, int);
	va_end(argp);

	result = syscall_open(name, flags, mode);
	if (result == -1) {
		if (flags & O_CREAT) {
			errno = EACCES;
		} else {
			errno = ENOENT;
		}
	} else if (result < 0) {
		errno = -result;
	}
}

int dup(int fd)
{
	return syscall_dup(fd);
}

int dup2(int fd, int fd2)
{
	return syscall_dup2(fd, fd2);
}

int read(int file, char *ptr, int len)
{
	return syscall_read(file, ptr, len);
}

caddr_t sbrk(int incr)
{
	return (caddr_t) syscall_sbrk(incr);
}

int stat(const char *file, struct stat *st)
{
	return syscall_stat(file, st);
}

clock_t times(struct tms *buf)
{
	//TODO: Implement
	return -1;
}

int unlink(char *name)
{
	return syscall_unlink(name);
}

int wait(int *status)
{
	return syscall_wait(status);
}

int write(int file, char *ptr, int len)
{
	return syscall_write(file, ptr, len);
}

int gettimeofday(struct timeval *tv, void *tz)
{
	return syscall_gettimeofday(tv, tz);
}

