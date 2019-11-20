#define __LIBRARY__
#include <linux/sys.h>
#include <semaphore.h>
#include <unistd.h>

_syscall1(sem_t*, sem_open, const char*, name);
_syscall1(int, sem_wait, sem_t*, sem);
_syscall1(int, sem_post, sem_t*, sem);
_syscall1(int, sem_unlink, const char*, name);
