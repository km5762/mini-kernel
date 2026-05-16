#pragma once

#include "multiboot.h"
#include "panic.h"

#include <stddef.h>
#include <stdint.h>

struct memory_free_block {
  size_t size;
  struct memory_free_block *next;
};

struct memory {
  struct memory_free_block *free_list;
  struct panic_handler *panic_handler;
};

struct memory_map {
  const struct multiboot_memory_map_entry *data;
  size_t size;
};

struct memory memory_create(const struct memory_map *memory_map,
                            struct panic_handler *panic_handler);
void *memory_allocate(struct memory *memory, size_t bytes);
