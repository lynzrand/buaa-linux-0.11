/*
 *  linux/kernel/fork.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also system_call.s), and some misc functions ('verify_area').
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/mm.c': 'copy_page_tables()'
 */
#include <errno.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/sched.h>

extern void write_verify(unsigned long address);
extern void first_return_from_kernel();

long last_pid = 0;

void verify_area(void *addr, int size)
{
	unsigned long start;

	start = (unsigned long)addr;
	size += start & 0xfff;
	start &= 0xfffff000;
	start += get_base(current->ldt[2]);
	while (size > 0)
	{
		size -= 4096;
		write_verify(start);
		start += 4096;
	}
}

int copy_mem(int nr, struct task_struct *p)
{
	unsigned long old_data_base, new_data_base, data_limit;
	unsigned long old_code_base, new_code_base, code_limit;

	code_limit = get_limit(0x0f);
	data_limit = get_limit(0x17);
	old_code_base = get_base(current->ldt[1]);
	old_data_base = get_base(current->ldt[2]);
	if (old_data_base != old_code_base)
		panic("We don't support separate I&D");
	if (data_limit < code_limit)
		panic("Bad data_limit");
	new_data_base = new_code_base = nr * 0x4000000;
	p->start_code = new_code_base;
	set_base(p->ldt[1], new_code_base);
	set_base(p->ldt[2], new_data_base);
	if (copy_page_tables(old_data_base, new_data_base, data_limit))
	{
		printk("free_page_tables: from copy_mem\n");
		free_page_tables(new_data_base, data_limit);
		return -ENOMEM;
	}
	return 0;
}

/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It
 * also copies the data segment in it's entirety.
 */
int copy_process(int nr, long ebp, long edi, long esi, long gs, long none,
				 long ebx, long ecx, long edx,
				 long fs, long es, long ds,
				 long eip, long cs, long eflags, long esp, long ss)
{
	struct task_struct *p;
	int i;
	struct file *f;
	long *krnstack;

	p = (struct task_struct *)get_free_page();
	if (!p)
		return -EAGAIN;
	/* 输出新建进程的信息到日志文件中 */
	/* 此时为新建态('N')*/

	/* 子进程内核栈位置 */
	krnstack = (long *)(PAGE_SIZE + (long)p);
	printk("%p %ld\n", p, krnstack);
	/* 设置子进程内核栈 */
	*(--krnstack) = ss & 0xffff;
	*(--krnstack) = esp;
	*(--krnstack) = eflags;
	*(--krnstack) = cs & 0xffff;
	*(--krnstack) = eip;
	/* first_return_from_kernel中会弹出这些值 */
	*(--krnstack) = ds & 0xffff;
	*(--krnstack) = es & 0xffff;
	*(--krnstack) = fs & 0xffff;
	*(--krnstack) = gs & 0xffff;
	*(--krnstack) = esi;
	*(--krnstack) = edi;
	*(--krnstack) = edx;
	/* 当一个新建立的进程被调度执行时在switch_to结束后要执行first_return_from_kernle */
	*(--krnstack) = (long)first_return_from_kernel;
	/* switch_to函数中会使用内核栈中的这些值，所以这里需要在最开始的时候先保存下来 */
	*(--krnstack) = ebp;
	*(--krnstack) = ecx;
	*(--krnstack) = ebx;
	*(--krnstack) = 0; // 实际上是eax，子进程fork返回值为0，所以这里设置为0

	task[nr] = p;
	*p = *current; /* NOTE! this doesn't copy the supervisor stack */

	p->kernel_stack = krnstack; /* 在PCB中设置好内核栈栈顶 */

	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
	p->father = current->pid;
	p->counter = p->priority;
	p->signal = 0;
	p->alarm = 0;
	p->leader = 0; /* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;

	/* 修改TSS的内容全部注释掉*/
	/*
	p->tss.back_link = 0;
	p->tss.esp0 = PAGE_SIZE + (long) p;
	p->tss.ss0 = 0x10;
	p->tss.eip = eip;  // 子进程在被调度时直接跳到父进程中同样的位置开始执行
	p->tss.eflags = eflags;
	p->tss.eax = 0;  // 子进程返回0
	p->tss.ecx = ecx;
	p->tss.edx = edx;
	p->tss.ebx = ebx;
	p->tss.esp = esp;
	p->tss.ebp = ebp;
	p->tss.esi = esi;
	p->tss.edi = edi;
	p->tss.es = es & 0xffff;
	p->tss.cs = cs & 0xffff;
	p->tss.ss = ss & 0xffff;
	p->tss.ds = ds & 0xffff;
	p->tss.fs = fs & 0xffff;
	p->tss.gs = gs & 0xffff;
	p->tss.ldt = _LDT(nr);
	p->tss.trace_bitmap = 0x80000000;
	if (last_task_used_math == current)
		__asm__("clts ; fnsave %0"::"m" (p->tss.i387));
	*/
	if (copy_mem(nr, p))
	{
		task[nr] = NULL;
		free_page((long)p);
		return -EAGAIN;
	}
	for (i = 0; i < NR_OPEN; i++)
		if ((f = p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;
	set_tss_desc(gdt + (nr << 1) + FIRST_TSS_ENTRY, &(p->tss));
	set_ldt_desc(gdt + (nr << 1) + FIRST_LDT_ENTRY, &(p->ldt));
	p->state = TASK_RUNNING; /* do this last, just in case */
	return last_pid;
}

int find_empty_process(void)
{
	int i;

repeat:
	if ((++last_pid) < 0)
		last_pid = 1;
	for (i = 0; i < NR_TASKS; i++)
		if (task[i] && task[i]->pid == last_pid)
			goto repeat;
	for (i = 1; i < NR_TASKS; i++)
		if (!task[i])
			return i;
	return -EAGAIN;
}
