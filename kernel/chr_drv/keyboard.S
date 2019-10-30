/*
 *  linux/kernel/keyboard.S
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *	Thanks to Alfred Leung for US keyboard patches
 *		Wolfgang Thiel for German keyboard patches
 *		Marc Corsini for the French keyboard
 */

#include <linux/config.h>

.text
.globl keyboard_interrupt

/*
 * these are for the keyboard read functions
 */
size	= 1024		/* must be a power of two ! And MUST be the same
			   as in tty_io.c !!!! */
head = 4
tail = 8
proc_list = 12
buf = 16

mode:	.byte 0		/* caps, alt, ctrl and shift mode */
leds:	.byte 2		/* num-lock, caps, scroll-lock mode (nom-lock on) */
e0:	.byte 0

/*
 *  con_int is the real interrupt routine that reads the
 *  keyboard scan-code and converts it into the appropriate
 *  ascii character(s).
 */
keyboard_interrupt:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	xor %al,%al		/* %eax is scan code */
	inb $0x60,%al
	cmpb $0xe0,%al
	je set_e0
	cmpb $0xe1,%al
	je set_e1
	call key_table(,%eax,4)
	movb $0,e0
e0_e1:	inb $0x61,%al
	jmp 1f
1:	jmp 1f
1:	orb $0x80,%al
	jmp 1f
1:	jmp 1f
1:	outb %al,$0x61
	jmp 1f
1:	jmp 1f
1:	andb $0x7F,%al
	outb %al,$0x61
	movb $0x20,%al
	outb %al,$0x20
	pushl $0
	call do_tty_interrupt
	addl $4,%esp
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret
set_e0:	movb $1,e0
	jmp e0_e1
set_e1:	movb $2,e0
	jmp e0_e1

/*
 * This routine fills the buffer with max 8 bytes, taken from
 * %ebx:%eax. (%edx is high). The bytes are written in the
 * order %al,%ah,%eal,%eah,%bl,%bh ... until %eax is zero.
 */
put_queue:
	pushl %ecx
	pushl %edx
	movl table_list,%edx		# read-queue for console
	movl head(%edx),%ecx
1:	movb %al,buf(%edx,%ecx)
	incl %ecx
	andl $size-1,%ecx
	cmpl tail(%edx),%ecx		# buffer full - discard everything
	je 3f
	shrdl $8,%ebx,%eax
	je 2f
	shrl $8,%ebx
	jmp 1b
2:	movl %ecx,head(%edx)
	movl proc_list(%edx),%ecx
	testl %ecx,%ecx
	je 3f
	movl $0,(%ecx)
3:	popl %edx
	popl %ecx
	ret

ctrl:	movb $0x04,%al
	jmp 1f
alt:	movb $0x10,%al
1:	cmpb $0,e0
	je 2f
	addb %al,%al
2:	orb %al,mode
	ret
unctrl:	movb $0x04,%al
	jmp 1f
unalt:	movb $0x10,%al
1:	cmpb $0,e0
	je 2f
	addb %al,%al
2:	notb %al
	andb %al,mode
	ret

lshift:
	orb $0x01,mode
	ret
unlshift:
	andb $0xfe,mode
	ret
rshift:
	orb $0x02,mode
	ret
unrshift:
	andb $0xfd,mode
	ret

caps:	testb $0x80,mode
	jne 1f
	xorb $4,leds
	xorb $0x40,mode
	orb $0x80,mode
set_leds:
	call kb_wait
	movb $0xed,%al		/* set leds command */
	outb %al,$0x60
	call kb_wait
	movb leds,%al
	outb %al,$0x60
	ret
uncaps:	andb $0x7f,mode
	ret
scroll:
	xorb $1,leds
	jmp set_leds
num:	xorb $2,leds
	jmp set_leds

/*
 *  curosr-key/numeric keypad cursor keys are handled here.
 *  checking for numeric keypad etc.
 */
cursor:
	subb $0x47,%al
	jb 1f
	cmpb $12,%al
	ja 1f
	jne cur2		/* check for ctrl-alt-del */
	testb $0x0c,mode
	je cur2
	testb $0x30,mode
	jne reboot
