# 1 "user-utils.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "user-utils.S"
# 1 "include/asm.h" 1
# 2 "user-utils.S" 2

.globl syscall_sysenter; .type syscall_sysenter, @function; .align 0; syscall_sysenter:
 push %ecx
 push %edx
 push $SYSENTER_RETURN
 push %ebp
 mov %esp, %ebp
 sysenter
.globl SYSENTER_RETURN; .type SYSENTER_RETURN, @function; .align 0; SYSENTER_RETURN:
 pop %ebp
 pop %edx
 pop %edx
 pop %ecx
 ret


.globl write; .type write, @function; .align 0; write:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $4, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 movl 0x10(%ebp), %edx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret


nok:
 neg %eax
 mov %eax, errno
 mov $-1, %eax
 popl %ebp
 ret


.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp, %ebp
 movl $10, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl getpid; .type getpid, @function; .align 0; getpid:
 pushl %ebp
 movl %esp, %ebp
 movl $20, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl fork; .type fork, @function; .align 0; fork:
 pushl %ebp
 movl %esp, %ebp
 movl $2, %eax
 call syscall_sysenter
 test %eax, %eax
 js nok
 popl %ebp
 ret


.globl exit; .type exit, @function; .align 0; exit:
 pushl %ebp
 movl %esp, %ebp
 movl $1, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl yield; .type yield, @function; .align 0; yield:
 pushl %ebp
 movl %esp, %ebp
 movl $13, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl get_stats; .type get_stats, @function; .align 0; get_stats:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $35, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl pollKey; .type pollKey, @function; .align 0; pollKey:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $3, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl threadCreate; .type threadCreate, @function; .align 0; threadCreate:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $5, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl threadExit; .type threadExit, @function; .align 0; threadExit:
 pushl %ebp
 movl %esp, %ebp
 movl $6, %eax
 call syscall_sysenter
 popl %ebp
 ret

.globl memoryInc; .type memoryInc, @function; .align 0; memoryInc:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 pushl %edx;
 pushl %ecx;
 movl $21, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ecx;
 popl %edx;
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl spritePut; .type spritePut, @function; .align 0; spritePut:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $22, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 movl 0x10(%ebp), %edx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl gotoXY; .type gotoXY, @function; .align 0; gotoXY:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $23, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl SetColor; .type SetColor, @function; .align 0; SetColor:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $24, %eax
 movl 0x8(%ebp), %ebx;
 movl 0xC(%ebp), %ecx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl semCreate; .type semCreate, @function; .align 0; semCreate:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $25, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl semWait; .type semWait, @function; .align 0; semWait:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $26, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl semSignal; .type semSignal, @function; .align 0; semSignal:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $27, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret

.globl semDestroy; .type semDestroy, @function; .align 0; semDestroy:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx;
 movl $28, %eax
 movl 0x8(%ebp), %ebx;
 call syscall_sysenter
 popl %ebx
 test %eax, %eax
 js nok
 popl %ebp
 ret
