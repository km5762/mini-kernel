#pragma once

#include "acpi.h"

#include <stddef.h>
#include <stdint.h>

struct framebuffer_info {
  uint64_t address;
  uint32_t pitch;
  uint32_t width;
  uint32_t height;
  uint8_t bits_per_pixel;
};

enum memory_map_region_type : uint32_t {
  MEMORY_MAP_REGION_AVAILABLE = 1,
};

struct memory_map_region {
  uint64_t address;
  uint64_t size;
  enum memory_map_region_type type;
  uint32_t reserved;
};

struct memory_info {
  struct memory_map_region *data;
  size_t size;
  uintptr_t max_physical_address;
};

struct hardware_info {
  struct xsdp *xsdp;
};

struct boot_info {
  struct framebuffer_info framebuffer;
  struct memory_info memory;
  struct hardware_info hardware;
};

void boot_info_extract(uintptr_t address, struct boot_info *info);
