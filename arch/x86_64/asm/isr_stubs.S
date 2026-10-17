.intel_syntax noprefix

.section .text

.macro isr_no_err_stub num
isr_stub_\num:
    push 0
    push \num
    jmp isr_common_stub
.endm

.macro isr_err_stub num
isr_stub_\num:
    push \num
    jmp isr_common_stub
.endm

isr_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rbx, rsp
    mov rdi, rsp
    and rsp, -16
    call interrupt_dispatch
    mov rsp, rbx

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

	isr_no_err_stub 0
	isr_no_err_stub 1
	isr_no_err_stub 2
	isr_no_err_stub 3
	isr_no_err_stub 4
	isr_no_err_stub 5
	isr_no_err_stub 6
	isr_no_err_stub 7
	isr_err_stub    8
	isr_no_err_stub 9
	isr_err_stub    10
	isr_err_stub    11
	isr_err_stub    12
	isr_err_stub    13
	isr_err_stub    14
	isr_no_err_stub 15
	isr_no_err_stub 16
	isr_err_stub    17
	isr_no_err_stub 18
	isr_no_err_stub 19
	isr_no_err_stub 20
	isr_no_err_stub 21
	isr_no_err_stub 22
	isr_no_err_stub 23
	isr_no_err_stub 24
	isr_no_err_stub 25
	isr_no_err_stub 26
	isr_no_err_stub 27
	isr_no_err_stub 28
	isr_no_err_stub 29
	isr_err_stub    30
	isr_no_err_stub 31
	isr_no_err_stub 64

	.global isr_stub_table

	.section .data

isr_stub_table:
	.irp  i, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 64
	.quad isr_stub_\i
	.endr
