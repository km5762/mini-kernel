#include "paging.h"
#include "algorithms/bitmap.h"
#include "asm/paging.h"
#include "multiboot.h"

static size_t bitmap_index(uintptr_t address) { return address / PAGE_BYTES; }
static uintptr_t page_address(size_t index) { return index * PAGE_BYTES; }

struct pages pages_create(const struct multiboot_memory_map *memory_map) {
  struct pages pages = {0};
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    const size_t start_index = bitmap_index(entry->address);
    const size_t total_pages = entry->size / PAGE_BYTES;
    bitmap_set_range(pages.bitmap, start_index, total_pages);
  }
  return pages;
}

uintptr_t pages_allocate(struct pages *pages) {
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

void pages_free(struct pages *pages, uintptr_t address) {
  if (pages == nullptr) {
    return;
  }

  const size_t index = bitmap_index(address);
  bitmap_set(pages->bitmap, index);
}

uintptr_t pages_reserve(struct pages *pages, uintptr_t address,
                        size_t n_pages) {
  if (pages == nullptr) {
    return 0;
  }

  const size_t start_index = bitmap_index(address);
  bitmap_clear_range(pages->bitmap, start_index, n_pages);
  return address;
}
