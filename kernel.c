#include "graphics.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

enum multiboot_tag_type { END = 0, FRAMEBUFFER = 8 };

struct multiboot_tag {
  uint32_t type;
  uint32_t size;
};

static void parse_multiboot(size_t address) {
  const struct multiboot_tag *tag = (struct multiboot_tag *)(address + 8);
  while (tag->type != END) {
    switch (tag->type) {
    case FRAMEBUFFER:;
      const struct framebuffer_info *framebuffer_info =
          (const struct framebuffer_info *)((uint8_t *)tag + 8);
      graphics_initialize(framebuffer_info);
      break;
    case END:
      break;
    }
    tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
  }
}

void kernel_main(unsigned long magic, unsigned long addr) {
  parse_multiboot(addr);
  terminal_init(0xFFFFFF);
  terminal_print_line("Hello, world!");
}
