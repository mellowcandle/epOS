/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#include <syscall.h>

DEFN_SYSCALL0(exit, 0)
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

