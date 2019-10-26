---
title: 系统调用
---

> ## 实验环境
>
> Windows 10, WSL2 Ubuntu 18.04  
> QEMU  
> VSCode

# 回答问题

> 从 Linux 0.11 现在的机制看，它的系统调用最多能传递几个参数？你能想出办法来扩大这个限制吗？

3 个。扩大限制的话一个是可以利于 x86 调用门等选择符的选项设置增加到 16 个，另一个也可以将参数存在用户栈顶。

> 用文字简要描述向 Linux 0.11 添加一个系统调用 foo() 的步骤。

在内核源代码中添加相应系统调用编号和实现；在用户库中添加调用编号和声明。

# 实验目标

- 了解 Linux 系统调用的原理
- 创建一个系统调用

# 实验过程

实验使用的 `whoami` 和 `iam` 函数使用 C 编写，存放在 `kernel/who.c` 中。

代码如下：

```c
#include <errno.h>

#include <asm/segment.h>
#include <linux/kernel.h>
#include <sys/times.h>
#include <sys/utsname.h>

int whoami_size = 0;
char whoami_name[24] = {0};

int sys_whoami(char *name, unsigned int size)
{
    int i;
    // printk("whoami gets called\n");
    if (size < whoami_size)
    {
        return -EINVAL;
        // return -2;
    }
    for (i = 0; i < size; i++)
    {
        put_fs_byte(whoami_name[i], name + i);
    }
    return 0;
}

int sys_iam(const char *name)
{
    int i;
    whoami_size = 0;
    // printk("iam gets called!\n");
    for (; whoami_size < 24; whoami_size++)
    {
        whoami_name[whoami_size] = get_fs_byte(name + whoami_size);
        if (whoami_name[whoami_size] == '\0')
        {
            whoami_size++;
            return 0;
        }
    }
    whoami_size = 0;
    return -EINVAL;
    // return -2;
}
```

![编写系统调用的实现](https://i.loli.net/2019/10/26/F8D6LEAekmsKhB3.png)

![将自定义调用添加到系统调用表中](https://i.loli.net/2019/10/26/1VrSToDinfJWBNa.png)

![用于测试的代码](https://i.loli.net/2019/10/26/z95ymDeClshk7co.png)

# 测试

> 检查了用于测试的脚本。发现脚本期望的是在输入字符串过长的情况下丢弃传入的字符串。但是这个行为在实验要求中没有定义，是 UB。因此没有运行测试脚本，而是自己写了另一个测试程序实验。

程序如下:

```c
#define __LIBRARY__

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

_syscall1(int, iam, const char *, name);
_syscall2(int, whoami, char *, name, unsigned int, size);

int main()
{
    int result;
    char name[14] = 0;
    printf("System call id: %d %d\n", __NR_whoami, __NR_iam);
    errno = 0;

    printf("Regular iam call:\n");
    result = iam("Rynco");
    printf("call result: %d %d %s", result, errno, strerror(errno));

    errno = 0;
    printf("Regular whoami call:\n");
    result = whoami(name, 14);
    printf("call result: %d %d %s\nbuf: %s\n", result, errno, strerror(errno), name);

    {
        int i;
        for (i = 0; i < 14; i++)
            name[i] = 0;
    }

    errno = 0;
    printf("whoami call with small buffer:\n");
    result = whoami(name, 1);
    printf("call result: %d %d %s\nbuf: %s\n", result, errno, strerror(errno), name);

    errno = 0;
    printf("iam call with long content:\n");
    result = iam("Ryncoooooooooooooooooooooooooooooooooooooooooooo");
    printf("call result: %d %d %s", result, errno, strerror(errno));
}
```

![测试程序输出](https://i.loli.net/2019/10/26/IYl1bV6tskS3gFB.png)

# 感想/收获

- 系统调用利用了 CPU 的中断特性，具体实现上与之前的 8086 中断向量表有些类似
- 学会了 qemu 的配置方法（就是这玩意从硬盘而不是软盘启动卡了我一整天）

---

本次实验的大多数素材可以在 <https://github.com/01010101lzy/buaa-linux-0.11/tree/chap-03> 找到。
