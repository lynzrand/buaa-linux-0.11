#define __LIBRARY__
#include <fcntl.h>
#include <linux/semaphore.h>
#include <linux/shm.h>
#include <stdio.h>
#include <unistd.h>

#define PRODUCER_MAX 1000

_syscall2(sem_t*, sem_open, const char*, name, unsigned int, init_val);
_syscall1(int, sem_wait, sem_t*, sem);
_syscall1(int, sem_post, sem_t*, sem);
_syscall1(int, sem_unlink, const char*, name);
_syscall3(int, shmget, int, key, size_t, size, int, shm_flag);
_syscall1(int, shmat, int, id);
_syscall1(int, shmdt, int, key);

#define BUFFER_FILE "/usr/root/buf"
#define PRODUCER_MAX 1000
#define buf_size 10
#define char_buf_size buf_size*(sizeof(int) / sizeof(char))

void producer() {
  int buffer_id;
  sem_t* empty = sem_open("empty", 0);
  sem_t* sync = sem_open("sync", 0);
  sem_t* full = sem_open("full", 0);
  int i, j, k;
  int shmid;
  int* buf;

  shmid = shmget(789, sizeof(int) * buf_size, SHM_CREAT);
  buf = shmat(shmid);

  for (i = 0; i < buf_size; i++) {
    buf[i] = -1;
  }

  sem_post(sync);
  sem_post(empty);

  while (i <= PRODUCER_MAX) {
    int j;
    sem_wait(empty);
    sem_wait(sync);

    for (j = 0; j < buf_size; j++) {
      if (buf[j] < 0) {
        buf[j] = i;
        i++;
      }
    }

    sem_post(sync);
    sem_post(full);
  }
  sem_wait(empty);
  sem_wait(sync);

  for (j = 0; j < buf_size; j++) {
    buf[j] = -2;
  }

  sem_post(sync);
  sem_post(full);
}

void consumer() {
  sem_t* empty = sem_open("empty", 0);
  sem_t* sync = sem_open("sync", 0);
  sem_t* full = sem_open("full", 0);
  int shmid;
  int* buf;
  int i, j;
  shmid = shmget(789, sizeof(int) * 10, SHM_CREAT);
  buf = shmat(shmid);

  while (1) {
    int min = 0;
    int num;
    int has_data = 0;
    sem_wait(full);
    sem_wait(sync);

    for (j = 0; j < buf_size; j++) {
      if (buf[j] >= 0) {
        printf("%d: %d\n", getpid(), buf[j]);
        buf[j] = -1;
        has_data = 1;
        break;
      }
      if (buf[j] == -2) {
        has_data = 1;
        break;
      }
    }

    sem_post(sync);
    if (has_data)
      sem_post(full);
    else
      sem_post(empty);
    sleep(0);
  }
}

int main(int argc, char** argv) {
  pid_t pid_1;
  pid_t pid_2;
  sem_unlink("empty");
  sem_unlink("full");
  sem_unlink("sync");

  if ((pid_1 = fork())) {
    if ((pid_2 = fork())) {
      producer();
      waitpid(pid_2, NULL, 0);
    } else {
      consumer();
    }
    waitpid(pid_1, NULL, 0);
  } else {
    pid_2 = fork();
    consumer();
    if (pid_2)
      waitpid(pid_2, NULL, 0);
  }
  return 0;
}
