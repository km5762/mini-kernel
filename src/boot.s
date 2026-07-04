.set MAGIC, 0xE85250D6
.set ARCHITECTURE, 0
.set FRAMEBUFFER_TAG_TYPE, 5
.set FRAMEBUFFER_TAG_FLAGS, 0
.set FRAMEBUFFER_TAG_WIDTH, 0
.set FRAMEBUFFER_TAG_HEIGHT, 0
.set FRAMEBUFFER_TAG_DEPTH, 0
.set END_TAG_TYPE, 0
.set END_TAG_FLAGS, 0

.section .multiboot
.align 8
header_start:
	.long MAGIC
	.long ARCHITECTURE
	.long header_end - header_start
	.long -(MAGIC + ARCHITECTURE + ( header_end - header_start ))
.align 8
framebuffer_tag_start:
	.hword FRAMEBUFFER_TAG_TYPE
	.hword FRAMEBUFFER_TAG_FLAGS
	.long  framebuffer_tag_end - framebuffer_tag_start
	.long  FRAMEBUFFER_TAG_WIDTH
	.long  FRAMEBUFFER_TAG_HEIGHT
	.long  FRAMEBUFFER_TAG_DEPTH
framebuffer_tag_end:
.align 8
end_tag_start:
	.hword END_TAG_TYPE
	.hword END_TAG_FLAGS
	.long  end_tag_end - end_tag_start
end_tag_end:
header_end:

.section .bss
.align   16
stack_bottom:
	.skip 16384
stack_top:

.section .rodata
gdt:
  .quad 0x0000000000000000
  .quad 0x00CF9A000000FFFF
  .quad 0x00CF92000000FFFF
gdt_end:
gdtr:
  .word gdt_end - gdt - 1
  .long gdt

.section .text
.global  _start
.type    _start, @function

.macro isr_err_stub num
isr_stub_\num:
    call exception_handler
    iret
.endm

.macro isr_no_err_stub num
isr_stub_\num:
    call exception_handler
    iret
.endm

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

.global isr_stub_table
isr_stub_table:
.irp i, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    .long isr_stub_\i
.endr

_start:
  cli
	mov  $stack_top, %esp

  lgdt gdtr
  ljmp $0x08, $flush

flush:
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss

  pushl   %ebx
  pushl   %eax
	call kernel_main

1:
	hlt
	jmp 1b

.size _start, . - _start
