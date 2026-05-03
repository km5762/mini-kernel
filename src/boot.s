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

.section .text
.global  _start
.type    _start, @function

_start:
	mov  $stack_top, %esp
  pushl   %ebx
  pushl   %eax
	call kernel_main
	cli

1:
	hlt
	jmp 1b

.size _start, . - _start
