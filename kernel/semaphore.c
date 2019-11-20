#include <asm/segment.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <string.h>
#include <unistd.h>

#define NR_SEMAPHORE 30

sem_t semaphores[NR_SEMAPHORE] = {0};

sem_t* scan_semaphores_for_name(const char* name) {
  int i;
  for (i = 0; i < NR_SEMAPHORE; i++) {
    if (semaphores[i].name[0] != 0) {
      int result = strcmp(semaphores[i].name, name);
      if (result == 0)
        return semaphores + i;
    }
  }
  return NULL;
}

sem_t* find_first_vacant_semaphore() {
  int i = 0;
  for (i = 0; i < NR_SEMAPHORE; i++) {
    if (semaphores[i].name[0] == 0)
      return semaphores + i;
  }
  return NULL;
}

sem_t* sem_open(const char* name, unsigned int value) {
  sem_t* target;

  if (get_fs_byte(name) == 0) {
    // errno = EINVAL;
    return (sem_t*)EINVAL;
  }

  if (target = scan_semaphores_for_name(name)) {
    // ignore value
    return target;
  } else if (target = find_first_vacant_semaphore()) {
    int i = 0;
    while (i < SEMAPHORE_NAME_LIMIT) {
      char c;
      c = get_fs_byte(name + i);
      *(target->name + i) = c;
      if (c == 0)
        break;

      target->value = value;
    }
    return target;
  } else {
    return (sem_t*)ENOENT;
  }
}

int sem_wait(sem_t* sem) {
  (sem->value)--;
  while (sem->value < 0) {
    sem->waiting = current;
    sleep_on(&sem->waiting);
  }
}

int sem_post(sem_t* sem) {
  (sem->value)++;
  if (sem->value <= 0)
    wake_up(&sem->waiting);
}

int sem_unlink(const char* name) {
  sem_t* target;

  if (name[0] == 0) {
    return -EINVAL;
  }

  target = scan_semaphores_for_name(name);
}
