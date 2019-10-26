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
