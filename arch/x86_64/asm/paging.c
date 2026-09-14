#include "asm/paging.h"

extern uint64_t pml4[512];

// void page_map(uintptr_t physical_address, uintptr_t virtual_address,
//               unsigned int flags) {
//   uint64_t *pml4_entry = &pml4[PML4_INDEX(physical_address)];
//   uint64_t *pdpt = if (*pml4_entry != 0 && *pml4_entry & PAGE_ENTRY_PRESENT)
//   {
//     *pml4_entry =
//   }
// }
