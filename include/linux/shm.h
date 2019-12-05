#ifndef SHM_H
#define SHM_H

#include <unistd.h>
#define VEC_DEF_SIZE 8
#define SHM_NAME_LIMIT 24

#define SHM_CREAT 0x02
#define SHM_DEL 0x10

typedef struct {
  unsigned long* data;
  int size;
  int cap;
} ul_vec_t;

ul_vec_t init_ul_vec() {
  unsigned long* data = malloc(VEC_DEF_SIZE);
  ul_vec_t vec = {data, 0, VEC_DEF_SIZE};
  return vec;
}

void ul_vec_push(ul_vec_t* self, unsigned long val) {
  if (self->size >= self->cap) {
    // realloc
    int i = 0;
    unsigned long* data = malloc(self->size * 2);
    for (i = 0; i < self->size; i++) {
      data[i] = self->data[i];
    }
    free(self->data);
    self->data = data;
  }
  self->data[self->size] = val;
  (self->size)++;
}

void ul_vec_drop(ul_vec_t* self) {
  free(self->data);
  self->size = self->cap = 0;
}

typedef struct {
  int id;
  size_t size;
  // ul_vec_t phys_page;
  unsigned long phys_page;
  void* start;
  int rc;
} shm_entry;

int sys_shmget(int key, size_t size, int shm_flag);
void* sys_shmat(int id);
int sys_shmdt(int id);

#endif
