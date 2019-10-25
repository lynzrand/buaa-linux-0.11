// #include "printk.c"
#include <errno.h>

#include <asm/segment.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <sys/times.h>
#include <sys/utsname.h>
int _name_size = 0;
char _name[24] = {0};

int sys_whoami(char *name, unsigned int size)
{
    int i;
    printk("whoami gets called!");
    if (size < _name_size)
    {
        errno = EINVAL;
        return -2;
    }
    for (i = 0; i < size; i++)
    {
        put_fs_byte(_name[i], name);
    }
    return 0;
}

int sys_iam(const char *name)
{
    int i;
    _name_size = 0;
    printk("whoami gets called!");
    for (; _name_size < 24; _name_size++)
    {
        _name[_name_size] = get_fs_byte(name + _name_size);
        if (_name[_name_size] == '\0')
        {
            _name_size++;
            return 0;
        }
    }
    for (i = 0; i < 24; i++)
    {
        _name[i] = 0;
    }
    _name_size = 0;
    errno = EINVAL;
    return -2;
}
