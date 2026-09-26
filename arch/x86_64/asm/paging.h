#pragma once

// The kernel must exist in the top 2 GB for -mcmodel=kernel.
// Before paging_init maps all memory (after boot) we must use
// this offset to switch virtual <-> physical.
#define KERNEL_VIRTUAL_BASE 0xffffffff80000000

// 8 MB scratch space for paging_init to map all of memory
#define PAGE_BSS_SIZE (8 * 1024 * 1024)

#define PAGE_BYTES 4096
#define PAGE_ENTRY_ADDRESS(entry) (entry & 0x000ffffffffff000)
#define PAGE_LARGE_BYTES 2097152
#define PAGE_TABLE_BYTES 4096
#define PAGE_TABLE_ENTRIES 512
#define PAGE_ENTRY_BYTES 8
#define PAGE_ENTRY_PRESENT 1
#define PAGE_ENTRY_WRITABLE (1 << 1)
#define PAGE_ENTRY_LARGE (1 << 7)
#define PML4_INDEX(va) (((va) >> 39) & 0x1ff)
#define PDPT_INDEX(va) (((va) >> 30) & 0x1ff)
#define PD_INDEX(va) (((va) >> 21) & 0x1ff)
#define PT_INDEX(va) (((va) >> 12) & 0x1ff)

#if !defined(__ASSEMBLER__) && !defined(LD_SCRIPT)
#include <stdint.h>

// These must be used before paging_init
static inline uintptr_t
kernel_physical_to_virtual_address(uintptr_t physical_address) {
  return physical_address + KERNEL_VIRTUAL_BASE;
}

static inline uintptr_t
kernel_virtual_to_physical_address(uintptr_t virtual_address) {
  return virtual_address - KERNEL_VIRTUAL_BASE;
}

static const uintptr_t physmap_base = 0xffff800000000000;

// These are only safe to use after paging_init
static inline uintptr_t
physical_to_virtual_address(uintptr_t physical_address) {
  return physical_address + physmap_base;
}

static inline uintptr_t virtual_to_physical_address(uintptr_t virtual_address) {
  return virtual_address - physmap_base;
}

void paging_init(uintptr_t max_physical_address);
void flush_tlb();
#endif
