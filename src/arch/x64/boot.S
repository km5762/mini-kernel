#include "arch/x64/paging.h"
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
.align 8
header_start:
	.long magic
	.long architecture
	.long header_end - header_start
	.long -(magic + architecture + ( header_end - header_start ))
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
.global gdt
gdt:
.quad 0x0000000000000000
.quad 0x00af9a000000ffff
.quad 0x00af92000000ffff
gdt_end:
.global gdtr
gdtr:
  .short gdt_end - gdt - 1
  .quad gdt_physical

.section .data
.equ flags, (PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE)
.balign 4096
.global pml4
pml4:
    .quad pdpt_low_physical + flags
    .zero ((PML4_INDEX(KERNEL_VIRTUAL_BASE) - 1) * PAGE_ENTRY_BYTES)
    .quad pdpt_high_physical + flags

.balign 4096
.global pdpt_low
pdpt_low:
    .quad pd_physical + flags
    .zero ((PAGE_TABLE_BYTES - 1) * PAGE_ENTRY_BYTES)

.balign 4096
.global pdpt_high
pdpt_high:
    .zero (PDPT_INDEX(KERNEL_VIRTUAL_BASE) * PAGE_ENTRY_BYTES)
    .quad pd_physical + flags
    .zero ((PAGE_TABLE_BYTES - 1 - PDPT_INDEX(KERNEL_VIRTUAL_BASE)) * PAGE_ENTRY_BYTES)

.balign 4096
.global pd
pd:
    .quad KERNEL_PHYSICAL_BASE + flags + PAGE_ENTRY_LARGE
    .zero ((PAGE_TABLE_BYTES - 1) * PAGE_ENTRY_BYTES)

.section .text
.code32
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
    .quad isr_stub_\i
.endr

.section .text.boot
_start:
    cli

    # enable pae
    mov eax, cr3
    mov eax, cr4
    or  eax, 0x20
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
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax

    mov  rsp, OFFSET FLAT:stack_top
    mov  rax, OFFSET FLAT:entry64_high
    jmp  rax

entry64_high:
    mov edi, eax
    mov esi, ebx
    mov rax, OFFSET FLAT:kernel_main
    call rax

1:
    hlt
    jmp 1b

.size _start, . - _start
