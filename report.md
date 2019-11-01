---
title: 进程运行轨迹的跟踪
---

> ## 实验环境
>
> Windows 10, WSL2 Ubuntu 18.04  
> QEMU  
> VSCode

# 实验步骤

## 添加 `fprintk` 和 `process.log` 初始化函数

![添加 `fprintk` 函数](https://i.loli.net/2019/11/01/nDk1cmMgxtNUBVJ.png)

![添加 `process.log` 初始化函数](https://i.loli.net/2019/11/01/EwizjdbY5yhUSTI.png)

_因为在 QEMU 环境下把硬盘初始化函数放在 0 号进程中会引起 Kernel Panic（见 <https://piazza.com/class/k00qtx91fn64t1?cid=12>），因此把 `fprintk` 规定为只在 1 号进程创建之后进行记录，维持硬盘初始化函数在 1 号进程中不变。_

## 在各个修改进程状态的位置添加追踪函数

![在 `include/linux/sched.h` 添加打印宏](https://i.loli.net/2019/11/01/StpumqVRvNlbU1B.png)

下面是在各个函数中插入打印宏的位置：

```c
/* kernel/sched.c */
void sleep_on(struct task_struct **p)
{
  struct task_struct *tmp;

    // --snip--

  *p = current;
  current->state = TASK_UNINTERRUPTIBLE;

  // current process gets blocked
  log_proc(current, 'W');

  schedule();
  if (tmp)
    tmp->state = 0;

  // tmp runs
  log_proc(tmp, 'R');
}

void interruptible_sleep_on(struct task_struct **p)
{
    // --snip--
    
  current->state = TASK_INTERRUPTIBLE;

  // current process gets blocked
  log_proc(current, 'W');

  schedule();
  if (*p && *p != current)
  {
    (**p).state = 0;

    // p runs
    log_proc(*p, 'R');

    goto repeat;
  }
  *p = NULL;
  if (tmp)
    tmp->state = 0;

  // tmp runs
  log_proc(tmp, 'R');
}

void wake_up(struct task_struct **p)
{
  if (p && *p)
  {
    (**p).state = 0;

    // p runs
    log_proc(*p, 'R');

    *p = NULL;
  }
}
```

# 回答问题

> 结合自己的体会，谈谈从程序设计者的角度看，单进程编程和多进程编程最大的区别是什么？

需要

> 你是如何修改时间片的？仅针对样本程序建立的进程，在修改时间片前后，log 文件的统计结果（不包括 Graphic）都是什么样？结合你的修改分析一下为什么会这样变化，或者为什么没变化？

