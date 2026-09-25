#include "memory/memory_pool.h"
#include "algorithms/math.h"
#include "asm/paging.h"
#include "data_structures/doubly_linked_list.h"
#include "data_structures/linked_list.h"
#include "page_pool.h"
#include "panic.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

enum slab_header_flag {
  SLAB_POOL_ALLOCATED = 0,
  SLAB_PAGE_ALLOCATED = 1,
};

typedef size_t slab_header;
typedef size_t slab_header_value;

static inline enum slab_header_flag header_flag(slab_header header) {
  return header & 1;
}

static inline slab_header_value header_value(slab_header header) {
  return header >> 1;
}

static inline void set_header_value(slab_header *header,
                                    slab_header_value value) {
  *header = (value << 1) | header_flag(*header);
}

static inline void set_header_flag(slab_header *header,
                                   enum slab_header_flag flag) {
  *header = header_value(*header) << 1 | flag;
}

static inline void increment_header_value(slab_header *header) {
  set_header_value(header, header_value(*header) + 1);
}

static inline void decrement_header_value(slab_header *header) {
  set_header_value(header, header_value(*header) - 1);
}

struct memory_pool_slab {
  slab_header header; // live allocation count
  struct list_link *free_list;
  struct list_double_link slab_list;
  struct memory_pool_cache *cache;
};

static const size_t slab_header_size =
    ((sizeof(struct memory_pool_slab) + alignof(max_align_t) - 1) &
     ~(alignof(max_align_t) - 1));

static const size_t page_header_size =
    ((sizeof(size_t) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1));

static const size_t min_size_class = 6;

struct memory_pool memory_pool_create(struct page_pool *page_pool,
                                      struct panic_handler panic_handler) {
  struct memory_pool pool = {0};
  pool.page_pool = page_pool;
  pool.panic_handler = panic_handler;

  for (size_t i = 0; i < CACHE_SIZE_CLASSES; ++i) {
    pool.caches[i].object_bytes = 1 << (min_size_class + i);
  }

  return pool;
}

static size_t cache_index(size_t requested_bytes) {
  size_t log2 = math_int_ceiling_log2(requested_bytes);
  return log2 - min_size_class;
}

static struct memory_pool_cache *find_cache(struct memory_pool *pool,
                                            size_t requested_bytes) {
  const size_t index = cache_index(requested_bytes);
  if (index >= CACHE_SIZE_CLASSES) {
    return nullptr;
  }

  return &pool->caches[index];
}

static void *allocate_from_page_pool(struct memory_pool *pool, size_t bytes) {
  const size_t pages =
      MATH_INT_CEILING_DIVIDE(bytes + page_header_size, PAGE_BYTES);
  uintptr_t allocation = page_pool_allocate(pool->page_pool, pages);
  if (!allocation) {
    return nullptr;
  }

  slab_header *header = (slab_header *)allocation;
  ASSERT(pages < SIZE_MAX >> 1, pool->panic_handler);
  set_header_value(header, pages);
  set_header_flag(header, SLAB_PAGE_ALLOCATED);
  return (void *)(allocation + page_header_size);
}

static void slab_init(struct memory_pool_slab *slab, size_t slab_pages,
                      struct memory_pool_cache *cache) {
  *slab = (struct memory_pool_slab){0};
  const uintptr_t end = (uintptr_t)slab + slab_pages * PAGE_BYTES;
  const uintptr_t start = (uintptr_t)slab + slab_header_size;
  for (uintptr_t cursor = start; cursor + cache->object_bytes <= end;
       cursor += cache->object_bytes) {
    struct list_link *link = (struct list_link *)cursor;
    linked_list_push(&slab->free_list, link);
  }
  set_header_value(&slab->header, 0);
  set_header_flag(&slab->header, SLAB_POOL_ALLOCATED);
  slab->cache = cache;
}

static void *allocate_from_slab(struct memory_pool_slab *slab) {
  struct list_link *link = linked_list_pop(&slab->free_list);
  increment_header_value(&slab->header);
  return link;
}

static bool slab_full(struct memory_pool_slab *slab) {
  return slab->free_list == nullptr;
}

void *memory_pool_allocate(struct memory_pool *pool, size_t bytes) {
  if (!pool) {
    return nullptr;
  }

  struct memory_pool_cache *cache = find_cache(pool, bytes);
  if (!cache) {
    return allocate_from_page_pool(pool, bytes);
  }

  struct list_double_link *free_slab_list = cache->free_slabs;
  struct memory_pool_slab *free_slab =
      free_slab_list ? LIST_LINK_TO_TYPE(free_slab_list,
                                         struct memory_pool_slab, slab_list)
                     : nullptr;

  if (!free_slab) {
    const uintptr_t slab_pages = 1;
    uintptr_t page = page_pool_allocate(pool->page_pool, slab_pages);
    if (!page) {
      return nullptr;
    }

    struct memory_pool_slab *slab = (struct memory_pool_slab *)page;
    slab_init(slab, slab_pages, cache);
    doubly_linked_list_push(&cache->free_slabs, &slab->slab_list);
    free_slab = slab;
  }

  void *allocation = allocate_from_slab(free_slab);
  if (slab_full(free_slab)) {
    doubly_linked_list_unlink(&cache->free_slabs, &free_slab->slab_list);
    doubly_linked_list_push(&cache->full_slabs, &free_slab->slab_list);
  }

  return allocation;
}

static inline uintptr_t find_slab_header(uintptr_t address) {
  static_assert(MATH_POWER_OF_2(PAGE_BYTES));
  return address & ~(PAGE_BYTES - 1);
}

static inline bool slab_empty(struct memory_pool_slab *slab) {
  return header_value(slab->header) == 0;
}

void memory_pool_free(struct memory_pool *pool, void *allocation) {
  const uintptr_t address = (uintptr_t)allocation;
  const uintptr_t header_address = find_slab_header(address);
  const slab_header header = *(slab_header *)header_address;
  const enum slab_header_flag flag = header_flag(header);

  switch (flag) {
  case SLAB_POOL_ALLOCATED: {
    struct memory_pool_slab *slab = (struct memory_pool_slab *)header_address;
    struct memory_pool_cache *cache = slab->cache;
    const bool was_full = slab_full(slab);

    decrement_header_value(&slab->header);
    if (slab_empty(slab)) {
      doubly_linked_list_unlink(
          was_full ? &cache->full_slabs : &cache->free_slabs, &slab->slab_list);
      page_pool_free(pool->page_pool, (uintptr_t)slab, 1);
      return;
    }

    if (was_full) {
      doubly_linked_list_unlink(&cache->full_slabs, &slab->slab_list);
      doubly_linked_list_push(&cache->free_slabs, &slab->slab_list);
    }
    linked_list_push(&slab->free_list, allocation);
    break;
  }
  case SLAB_PAGE_ALLOCATED: {
    const size_t pages = header_value(header);
    page_pool_free(pool->page_pool, address, pages);
    break;
  }
  }
}
