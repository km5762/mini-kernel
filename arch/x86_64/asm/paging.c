// #include "asm/paging.h"
// #include "paging.h"
//
// extern uint64_t pml4[512];
//
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
