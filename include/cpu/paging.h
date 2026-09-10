#pragma once

#include <stdint.h>

inline void paging_enable(uintptr_t table) {
  uint32_t x = 0;
  __asm__ volatile("movl %1, %%cr3;"
                   "movl %%cr0, %0;"
                   "orl $0x80000000, %0;"
                   "movl %0, %%cr0;"
                   : "+r"(x)
                   : "r"(table));
}
