#include "memory.h"
#include "panic.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

typedef size_t memory_footer;

static const size_t header_size =
    ((sizeof(memory_header) + alignof(max_align_t) - 1) &
     ~(alignof(max_align_t) - 1));

static uintptr_t align_up(uintptr_t address, size_t alignment) {
  return (address + alignment - 1) & ~(alignment - 1);
}

static uintptr_t align_down(uintptr_t address, size_t alignment) {
  return address & ~(alignment - 1);
}

enum {
  MEMORY_HEADER_PREVIOUS_ADJACENT_FREE = (size_t)1 << 0,
  MEMORY_HEADER_FREE = (size_t)1 << 1,
  MEMORY_HEADER_LAST_IN_REGION = (size_t)1 << 2,
};

struct memory memory_create(const struct memory_map *memory_map,
                            struct panic_handler *panic_handler) {
  struct memory_free_block *previous = nullptr;
  struct memory_free_block *first = nullptr;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    const uintptr_t address = align_up(entry->address, alignof(max_align_t));
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

static const size_t flags_mask = 0x7;
size_t block_size(memory_header header) { return header & ~(flags_mask); }
void set_block_size(memory_header *header, size_t size) {
  *header = (*header & flags_mask) | (size & ~0x7);
}

static void unlink_block(struct memory *memory,
                         struct memory_free_block *block) {
  if (block->previous) {
    block->previous->next = block->next;
  } else {
    memory->free_list = block->next;
  }

  if (block->next) {
    block->next->previous = block->previous;
  }
}

static void push_block(struct memory *memory, struct memory_free_block *block) {
  block->previous = nullptr;
  block->next = memory->free_list;
  memory->free_list = block;
}

void *memory_allocate(struct memory *memory, size_t bytes) {
  for (struct memory_free_block *block = memory->free_list; block;
       block = block->next) {
    const uintptr_t allocation_start = (uintptr_t)block + header_size;
    ASSERT(allocation_start % alignof(max_align_t) == 0, memory->panic_handler);
    const uintptr_t allocation_end = allocation_start + bytes;
    const uintptr_t block_end = (uintptr_t)block + block_size(block->header);

    if (allocation_end > block_end) {
      continue;
    }

    const uintptr_t next_block_start =
        align_up(allocation_end, alignof(max_align_t));
    const bool needs_footer = !(block->header & MEMORY_HEADER_LAST_IN_REGION);
    const uintptr_t next_footer_start = block_end - sizeof(memory_footer);
    ASSERT(!needs_footer || next_footer_start % alignof(memory_footer) == 0,
           memory->panic_handler);
    const uintptr_t next_block_free_block_end =
        next_block_start + sizeof(struct memory_free_block);
    const bool can_split =
        next_block_free_block_end <= block_end &&
        (!needs_footer || next_footer_start > next_block_free_block_end);
    if (can_split) {
      struct memory_free_block *new_block =
          (struct memory_free_block *)next_block_start;
      const size_t new_block_size = block_end - next_block_start;
      const memory_header new_header =
          new_block_size | ((block->header & MEMORY_HEADER_LAST_IN_REGION) &
                            ~MEMORY_HEADER_PREVIOUS_ADJACENT_FREE);
      *new_block = (struct memory_free_block){.header = new_header,
                                              .next = block->next,
                                              .previous = block->previous};

      set_block_size(&block->header, next_block_start - (uintptr_t)block);
      unlink_block(memory, block);
      push_block(memory, new_block);

      memory_footer *footer = (memory_footer *)next_footer_start;
      *footer = new_block_size;
    } else {
      unlink_block(memory, block);
    }
    block->header &= ~MEMORY_HEADER_FREE;

    if (!((block->header) & MEMORY_HEADER_LAST_IN_REGION)) {
      memory_header *next_adjacent_header =
          (memory_header *)((uintptr_t)block + block_size(block->header));
      *next_adjacent_header &= ~MEMORY_HEADER_PREVIOUS_ADJACENT_FREE;
    }

    return (void *)allocation_start;
  }

  return nullptr;
}

void memory_free(struct memory *memory, void *allocation) {
  const memory_header *freed_block_header =
      (memory_header *)((uintptr_t)allocation - header_size);
  struct memory_free_block *freed_block =
      (struct memory_free_block *)freed_block_header;
  const memory_footer previous_block_size = *(freed_block_header - 1);
  struct memory_free_block *previous_block =
      (struct memory_free_block *)((uintptr_t)freed_block_header -
                                   previous_block_size);
  const size_t freed_block_size = block_size(*freed_block_header);
  struct memory_free_block *next_block =
      (struct memory_free_block *)((uintptr_t)freed_block_header +
                                   freed_block_size);
  const size_t next_block_size = block_size(next_block->header);

  const bool previous_mergable =
      *freed_block_header & MEMORY_HEADER_PREVIOUS_ADJACENT_FREE;
  const bool next_mergable =
      !(*freed_block_header & MEMORY_HEADER_LAST_IN_REGION) &&
      next_block->header & MEMORY_HEADER_FREE;

  if (previous_mergable && next_mergable) {
    unlink_block(memory, next_block);
    set_block_size(&previous_block->header,
                   previous_block_size + freed_block_size + next_block_size);
    previous_block->header |=
        (next_block->header & MEMORY_HEADER_LAST_IN_REGION) |
        MEMORY_HEADER_FREE;
  } else if (previous_mergable) {
    set_block_size(&previous_block->header,
                   previous_block_size + freed_block_size);
    previous_block->header |= MEMORY_HEADER_FREE;
  } else if (next_mergable) {
    unlink_block(memory, next_block);
    set_block_size(&freed_block->header, freed_block_size + next_block_size);
    freed_block->header |= (next_block->header & MEMORY_HEADER_LAST_IN_REGION) |
                           MEMORY_HEADER_FREE;
    push_block(memory, freed_block);
  } else {
    freed_block->header |= MEMORY_HEADER_FREE;
    push_block(memory, freed_block);
  }
}
