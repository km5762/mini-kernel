#pragma once

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

struct pages {
  uint32_t bitmap[32768];
};

struct memory_map {
  const struct multiboot_memory_map_entry *data;
  size_t size;
};

struct pages pages_init(const struct memory_map *memory_map);
uintptr_t pages_allocate();
void pages_free(uintptr_t address);
