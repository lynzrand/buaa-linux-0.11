#define __LIBRARY__
#include <fcntl.h>
#include <linux/sys.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define PRODUCER_MAX 1000

_syscall2(sem_t*, sem_open, const char*, name, int, init_val);
_syscall1(int, sem_wait, sem_t*, sem);
_syscall1(int, sem_post, sem_t*, sem);
_syscall1(int, sem_unlink, const char*, name);

#define BUFFER_FILE "/usr/root/buf"

int buffer_id;

void producer() {
  sem_t* empty = sem_open("empty", 1);
  sem_t* sync = sem_open("sync_sem", 1);
  sem_t* full = sem_open("full", 1);
  int i;

  for (i = 0; i < PRODUCER_MAX; i++) {
    sem_wait(empty);
    sem_wait(sync);

    sem_post(sync);
    sem_post(full);
  }
}

void consumer() {
  sem_t* empty = sem_open("empty", 1);
  sem_t* sync = sem_open("sync_sem", 1);
  sem_t* full = sem_open("full", 1);

  while (1) {
    sem_wait(full);
    sem_wait(sync);

    sem_post(sync);
    sem_post(empty);
  }
}

int main(int argc, char** argv) {
  pid_t pid_1;
  pid_t pid_2;

  buffer_id = open(BUFFER_FILE, O_RDWR | O_CREAT);

  if (pid_1 = fork()) {
    if (pid_2 = fork()) {
      producer();
    } else {
      consumer();
    }
  } else {
    pid_2 = fork();
    consumer();
  }
  return 0;
}
