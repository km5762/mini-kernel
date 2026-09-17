#include "multiboot.h"
#include "algorithms/math.h"

uintptr_t
multiboot_find_max_address(const struct multiboot_memory_map *memory_map) {
  uintptr_t highest_address = 0;
  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];
    const uintptr_t end = entry->address + entry->size;

    highest_address = MATH_MAX(highest_address, end);
  }

  return highest_address;
}
