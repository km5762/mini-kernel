#include "asm/paging.h"
#include "memory/memory_arena.h"
#include "memory/utils.h"
#include "paging.h"
#include <stddef.h>

extern uint64_t pd_low[];
extern uint64_t pml4[];
extern unsigned char page_bss[];
extern const uint8_t gdtr_high[];

// void page_map(uintptr_t physical_address, uintptr_t virtual_address,
//               unsigned int flags, struct page_pool *page_pool) {
//   uint64_t *pml4_entry = &pml4[PML4_INDEX(physical_address)];
//   const uintptr_t pdpt_address = HIGHER_HALF_ADDRESS((uintptr_t)pml4_entry);
//   if (!(pdpt_address & PAGE_ENTRY_PRESENT)) {
//     const uintptr_t page = page_pool_allocate(page_pool);
//     if (!page) {
//     }
//     *pml4_entry = page_pool_allocate(page_pool);
//   }
// }

static void clear_identity_map() {
  __asm__ volatile("lgdt %0" ::"m"(*gdtr_high) : "memory");
  memory_zero(pd_low, PAGE_TABLE_BYTES);
}

void paging_init(uintptr_t max_physical_address) {
  struct memory_arena page_arena = memory_arena_create(page_bss, PAGE_BSS_SIZE);

  for (uintptr_t physical_address = 0;
       physical_address + PAGE_LARGE_BYTES <= max_physical_address;
       physical_address += PAGE_LARGE_BYTES) {
    const uintptr_t virtual_address = HIGHER_HALF_ADDRESS(physical_address);
    uint64_t *pml4_entry = &pml4[PML4_INDEX(virtual_address)];
    if (!(*pml4_entry & PAGE_ENTRY_PRESENT)) {
      const uintptr_t allocated_page =
          (uintptr_t)memory_arena_allocate(&page_arena, PAGE_BYTES);

      if (!allocated_page) {
        return;
      }

      *pml4_entry = LOWER_HALF_ADDRESS(allocated_page) | PAGE_ENTRY_PRESENT |
                    PAGE_ENTRY_WRITABLE;
    }

    uintptr_t pdpt_physical = PAGE_ENTRY_ADDRESS(*pml4_entry);
    uint64_t *pdpt = (uint64_t *)HIGHER_HALF_ADDRESS(pdpt_physical);
    uint64_t *pdpt_entry = &pdpt[PDPT_INDEX(virtual_address)];
    if (!(*pdpt_entry & PAGE_ENTRY_PRESENT)) {
      const uintptr_t allocated_page =
          (uintptr_t)memory_arena_allocate(&page_arena, PAGE_BYTES);

      if (!allocated_page) {
        return;
      }

      *pdpt_entry = LOWER_HALF_ADDRESS(allocated_page) | PAGE_ENTRY_PRESENT |
                    PAGE_ENTRY_WRITABLE;
    }

    uintptr_t pd_physical = PAGE_ENTRY_ADDRESS(*pdpt_entry);
    uint64_t *pd = (uint64_t *)HIGHER_HALF_ADDRESS(pd_physical);
    uint64_t *pd_entry = &pd[PD_INDEX(virtual_address)];
    if (!(*pd_entry & PAGE_ENTRY_PRESENT)) {
      *pd_entry = physical_address | PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE |
                  PAGE_ENTRY_LARGE;
    }
  }

  clear_identity_map();
}
