#include "page_pool.h"
#include "algorithms/alignment.h"
#include "algorithms/bitmap.h"
#include "algorithms/math.h"
#include "asm/paging.h"
#include "multiboot.h"
#include "panic.h"

#include <stdint.h>

static size_t bitmap_index(const struct page_pool *page_pool,
                           uintptr_t address) {
  ASSERT(address >= page_pool->base, page_pool->panic_handler);
  return (address - page_pool->base) / PAGE_BYTES;
}

static uintptr_t page_address(uintptr_t base, size_t index) {
  return index * PAGE_BYTES + base;
}

struct page_pool page_pool_create(const struct multiboot_memory_map *memory_map,
                                  uintptr_t kernel_physical_end,
                                  struct panic_handler panic_handler) {
  struct page_pool pages = {0};
  pages.panic_handler = panic_handler;

  uintptr_t highest_address = 0;
  pages.base = UINTPTR_MAX;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];
    const uintptr_t end = entry->address + entry->size;

    highest_address = MATH_MAX(highest_address, end);
    pages.base = MATH_MIN(pages.base, entry->address);
  }
  highest_address = align_down(highest_address, PAGE_BYTES);
  pages.base = align_up(pages.base, PAGE_BYTES);

  const size_t total_pages = (highest_address - pages.base) / PAGE_BYTES;
  const size_t pages_per_word = sizeof(bitmap_word) * 8;
  const size_t bitmap_words =
      MATH_INT_CEILING_DIVIDE(total_pages, pages_per_word);
  const size_t bitmap_bytes = bitmap_words * sizeof(bitmap_word);
  const size_t bitmap_pages = MATH_INT_CEILING_DIVIDE(bitmap_bytes, PAGE_BYTES);
  const size_t kernel_end_page =
      MATH_INT_CEILING_DIVIDE(kernel_physical_end, PAGE_BYTES);
  uintptr_t bitmap_physical_address = 0;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    const size_t aligned_start = align_up(entry->address, PAGE_BYTES);
    const size_t aligned_end =
        align_down(entry->address + entry->size, PAGE_BYTES);
    const size_t total_pages = (aligned_end - aligned_start) / PAGE_BYTES;

    const bool kernel_memory_region = kernel_physical_end >= entry->address &&
                                      kernel_physical_end < aligned_end;

    if (kernel_memory_region) {
      const size_t region_end_page = aligned_end / PAGE_BYTES;
      const size_t trailing_contiguous_pages =
          region_end_page - kernel_end_page;
      if (trailing_contiguous_pages >= bitmap_pages) {
        bitmap_physical_address = align_up(kernel_physical_end, PAGE_BYTES);
        break;
      }
    } else if (total_pages >= bitmap_pages) {
      bitmap_physical_address = align_up(entry->address, PAGE_BYTES);
      break;
    }
  }

  ASSERT(bitmap_physical_address, panic_handler);
  pages.bitmap.data = (bitmap_word *)HIGHER_HALF_ADDRESS(bitmap_physical_address);
  pages.bitmap.size = bitmap_words;

  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    const size_t aligned_start = align_up(entry->address, PAGE_BYTES);
    const size_t aligned_end =
        align_down(entry->address + entry->size, PAGE_BYTES);
    const size_t total_pages = (aligned_end - aligned_start) / PAGE_BYTES;
    bitmap_set_range(pages.bitmap.data, bitmap_index(&pages, aligned_start),
                     total_pages);
  }

  page_pool_reserve(&pages, pages.base, kernel_end_page);
  page_pool_reserve(&pages, bitmap_physical_address, bitmap_pages);

  return pages;
}

uintptr_t page_pool_allocate(struct page_pool *pool, size_t pages) {
  if (pool == nullptr) {
    return 0;
  }

  const ssize_t free_page_index =
      bitmap_find_contiguous_set(pool->bitmap, pages);

  if (free_page_index < 0) {
    return 0;
  }

  bitmap_clear_range(pool->bitmap.data, free_page_index, pages);
  return page_address(pool->base, free_page_index);
}

void page_pool_free(struct page_pool *pool, uintptr_t address, size_t pages) {
  if (pool == nullptr) {
    return;
  }

  const size_t index = bitmap_index(pool, address);
  bitmap_set_range(pool->bitmap.data, index, pages);
}

uintptr_t page_pool_reserve(struct page_pool *pages, uintptr_t address,
                            size_t n_pages) {
  if (pages == nullptr) {
    return 0;
  }

  const size_t start_index = bitmap_index(pages, address);
  bitmap_clear_range(pages->bitmap.data, start_index, n_pages);
  return address;
}
