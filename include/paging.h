#pragma once

#include "algorithms/bitmap.h"
#include "multiboot.h"
#include "panic.h"

#include <stddef.h>
#include <stdint.h>

#define PAGES_BITMAP_SIZE 32768

struct page_pool {
  bitmap_word *bitmap;
};

struct page_pool page_pool_create(const struct multiboot_memory_map *memory_map,
                                  struct panic_handler *panic_handler);
uintptr_t page_pool_allocate(struct page_pool *pages);
uintptr_t page_pool_reserve(struct page_pool *pages, uintptr_t address,
                            size_t n_pages);
void page_pool_free(struct page_pool *pages, uintptr_t address);
