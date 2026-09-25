#pragma once

#include "data_structures/doubly_linked_list.h"
#include "page_pool.h"
#include "panic.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

struct memory_pool_slab;

struct memory_pool_cache {
  struct list_double_link *free_slabs;
  struct list_double_link *full_slabs;
  size_t object_bytes;
};

#define CACHE_SIZE_CLASSES 6

struct memory_pool {
  struct memory_pool_cache caches[CACHE_SIZE_CLASSES];
  struct panic_handler panic_handler;
  struct page_pool *page_pool;
};

struct memory_pool memory_pool_create(struct page_pool *page_pool,
                                      struct panic_handler panic_handler);
void *memory_pool_allocate(struct memory_pool *memory, size_t bytes);
void *memory_pool_reserve(struct memory_pool *memory, uintptr_t address,
                          size_t bytes);
void memory_pool_free(struct memory_pool *memory, void *allocation);
