#include "memory.h"
#include "panic.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

// struct memory_map_tag_entry {
//   uint64_t address;
//   uint64_t size;
//   uint32_t type;
//   uint32_t reserved;
// };

struct memory memory_create(const struct memory_map *memory_map,
                            struct panic_handler *panic_handler) {
  struct free_list_node *last = nullptr;
  struct free_list_node *first = nullptr;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type != MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE) {
      continue;
    }

    if (entry->size < sizeof(struct free_list_node)) {
      continue;
    }

    const struct free_list_node node = {entry->size, nullptr};
    struct free_list_node *cursor = (struct free_list_node *)entry->address;
    *cursor = node;
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

static uintptr_t start_address(const struct free_list_node *node,
                               size_t alignment) {
  uintptr_t address = (uintptr_t)node;
  return align(address + sizeof(struct free_list_node), alignment);
}

static uintptr_t end_address(const struct free_list_node *node) {
  return (uintptr_t)node + node->size;
}

void *memory_allocate(struct memory *memory, size_t bytes) {
  struct free_list_node *last = nullptr;
  for (struct free_list_node *node = memory->free_list; node;
       node = node->next) {
    const uintptr_t allocation_start =
        start_address(node, alignof(max_align_t));
    const uintptr_t allocation_end = allocation_start + bytes;
    const uintptr_t node_end = end_address(node);
    if (allocation_end == node_end) {
      if (last) {
        last->next = node->next;
      } else {
        memory->free_list = node->next;
      }
      return (void *)allocation_start;
      break;
    } else if (allocation_end < node_end) {
      const uintptr_t next_node_start =
          align(allocation_end, alignof(struct free_list_node));
      const uintptr_t next_allocation_start =
          align(next_node_start + sizeof(struct free_list_node),
                alignof(max_align_t));
      if (next_allocation_start <= node_end) {
        struct free_list_node *next_node =
            (struct free_list_node *)next_node_start;
        *next_node =
            (struct free_list_node){node_end - next_node_start, node->next};
        node->size = next_node_start - (uintptr_t)node;
        if (last) {
          last->next = next_node;
        } else {
          memory->free_list = next_node;
        }
      }
    }
    last = node;
  }

  PANIC("Out of memory", memory->panic_handler);
  return nullptr;
}
