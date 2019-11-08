KERNEL_STACK = (33*16+4)
ESP0=4

.text
.global switch_to
.global first_return_from_kernel

.align 2
switch_to:
        pushl %ebp      # 保存调用此函数的函数的栈帧基地址
        movl %esp, %ebp # 当前函数(switch_to)的栈帧基地址为栈顶地址
        pushl %ecx
        pushl %ebx
        pushl %eax      # 以上保存下面用到的几个寄存器
        movl 8(%ebp), %ebx      # 调用switch_to的第一个参数，即pnext——目标进程的PCB
        cmpl %ebx, current      # current为全局变量，指向当前进程的PCB
        je 1f           # 如果要切换到的进程就是目标进程，则不需要做任何操作
        
        # 切换PCB
        movl %ebx, %eax
        xchgl %eax, current     # eax指向当前进程，ebx指向下一个进程，current指向下一个进程
        
        # TSS中的内核栈基地址指针（esp）重写（处理器处理中断时会用到TSS中的内核栈指针来恢复内核栈位置，即设置内核栈的ebp）
        movl tss, %ecx  # tss为全局变量，指向当前进程的tss，以后所有进程都用这个tss，任务切换时不再像以前一样发生变化
        addl $4096, %ebx  # 加上4096是因为如实验指导书所说，内核栈栈底位于PCB所在内存的高地址空间
        movl %ebx, ESP0(%ecx)
        
        # 切换内核栈栈顶指针（切换当前的内核栈为目标内核栈）,即保存当前内核栈用到了哪个位置
        movl %esp, KERNEL_STACK(%eax)  # 保存当前进程内核栈栈顶指针到PCB中，注意，上面已经将eax指向了当前进程的PCB
        movl 8(%ebp), %ebx      # 注意，这里取出的是下一个进程的PCB地址
        movl KERNEL_STACK(%ebx), %esp  # 通过PCB地址可以获得之前保存的内核栈栈顶指针位置
        
        # 切换LDT
        movl 12(%ebp), %ecx
        lldt %cx
        
        movl $0x17, %ecx
        mov %cx, %fs  # 为啥？？目的是修改一下fs的值，会重新加载段寄存器的隐式部分
        cmpl %eax, last_task_used_math
        jne 1f
        clts
1:      popl %eax  # 以下恢复函数开始时保存的寄存器，注意，这里已经切换到了另一个进程的内核栈，所以这些参数进程中开始的时候需要保存下来
        popl %ebx
        popl %ecx
        popl %ebp  # 恢复栈帧基地址
        ret

first_return_from_kernel:
    popl %edx
    popl %edi
    popl %esi
    pop %gs
    pop %fs
    pop %es
    pop %ds
    iret

