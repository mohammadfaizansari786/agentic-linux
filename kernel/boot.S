.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    push %ebx
    push %eax
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start

.extern isr_exception_handler
.macro ISR_NOERR n
.global isr\n
.type isr\n, @function
isr\n:
    cli
    pushl $0
    pushl $\n
    pusha
    pushl %esp
    call isr_exception_handler
    addl $4, %esp
1:  hlt
    jmp 1b
.endm

.macro ISR_ERR n
.global isr\n
.type isr\n, @function
isr\n:
    cli
    pushl $\n
    pusha
    pushl %esp
    call isr_exception_handler
    addl $4, %esp
1:  hlt
    jmp 1b
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $1f
1:
    ret

.section .note.GNU-stack,"",@progbits
