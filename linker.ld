#include "arch/x64/paging.h"

ENTRY(_start)

SECTIONS
{
  . = KERNEL_PHYSICAL_BASE;

  .boot :
  {
    KEEP(*(.rodata.multiboot))
    KEEP(*(.text.boot))
  } 

  . = KERNEL_VIRTUAL_BASE;

  HIDDEN(kernel_load_offset =
             KERNEL_PHYSICAL_BASE + ALIGN(SIZEOF(.boot), 4K) - KERNEL_VIRTUAL_BASE);

  .text : AT(ADDR(.text) + kernel_load_offset)
  {
    *(.text)
  } 

  . = ALIGN(4K);

  .rodata : AT(ADDR(.rodata) + kernel_load_offset)
  {
    *(.rodata)
  } 

  . = ALIGN(4K);

  .data : AT(ADDR(.data) + kernel_load_offset)
  {
    *(.data)
  } 

  . = ALIGN(4K);

  .bss : AT(ADDR(.bss) + kernel_load_offset)
  {
    *(COMMON)
    *(.bss)
  } 

}

gdt_physical       = gdt       + kernel_load_offset;
gdtr_physical       = gdtr       + kernel_load_offset;
pml4_physical      = pml4      + kernel_load_offset;
pdpt_low_physical  = pdpt_low  + kernel_load_offset;
pdpt_high_physical = pdpt_high + kernel_load_offset;
pd_physical        = pd        + kernel_load_offset;
