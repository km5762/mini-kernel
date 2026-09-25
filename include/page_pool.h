#pragma once

#include "algorithms/bitmap.h"
#include "multiboot.h"
#include "panic.h"

#include <stddef.h>
#include <stdint.h>

struct page_pool {
  struct bitmap_span bitmap;
  uintptr_t base;
  struct panic_handler panic_handler;
};

struct page_pool page_pool_create(const struct multiboot_memory_map *memory_map,
                                  uintptr_t kernel_physical_end,
                                  struct panic_handler panic_handler);
uintptr_t page_pool_allocate(struct page_pool *pool, size_t pages);
uintptr_t page_pool_reserve(struct page_pool *pool, uintptr_t address,
                            size_t pages);
void page_pool_free(struct page_pool *pool, uintptr_t address, size_t pages);
