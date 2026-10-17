#pragma once

#include "boot/info.h"

#include <stddef.h>
#include <stdint.h>

enum multiboot_tag_type : uint32_t {
  END = 0,
  MEMORY_MAP = 6,
  FRAMEBUFFER = 8,
  ACPI_RDSP = 14
};

struct multiboot_tag {
  enum multiboot_tag_type type;
  uint32_t size;
};

struct multiboot_memory_map_tag {
  uint32_t entry_size;
  uint32_t entry_version;
  struct memory_map_region entries[];
};

uintptr_t multiboot_find_max_address(const struct memory_info *memory_map);
