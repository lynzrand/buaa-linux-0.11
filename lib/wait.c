/*
 *  linux/lib/wait.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <sys/wait.h>
#include <unistd.h>

_syscall3(pid_t, waitpid, pid_t, pid, int *, wait_stat, int, options)

	pid_t wait(int *wait_stat)
{
	return waitpid(-1, wait_stat, 0);
}
