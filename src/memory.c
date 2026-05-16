#include "memory.h"
#include "panic.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

struct memory memory_create(const struct memory_map *memory_map,
                            struct panic_handler *panic_handler) {
  struct memory_free_block *last = nullptr;
  struct memory_free_block *first = nullptr;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    if (entry->size < sizeof(struct memory_free_block)) {
      continue;
    }

    const struct memory_free_block block = {entry->size, nullptr};
    struct memory_free_block *cursor =
        (struct memory_free_block *)entry->address;
    *cursor = block;
    if (last) {
      last->next = cursor;
    }
    if (!first) {
      first = cursor;
    }
  }

  const struct memory memory = {first, panic_handler};
  return memory;
}

static uintptr_t align(uintptr_t address, size_t alignment) {
  return (address + alignment - 1) & ~(alignment - 1);
}

static uintptr_t start_address(const struct memory_free_block *block,
                               size_t alignment) {
  uintptr_t address = (uintptr_t)block;
  return align(address + sizeof(struct memory_free_block), alignment);
}

static uintptr_t end_address(const struct memory_free_block *block) {
  return (uintptr_t)block + block->size;
}

void *memory_allocate(struct memory *memory, size_t bytes) {
  struct memory_free_block *last = nullptr;
  const size_t alignment = alignof(max_align_t);
  for (struct memory_free_block *block = memory->free_list; block;
       block = block->next) {
    const uintptr_t allocation_start = start_address(block, alignment);
    const uintptr_t allocation_end = allocation_start + bytes;
    const uintptr_t block_end = end_address(block);

    if (allocation_end <= block_end) {
      const uintptr_t next_block_start =
          align(allocation_end, alignof(struct memory_free_block));
      const uintptr_t next_allocation_start =
          align(next_block_start + sizeof(struct memory_free_block), alignment);

      struct memory_free_block *next = block->next;
      if (allocation_end < block_end && next_allocation_start <= block_end) {
        next = (struct memory_free_block *)next_block_start;

        *next = (struct memory_free_block){
            .size = block_end - next_block_start,
            .next = block->next,
        };

        block->size = next_block_start - (uintptr_t)block;
      }

      if (last) {
        last->next = next;
      } else {
        memory->free_list = next;
      }

      return (void *)allocation_start;
    }

    last = block;
  }

  PANIC("Out of memory", memory->panic_handler);
  return nullptr;
}
