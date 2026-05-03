#pragma once

#include "multiboot.h"
#include "panic.h"

#include <stddef.h>
#include <stdint.h>

struct free_list_node {
  size_t size;
  struct free_list_node *next;
};

struct memory {
  struct free_list_node *free_list;
  struct panic_sink *panic_sink;
};

struct memory_map {
  const struct multiboot_memory_map_entry *data;
  size_t size;
};

struct memory memory_create(const struct memory_map *memory_map,
                            struct panic_sink *panic_sink);
void *memory_allocate(struct memory *memory, size_t bytes);