cur2:	cmpb $0x01,e0		/* e0 forces cursor movement */
	je cur
	testb $0x02,leds	/* not num-lock forces cursor */
	je cur
	testb $0x03,mode	/* shift forces cursor */
	jne cur
	xorl %ebx,%ebx
	movb num_table(%eax),%al
	jmp put_queue
1:	ret

cur:	movb cur_table(%eax),%al
	cmpb $'9,%al
	ja ok_cur
	movb $'~,%ah
ok_cur:	shll $16,%eax
	movw $0x5b1b,%ax
	xorl %ebx,%ebx
	jmp put_queue

#if defined(KBD_FR)
num_table:
	.ascii "789 456 1230."
#else
num_table:
	.ascii "789 456 1230,"
#endif
cur_table:
	.ascii "HA5 DGC YB623"

/*
 * this routine handles function keys
 */
func:
	pushl %eax
	pushl %ecx
	pushl %edx
	call show_stat
	popl %edx
	popl %ecx
	popl %eax
	subb $0x3B,%al
	jb end_func
	cmpb $9,%al
	jbe ok_func
	subb $18,%al
	cmpb $10,%al
	jb end_func
	cmpb $11,%al
	ja end_func
ok_func:
	cmpl $4,%ecx		/* check that there is enough room */
	jl end_func
	movl func_table(,%eax,4),%eax
	xorl %ebx,%ebx
	jmp put_queue
end_func:
	ret

/*
 * function keys send F1:'esc [ [ A' F2:'esc [ [ B' etc.
 */
func_table:
	.long 0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b
	.long 0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b
	.long 0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b

#if	defined(KBD_FINNISH)
key_map:
	.byte 0,27
	.ascii "1234567890+'"
	.byte 127,9
	.ascii "qwertyuiop}"
	.byte 0,13,0
	.ascii "asdfghjkl|{"
	.byte 0,0
	.ascii "'zxcvbnm,.-"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '<
	.fill 10,1,0

shift_map:
	.byte 0,27
	.ascii "!\"#$%&/()=?`"
	.byte 127,9
	.ascii "QWERTYUIOP]^"
	.byte 13,0
	.ascii "ASDFGHJKL\\["
	.byte 0,0
	.ascii "*ZXCVBNM;:_"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '>
	.fill 10,1,0

alt_map:
	.byte 0,0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte 0,0,0,0,0		/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '|
	.fill 10,1,0

#elif defined(KBD_US)

key_map:
	.byte 0,27
	.ascii "1234567890-="
	.byte 127,9
	.ascii "qwertyuiop[]"
	.byte 13,0
	.ascii "asdfghjkl;'"
	.byte '`,0
	.ascii "\\zxcvbnm,./"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '<
	.fill 10,1,0


shift_map:
	.byte 0,27
	.ascii "!@#$%^&*()_+"
	.byte 127,9
	.ascii "QWERTYUIOP{}"
	.byte 13,0
	.ascii "ASDFGHJKL:\""
	.byte '~,0
	.ascii "|ZXCVBNM<>?"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '>
	.fill 10,1,0

alt_map:
	.byte 0,0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte 0,0,0,0,0		/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '|
	.fill 10,1,0

#elif defined(KBD_GR)

key_map:
	.byte 0,27
	.ascii "1234567890\\'"
	.byte 127,9
	.ascii "qwertzuiop@+"
	.byte 13,0
	.ascii "asdfghjkl[]^"
	.byte 0,'#
	.ascii "yxcvbnm,.-"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '<
	.fill 10,1,0


shift_map:
	.byte 0,27
	.ascii "!\"#$%&/()=?`"
	.byte 127,9
	.ascii "QWERTZUIOP\\*"
	.byte 13,0
	.ascii "ASDFGHJKL{}~"
	.byte 0,''
	.ascii "YXCVBNM;:_"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		/* 3A-49 */
	.byte '-,0,0,0,'+	/* 4A-4E */
	.byte 0,0,0,0,0,0,0	/* 4F-55 */
	.byte '>
	.fill 10,1,0

alt_map:
	.byte 0,0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte '@,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0