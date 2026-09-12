#pragma once

#include "algorithms/bitmap.h"
#include "multiboot.h"

#include <stddef.h>
#include <stdint.h>

#define PAGES_BITMAP_SIZE 32768

struct pages {
  bitmap_word *bitmap;
};

struct pages pages_create(const struct multiboot_memory_map *memory_map);
uintptr_t pages_allocate(struct pages *pages);
uintptr_t pages_reserve(struct pages *pages, uintptr_t address, size_t n_pages);
void pages_free(struct pages *pages, uintptr_t address);
