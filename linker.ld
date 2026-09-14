#include "asm/paging.h"

ENTRY(_start)

SECTIONS
{
  . = KERNEL_PHYSICAL_BASE;

  .boot :
  {
    KEEP(*(.rodata.multiboot))
    KEEP(*(.text.boot))
  }

  . += KERNEL_VIRTUAL_BASE;
  . = ALIGN(4K);

  .text : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
  {
    *(.text)
  }

  . = ALIGN(4K);

  .rodata : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
  {
    *(.rodata)
  }

  . = ALIGN(4K);

  .data : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
  {
    *(.data)
  }

  . = ALIGN(4K);

  .bss : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
  {
    *(COMMON)
    *(.bss)
  }

  . = ALIGN(4K);

  .data.page : AT(ADDR(.data.page) - KERNEL_VIRTUAL_BASE)
  {
    *(.data.page)
  }

  . = ALIGN(4K);
  .bss.page : AT(ADDR(.bss.page) - KERNEL_VIRTUAL_BASE)
  {
    *(.bss.page)
  }

  kernel_virtual_end = .;
  kernel_physical_end = kernel_virtual_end - KERNEL_VIRTUAL_BASE;
}

gdt_physical       = gdt       - KERNEL_VIRTUAL_BASE;
gdtr_physical      = gdtr      - KERNEL_VIRTUAL_BASE;
pml4_physical      = pml4      - KERNEL_VIRTUAL_BASE;
pdpt_low_physical  = pdpt_low  - KERNEL_VIRTUAL_BASE;
pdpt_high_physical = pdpt_high - KERNEL_VIRTUAL_BASE;
pd_low_physical    = pd_low    - KERNEL_VIRTUAL_BASE;
pd_high_physical   = pd_high   - KERNEL_VIRTUAL_BASE;
page_bss_physical        = page_bss        - KERNEL_VIRTUAL_BASE;
