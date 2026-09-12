#pragma once

#include "multiboot.h"
#include "panic.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

typedef size_t memory_header;
struct memory_free_block {
  memory_header header;
  struct memory_free_block *next;
  struct memory_free_block *previous;
};

struct memory {
  struct memory_free_block *free_list;
  struct panic_handler *panic_handler;
};

struct memory memory_create(const struct multiboot_memory_map *memory_map,
                            struct panic_handler *panic_handler);
void *memory_allocate(struct memory *memory, size_t bytes);
void *memory_reserve(struct memory *memory, uintptr_t address, size_t bytes);
void memory_free(struct memory *memory, void *allocation);
