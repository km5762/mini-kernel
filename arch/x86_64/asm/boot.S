#include "asm/paging.h"
.intel_syntax noprefix

.set magic, 0xe85250d6
.set architecture, 0
.set framebuffer_tag_type, 5
.set framebuffer_tag_flags, 0
.set framebuffer_tag_width, 0
.set framebuffer_tag_height, 0
.set framebuffer_tag_depth, 0
.set end_tag_type, 0
.set end_tag_flags, 0

.section .rodata.multiboot
.align   8

header_start:
	.long  magic
	.long  architecture
	.long  header_end - header_start
	.long  -(magic + architecture + ( header_end - header_start ))
	.align 8

framebuffer_tag_start:
	.hword framebuffer_tag_type
	.hword framebuffer_tag_flags
	.long  framebuffer_tag_end - framebuffer_tag_start
	.long  framebuffer_tag_width
	.long  framebuffer_tag_height
	.long  framebuffer_tag_depth

framebuffer_tag_end:
	.align 8

end_tag_start:
	.hword end_tag_type
	.hword end_tag_flags
	.long  end_tag_end - end_tag_start

end_tag_end:
header_end:

	.section .bss
	.align   16

stack_bottom:
	.skip 16384

stack_top:

	.section .rodata
	.global  gdt

gdt:
	.quad 0x0000000000000000
	.quad 0x00af9a000000ffff
	.quad 0x00af92000000ffff

gdt_end:
	.global gdtr

gdtr:
	.short gdt_end - gdt - 1
	.quad  gdt_physical

	.global gdtr_high

gdtr_high:
	.short gdt_end - gdt - 1
	.quad  gdt

	.section .data.page
	.equ     flags, (PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE)
	.balign  4096
	.global  pml4

pml4:
	.quad pdpt_low_physical + flags
	.zero ((PML4_INDEX(KERNEL_VIRTUAL_BASE) - 1) * PAGE_ENTRY_BYTES)
	.quad pdpt_high_physical + flags

	.balign 4096
	.global pdpt_low

pdpt_low:
	.quad pd_low_physical + flags
	.zero ((PAGE_TABLE_BYTES - 1) * PAGE_ENTRY_BYTES)

	.balign 4096
	.global pdpt_high

pdpt_high:
	.zero (PDPT_INDEX(KERNEL_VIRTUAL_BASE) * PAGE_ENTRY_BYTES)
	.quad pd_high_physical + flags
	.zero ((PAGE_TABLE_BYTES - 1 - PDPT_INDEX(KERNEL_VIRTUAL_BASE)) * PAGE_ENTRY_BYTES)

	.balign 4096
	.global pd_low

pd_low:
	.set  page_index, 0
	.rept PAGE_TABLE_ENTRIES
	.quad (page_index * PAGE_LARGE_BYTES) + flags + PAGE_ENTRY_LARGE
	.set  page_index, page_index + 1
	.endr

	.balign 4096
	.global pd_high

pd_high:
	.set  page_index, 0
	.rept PAGE_TABLE_ENTRIES
	.quad (page_index * PAGE_LARGE_BYTES) + flags + PAGE_ENTRY_LARGE
	.set  page_index, page_index + 1
	.endr

	.section .text.boot
	.code32
	.global  _start
	.type    _start, @function

_start:
	cli

# enable pae
mov eax, cr4
or  eax, 0x620
mov cr4, eax

# load page table into cr3
mov eax, OFFSET FLAT:pml4_physical
mov cr3, eax

# enable long mode
mov ecx, 0xc0000080
rdmsr
or  eax, 0x100
wrmsr

# enable paging
mov eax, cr0
or  eax, 0x80000000
mov cr0, eax

# load the gdt
lgdt gdtr_physical
.att_syntax
ljmp $0x08, $trampoline64
.intel_syntax noprefix

trampoline64:
	.code64
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov rsp, OFFSET FLAT:stack_top
	mov rax, OFFSET FLAT:entry64_high
	jmp rax

entry64_high:
	mov  edi, eax
	mov  esi, ebx
	mov  rax, OFFSET FLAT:kernel_main
	call rax

1:
	hlt
	jmp 1b

.size _start, . - _start
