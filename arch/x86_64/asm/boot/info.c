#include "boot/info.h"
#include "asm/boot/multiboot.h"

void boot_info_extract(uintptr_t address, struct boot_info *info) {
  const struct multiboot_tag *tag = (struct multiboot_tag *)(address + 8);
  while (tag->type != END) {
    const void *base = (uint8_t *)tag + 8;
    switch (tag->type) {
    case FRAMEBUFFER:
      struct framebuffer_info *framebuffer_info =
          (struct framebuffer_info *)base;
      info->framebuffer = *framebuffer_info;
      break;
    case MEMORY_MAP:
      struct multiboot_memory_map_tag *memory_map_tag =
          (struct multiboot_memory_map_tag *)base;
      const size_t header_size = sizeof(struct multiboot_tag) +
                                 sizeof(struct multiboot_memory_map_tag);
      const size_t entries =
          (tag->size - header_size) / memory_map_tag->entry_size;
      const struct memory_info memory_info = {
          .data = memory_map_tag->entries,
          .size = entries,
          .max_physical_address = multiboot_find_max_address(&memory_info)};
      info->memory = memory_info;
      break;
    case ACPI_RDSP:
      struct xsdp *xsdp = (struct xsdp *)base;
      const struct hardware_info hardware_info = {.xsdp = xsdp};
      info->hardware = hardware_info;
      break;
    case END:
      break;
    }
    tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
  }
}
