#include "asm/paging.h"
#include "algorithms/alignment.h"
#include "memory/memory_arena.h"
#include "memory/utils.h"
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
  // Before we clear the identity mappings (by setting the pml4 entry to 0)
  // we need to load the gtr at the high address so we won't fault
  __asm__ volatile("lgdt %0" ::"m"(*gdtr_high) : "memory");
  pml4[0] = 0;
  flush_tlb();
}

uintptr_t paging_init_direct_map(uintptr_t max_physical_address) {
  extern uint64_t pml4[];
  extern unsigned char page_bss[];
  struct memory_arena page_arena = memory_arena_create(page_bss, PAGE_BSS_SIZE);

  uintptr_t physical_address = 0;
  for (; physical_address + PAGE_LARGE_BYTES <= max_physical_address;
       physical_address += PAGE_LARGE_BYTES) {
    const uintptr_t virtual_address =
        physical_to_virtual_address(physical_address);
    uint64_t *pml4_entry = &pml4[PML4_INDEX(virtual_address)];
    if (!(*pml4_entry & PAGE_ENTRY_PRESENT)) {
      const uintptr_t allocated_page =
          (uintptr_t)memory_arena_allocate(&page_arena, PAGE_BYTES);

      if (!allocated_page) {
        return 0;
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
        return 0;
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
  return physical_address;
}

uintptr_t paging_map(uintptr_t physical_address, uintptr_t virtual_address,
                     uint64_t flags, struct page_pool *page_pool) {
  const uint64_t table_flags =
      PAGE_ENTRY_PRESENT | PAGE_ENTRY_WRITABLE | (flags & PAGE_ENTRY_USER);
  flags |= PAGE_ENTRY_PRESENT;

  uint64_t *pml4_entry = &pml4[PML4_INDEX(virtual_address)];
  if (!(*pml4_entry & PAGE_ENTRY_PRESENT)) {
    const uintptr_t allocated_page = page_pool_allocate(page_pool, 1);
    if (!allocated_page) {
      return 0;
    }

    memory_zero((void *)physical_to_virtual_address(allocated_page),
                PAGE_BYTES);
    *pml4_entry = allocated_page;
  }
  *pml4_entry |= table_flags;

  uintptr_t pdpt_physical = PAGE_ENTRY_ADDRESS(*pml4_entry);
  uint64_t *pdpt = (uint64_t *)physical_to_virtual_address(pdpt_physical);
  uint64_t *pdpt_entry = &pdpt[PDPT_INDEX(virtual_address)];
  if (!(*pdpt_entry & PAGE_ENTRY_PRESENT)) {
    const uintptr_t allocated_page = page_pool_allocate(page_pool, 1);
    if (!allocated_page) {
      return 0;
    }

    memory_zero((void *)physical_to_virtual_address(allocated_page),
                PAGE_BYTES);
    *pdpt_entry = allocated_page;
  }
  *pdpt_entry |= table_flags;

  uintptr_t pd_physical = PAGE_ENTRY_ADDRESS(*pdpt_entry);
  uint64_t *pd = (uint64_t *)physical_to_virtual_address(pd_physical);
  uint64_t *pd_entry = &pd[PD_INDEX(virtual_address)];
  if (flags & PAGE_ENTRY_LARGE) {
    *pd_entry = physical_address | flags;
    return virtual_address;
  }

  if ((*pd_entry & PAGE_ENTRY_PRESENT) && (*pd_entry & PAGE_ENTRY_LARGE)) {
    return 0;
  }

  if (!(*pd_entry & PAGE_ENTRY_PRESENT)) {
    const uintptr_t allocated_page = page_pool_allocate(page_pool, 1);
    if (!allocated_page) {
      return 0;
    }

    memory_zero((void *)physical_to_virtual_address(allocated_page),
                PAGE_BYTES);
    *pd_entry = allocated_page;
  }
  *pd_entry |= table_flags;

  uintptr_t pt_physical = PAGE_ENTRY_ADDRESS(*pd_entry);
  uint64_t *pt = (uint64_t *)physical_to_virtual_address(pt_physical);
  uint64_t *pt_entry = &pt[PT_INDEX(virtual_address)];
  *pt_entry = physical_address | flags;

  return virtual_address;
}

uintptr_t paging_map_range(uintptr_t physical_address,
                           uintptr_t virtual_address, size_t pages,
                           uint64_t flags, struct page_pool *page_pool) {
  for (size_t i = 0; i < pages; ++i) {
    const uintptr_t mapping =
        paging_map(physical_address + i * PAGE_BYTES,
                   virtual_address + i * PAGE_BYTES, flags, page_pool);
    if (!mapping) {
      return 0;
    }
  }

  return virtual_address;
}
