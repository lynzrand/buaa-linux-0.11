#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#define SEMAPHORE_NAME_LIMIT 24

typedef struct {
  char name[SEMAPHORE_NAME_LIMIT];
  int value;
  struct task_struct* waiting;
} sem_t;

sem_t* sem_open(const char* name, unsigned int value);
int sem_wait(sem_t* sem);
int sem_post(sem_t* sem);
int sem_unlink(const char* name);

#endif
