#include <asm/segment.h>
#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <string.h>
#include <unistd.h>

#define NR_SEMAPHORE 30
// #define SEM_DBG 1

sem_t semaphores[NR_SEMAPHORE] = {{0}};

sem_t* scan_semaphores_for_name(const char* name) {
  int i;
  for (i = 0; i < NR_SEMAPHORE; i++) {
    if (semaphores[i].name[0] != 0) {
      int result, j = 0;
      while (j < SEMAPHORE_NAME_LIMIT) {
        if (semaphores[i].name[j] == name[j]) {
          if (name[j] == 0) {
            result = 0;
            break;
          }
          j++;
          continue;
        } else {
          result = 1;
          break;
        }
      }
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

sem_t* sys_sem_open(const char* name, unsigned int value) {
  sem_t* target;

  char name_[SEMAPHORE_NAME_LIMIT + 1] = {0};
  int i = 0;
  cli();
  name_[0] = get_fs_byte(name);
  while (name_[i] != 0 && i <= SEMAPHORE_NAME_LIMIT) {
    i++;
    name_[i] = get_fs_byte(name + i);
  }
  // printk("kernel: Preparing for semaphore %s\n", name_);

  if (i == SEMAPHORE_NAME_LIMIT + 1) {
#if SEM_DBG
    printk("name too long. bad.\n", name_);
    sti();
#endif
    return -EINVAL;
  }

  if (name_[0] == 0) {
#if SEM_DBG
    printk("Name invalid. Bad.\n", name_);
#endif
    sti();
    return -EINVAL;
  }

  if (target = scan_semaphores_for_name(name_)) {
    // ignore value
#if SEM_DBG
    printk("kernel: Found semaphore %s at %p\n", name_, target);
#endif
    sti();
    return target;
  } else if (target = find_first_vacant_semaphore()) {
    int i = 0;
    while (i < SEMAPHORE_NAME_LIMIT) {
      target->name[i] = name_[i];
      if (name_[i] == 0)
        break;
      i++;
    }
    target->value = value;
    // #if SEM_DBG
    printk("kernel: Created semaphore %s = %d at %p\n", name_, value, target);
    // #endif
    // printk("kernel: Created semaphore %s\n", name_);
    sti();
    return target;
  } else {
    sti();
#if SEM_DBG
    printk("kernel: Unable to create semaphore %s\n", name_);
#endif
    return (sem_t*)-ENOENT;
  }
}

int sys_sem_wait(sem_t* sem) {
#if SEM_DBG
  printk("kernel: Waiting sem %s, val: %d, waiting: %p\n", sem->name,
         sem->value, sem->waiting);
#endif
  cli();
  (sem->value)--;
  while (sem->value < 0) {
    sti();
    sleep_on(&sem->waiting);
  }
  sti();
  return sem->value;
}

int sys_sem_post(sem_t* sem) {
  cli();
  (sem->value)++;
  if (sem->value <= 0) {
    sti();
    wake_up(&sem->waiting);
  }
#if SEM_DBG
  printk("kernel: Posting sem %s, val: %d, waiting: %p\n", sem->name,
         sem->value, sem->waiting);
#endif
  sti();
  return sem->value;
}

int sys_sem_unlink(const char* name) {
  sem_t* target;
  char name_[SEMAPHORE_NAME_LIMIT + 1] = {0};
  int i = 0;
  cli();
  name_[0] = get_fs_byte(name);
  while (name_[i] != 0 && i <= SEMAPHORE_NAME_LIMIT) {
    i++;
    name_[i] = get_fs_byte(name + i);
  }

  if (i == SEMAPHORE_NAME_LIMIT + 1) {
    sti();
    return -EINVAL;
  }

  if (name_[0] == 0) {
    sti();
    return -EINVAL;
  }

  target = scan_semaphores_for_name(name_);
  sti();
#if SEM_DBG
  printk("kernel: Unlinking sem %s\n", name_);
#endif
  if (target == NULL) {
    return -ENOENT;
  } else {
    // mark as invalid
    target->name[0] = 0;
    return target->value;
  }
}
