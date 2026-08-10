#pragma once

#include "algorithms/bitmap.h"

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 512
#define PAGES_BITMAP_SIZE 32768

struct pages {
  bitmap_word bitmap[PAGES_BITMAP_SIZE];
};

struct memory_map {
  const struct multiboot_memory_map_entry *data;
  size_t size;
};

struct pages pages_init(const struct memory_map *memory_map);
uintptr_t pages_allocate(struct pages *pages);
void pages_free(struct pages *pages, uintptr_t address);
