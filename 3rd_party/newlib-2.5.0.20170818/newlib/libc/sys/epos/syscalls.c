/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#include <syscall.h>
char **environ; /* pointer to array of char * strings that define the current environment variables */

DEFN_SYSCALL0(_exit, 0)
DEFN_SYSCALL1(close, 1,  int)
DEFN_SYSCALL3(execve, 2, char *, char **, char **)
DEFN_SYSCALL0(fork, 3)
DEFN_SYSCALL2(fstat, 4, int, struct stat *)
DEFN_SYSCALL0(getpid, 5)
DEFN_SYSCALL0(iastty, 6)
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
DEFN_SYSCALL2(gettimeofday, 18, struct timeval *, void *);

/*
void _exit();
int close(int file);
int execve(char *name, char **argv, char **env);
int fork();
int fstat(int file, struct stat *st);
int getpid();
int isatty(int file);
int kill(int pid, int sig);
int link(char *old, char *new);
int lseek(int file, int ptr, int dir);
int open(const char *name, int flags, ...);
int read(int file, char *ptr, int len);
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
int write(int file, char *ptr, int len);
int gettimeofday(struct timeval *tv, void *tz);

*/
