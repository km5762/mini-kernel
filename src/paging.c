#include "paging.h"
#include "algorithms/bitmap.h"
#include "algorithms/math.h"
#include "asm/paging.h"
#include "multiboot.h"

#include <stdint.h>

extern uintptr_t kernel_physical_end;

static size_t bitmap_index(uintptr_t address) { return address / PAGE_BYTES; }
static uintptr_t page_address(size_t index) { return index * PAGE_BYTES; }

struct page_pool page_pool_create(const struct multiboot_memory_map *memory_map,
                                  struct panic_handler *panic_handler) {
  struct page_pool pages = {0};

  uintptr_t highest_address = 0;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];
    const uintptr_t end = entry->address + entry->size;

    highest_address = MATH_MAX(highest_address, end);
  }

  // floor div - if the last page is partial, we ignore it
  const size_t total_pages = highest_address / PAGE_BYTES;
  const size_t pages_per_word = sizeof(bitmap_word) * 8;
  const size_t bitmap_words =
      MATH_INT_CEILING_DIVIDE(total_pages, pages_per_word);
  const size_t bitmap_bytes = bitmap_words * sizeof(bitmap_word);
  const size_t bitmap_pages = MATH_INT_CEILING_DIVIDE(bitmap_bytes, PAGE_BYTES);
  const size_t kernel_start_page = 0;
  const size_t kernel_end_page =
      MATH_INT_CEILING_DIVIDE(kernel_physical_end, PAGE_BYTES);
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];
    const uintptr_t end = entry->address + entry->size;
    const bool kernel_memory_region = 0 >= entry->address;
    ASSERT(!kernel_memory_region || kernel_physical_end < end, panic_handler);

    if (kernel_memory_region) {
      const size_t region_start_page = entry->address / PAGE_BYTES;
      const size_t region_end_page = MATH_INT_CEILING_DIVIDE(end, PAGE_BYTES);
      const size_t leading_contiguous_pages =
          kernel_start_page - region_start_page;
      const size_t trailing_contiguous_pages =
          region_end_page - kernel_end_page;
      const size_t largest_contiguous_pages =
          MATH_MAX(leading_contiguous_pages, trailing_contiguous_pages);
      if (largest_contiguous_pages >= bitmap_pages) {
        if (leading_contiguous_pages > bitmap_pages) {
          pages.bitmap = (bitmap_word *)entry->address;
        } else {
          pages.bitmap = (bitmap_word *)kernel_physical_end;
        }
      }
    } else if (entry->size >= bitmap_pages) {
      pages.bitmap = (bitmap_word *)entry->address;
    }
  }

  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    const size_t start_index = bitmap_index(entry->address);
    const size_t total_pages = entry->size / PAGE_BYTES;
    bitmap_set_range(pages.bitmap, start_index, total_pages);
  }

  page_pool_reserve(&pages, 0, kernel_end_page - kernel_start_page);
  page_pool_reserve(&pages, (uintptr_t)pages.bitmap, bitmap_pages);

  return pages;
}

uintptr_t page_pool_allocate(struct page_pool *pages) {
  if (pages == nullptr) {
    return 0;
  }

  const int free_page_index = bitmap_find_first_set(
      (struct bitmap_span){pages->bitmap, PAGES_BITMAP_SIZE});

  if (free_page_index < 0) {
    return 0;
  }

  bitmap_clear(pages->bitmap, free_page_index);
  return page_address(free_page_index);
}

void page_pool_free(struct page_pool *pages, uintptr_t address) {
  if (pages == nullptr) {
    return;
  }

  const size_t index = bitmap_index(address);
  bitmap_set(pages->bitmap, index);
}

uintptr_t page_pool_reserve(struct page_pool *pages, uintptr_t address,
                            size_t n_pages) {
  if (pages == nullptr) {
    return 0;
  }

  const size_t start_index = bitmap_index(address);
  bitmap_clear_range(pages->bitmap, start_index, n_pages);
  return address;
}
