#include <asm/segment.h>
#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/shm.h>
#define shm_cnt 50

shm_entry shm_repo[shm_cnt] = {{0}};
extern int sys_brk(unsigned long end_data_seg);

inline int is_shm_avail(shm_entry* shm) {
  return shm->rc != 0;
}

int get_spare_shm() {
  int i;
  for (i = 0; i < shm_cnt; i++) {
    if (shm_repo[i].rc == 0)
      return i;
  }
  return -1;
}

int get_shm(int key) {
  int i;
  for (i = 0; i < shm_cnt; i++) {
    if (shm_repo[i].id == key && shm_repo[i].rc != 0)
      return i;
  }
  return -1;
}

unsigned long create_shm_space(shm_entry* shm) {
  unsigned long page_address;
  unsigned long virt_address;
  unsigned long data_base;
  // int page_cnt = (shm->size / PAGE_SIZE) + 1;
  // sys_brk(current->brk + page_cnt * PAGE_SIZE);
  // data_base = current->brk + current->start_code;
  // while (page_cnt--) {
  //   page_address = shm->phys_page.data[page_cnt];
  //   data_base -= PAGE_SIZE;
  //   put_page(page_address, data_base);
  // }
  put_page(shm->phys_page, current->brk + current->start_code);

  return data_base;
}

void shm_new(shm_entry* self, int id, size_t size) {
  int page_cnt = (size / PAGE_SIZE) + 1;
  unsigned long page_address;
  self->id = id;
  self->phys_page = get_free_page();
  self->rc = 0;
  self->size = size;
  // self->start = create_shm_space(self);
  // while (page_cnt--) {
  //   page_address = get_free_page();
  //   ul_vec_push(&(self->phys_page), page_address);
  // }
}

int sys_shmget(int key, size_t size, int shm_flag) {
  int i;

  if ((i = get_shm(key)) != -1) {
    return i;
  } else if ((shm_flag & SHM_CREAT) && (i = get_spare_shm()) != -1) {
    shm_new(&shm_repo[i], key, size);
    return i;
  } else {
    return EINVAL;
  }
}

void* sys_shmat(int id) {
  shm_repo[id].rc++;
  return create_shm_space(&shm_repo[id]);
}

int sys_shmdt(int id) {
  shm_repo[id].rc--;
  // if (shm_repo[id].rc <= 0) {
  // ul_vec_drop(&shm_repo[id].rc);
  // }
}
