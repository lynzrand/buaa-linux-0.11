#define LIBRARY
#include "unistd.h"

/* iam()在用户空间的接口函数 */
_syscall1(int, iam, const char *, name);

/* whoami()在用户空间的接口函数 */
_syscall2(int, whoami, char *, name, unsigned int, size);

int main()
{
    iam("Rynco");
    char name[14] = {0};
    whoami(name, 14);
    printf("%s", name);
    return 0;
}
