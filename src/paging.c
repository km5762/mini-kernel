#include "paging.h"
#include "multiboot.h"

static size_t address_to_page_index(uintptr_t address) {
  return address / PAGE_SIZE;
}

struct pages pages_init(const struct memory_map *memory_map) {
  struct memory_free_block *previous = nullptr;
  struct memory_free_block *first = nullptr;
  struct pages;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {

      continue;
    }

    const uintptr_t address =
        align_up(entry->address, alignof(max_align_t), panic_handler);
    const uintptr_t size = entry->size - (address - entry->address);
    if (size < sizeof(struct memory_free_block)) {
      continue;
    }

    struct memory_free_block *block = (struct memory_free_block *)address;
    *block = (struct memory_free_block){
        .header = size | MEMORY_HEADER_LAST_IN_REGION | MEMORY_HEADER_FREE,
        .previous = previous,
    };
    if (previous) {
      previous->next = block;
    }
    if (!first) {
      first = block;
    }
    previous = block;
  }

  const struct memory memory = {first, panic_handler};
  return memory;
}

uintptr_t page_allocate() {}

void page_free(uintptr_t address) {}
