#pragma once

#include <stddef.h>
#include <stdint.h>

enum multiboot_tag_type : uint32_t { END = 0, MEMORY_MAP = 6, FRAMEBUFFER = 8 };

struct multiboot_tag {
  enum multiboot_tag_type type;
  uint32_t size;
};

enum multiboot_memory_map_entry_type : uint32_t {
  MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE = 1,
};

struct multiboot_memory_map_entry {
  uint64_t address;
  uint64_t size;
  enum multiboot_memory_map_entry_type type;
  uint32_t reserved;
};

struct multiboot_memory_map_tag {
  uint32_t entry_size;
  uint32_t entry_version;
  struct multiboot_memory_map_entry entries[];
};

struct multiboot_memory_map {
  const struct multiboot_memory_map_entry *data;
  size_t size;
};
