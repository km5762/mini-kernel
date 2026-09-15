#pragma once

#if !defined(__ASSEMBLER__) && !defined(LD_SCRIPT)
#include <stdint.h>
#endif

#define KERNEL_VIRTUAL_BASE 0xffffffff80000000
#define HIGHER_HALF_ADDRESS(address) (address + KERNEL_VIRTUAL_BASE)
#define LOWER_HALF_ADDRESS(va) (va - KERNEL_VIRTUAL_BASE)
#define PAGE_BYTES 4096
#define PAGE_ENTRY_ADDRESS(entry) (entry & 0x000ffffffffff000ULL)
#define PAGE_LARGE_BYTES 2097152
#define PAGE_TABLE_BYTES 4096
#define PAGE_ENTRY_BYTES 8
#define PAGE_ENTRY_PRESENT 1
#define PAGE_ENTRY_WRITABLE (1 << 1)
#define PAGE_ENTRY_LARGE (1 << 7)
#define PML4_INDEX(va) (((va) >> 39) & 0x1ff)
#define PDPT_INDEX(va) (((va) >> 30) & 0x1ff)
#define PD_INDEX(va) (((va) >> 21) & 0x1ff)
#define PT_INDEX(va) (((va) >> 12) & 0x1ff)

// void page_map(uintptr_t physical_address, uintptr_t virtual_address,
//               unsigned int flags);
