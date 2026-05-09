#include "font_bitmaps.h"
#include "graphics.h"
#include "memory.h"
#include "multiboot.h"
#include "panic.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

struct kernel {
  struct graphics graphics;
  struct terminal terminal;
  struct memory memory;
  struct panic_handler panic_handler;
};

static void parse_multiboot(size_t address, struct kernel *kernel) {
  const struct multiboot_tag *tag = (struct multiboot_tag *)(address + 8);
  while (tag->type != END) {
    const void *base = (uint8_t *)tag + 8;
    switch (tag->type) {
    case FRAMEBUFFER:;
      const struct framebuffer_info *framebuffer_info =
          (const struct framebuffer_info *)base;
      kernel->graphics = graphics_create(framebuffer_info);
      kernel->terminal = terminal_create(&kernel->graphics, &uni2_terminus16);
      kernel->panic_handler = create_panic_handler_terminal(&kernel->terminal);
      break;
    case MEMORY_MAP:;
      const struct multiboot_memory_map *memory_map_tag =
          (const struct multiboot_memory_map *)base;
      const size_t header_size =
          sizeof(struct multiboot_tag) + sizeof(struct multiboot_memory_map);
      const size_t entries =
          (tag->size - header_size) / memory_map_tag->entry_size;
      const struct memory_map memory_map = {memory_map_tag->entries, entries};
      kernel->memory = memory_create(&memory_map, &kernel->panic_handler);
      break;
    case END:
      break;
    }
    tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
  }
}

void kernel_main(unsigned long magic, unsigned long multiboot_address) {
  const unsigned long multiboot2_bootloader_magic = 0x36d76289;
  if (magic != multiboot2_bootloader_magic) {
    return;
  }
  struct kernel kernel = {0};
  parse_multiboot(multiboot_address, &kernel);
  graphics_set_screen(&kernel.graphics, 0x1e1e2e);
}
