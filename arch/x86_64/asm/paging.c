#include "asm/paging.h"
#include "memory/memory_arena.h"
#include "paging.h"
#include <stddef.h>

extern uint64_t pml4[];

void flush_tlb() {
  uint64_t cr3;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
  __asm__ volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

static void clear_identity_map() {
  extern const uint8_t gdtr_high[];
  __asm__ volatile("lgdt %0" ::"m"(*gdtr_high) : "memory");
  pml4[0] = 0;
  flush_tlb();
}

void paging_init(uintptr_t max_physical_address) {
  extern uint64_t pml4[];
  extern unsigned char page_bss[];
  struct memory_arena page_arena = memory_arena_create(page_bss, PAGE_BSS_SIZE);

  for (uintptr_t physical_address = 0;
       physical_address + PAGE_LARGE_BYTES <= max_physical_address;
       physical_address += PAGE_LARGE_BYTES) {
    const uintptr_t virtual_address =
        physical_to_virtual_address(physical_address);
    uint64_t *pml4_entry = &pml4[PML4_INDEX(virtual_address)];
    if (!(*pml4_entry & PAGE_ENTRY_PRESENT)) {
      const uintptr_t allocated_page =
          (uintptr_t)memory_arena_allocate(&page_arena, PAGE_BYTES);

      if (!allocated_page) {
        return;
      }

      *pml4_entry = kernel_virtual_to_physical_address(allocated_page) |
                    PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;
    }

    uintptr_t pdpt_physical = PAGE_ENTRY_ADDRESS(*pml4_entry);
    uint64_t *pdpt =
        (uint64_t *)kernel_physical_to_virtual_address(pdpt_physical);
    uint64_t *pdpt_entry = &pdpt[PDPT_INDEX(virtual_address)];
    if (!(*pdpt_entry & PAGE_ENTRY_PRESENT)) {
      const uintptr_t allocated_page =
          (uintptr_t)memory_arena_allocate(&page_arena, PAGE_BYTES);

      if (!allocated_page) {
        return;
      }

      *pdpt_entry = kernel_virtual_to_physical_address(allocated_page) |
                    PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE;
    }

    uintptr_t pd_physical = PAGE_ENTRY_ADDRESS(*pdpt_entry);
    uint64_t *pd = (uint64_t *)kernel_physical_to_virtual_address(pd_physical);
    uint64_t *pd_entry = &pd[PD_INDEX(virtual_address)];
    if (!(*pd_entry & PAGE_ENTRY_PRESENT)) {
      *pd_entry = physical_address | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE |
                  PAGE_ENTRY_LARGE;
    }
  }

  clear_identity_map();
}
