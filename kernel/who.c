// #include "printk.c"
#include <errno.h>

#include <asm/segment.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
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
