#include "bitmaps.h"
#include "font_bitmaps.h"
#include "graphics.h"
#include "terminal.h"

#include <stddef.h>
#include <stdint.h>

enum multiboot_tag_type { END = 0, FRAMEBUFFER = 8 };

struct multiboot_tag {
  uint32_t type;
  uint32_t size;
};

struct kernel {
  struct graphics graphics;
  struct terminal terminal;
};

static void parse_multiboot(size_t address, struct kernel *kernel) {
  const struct multiboot_tag *tag = (struct multiboot_tag *)(address + 8);
  while (tag->type != END) {
    switch (tag->type) {
    case FRAMEBUFFER:;
      const struct framebuffer_info *framebuffer_info =
          (const struct framebuffer_info *)((uint8_t *)tag + 8);
      struct graphics graphics = graphics_create(framebuffer_info);
      struct terminal terminal = terminal_create(&graphics, &uni2_terminus16);
      kernel->graphics = graphics;
      kernel->terminal = terminal;
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
  graphics_set_screen(&kernel.graphics, 0x1e1e2e);
  parse_multiboot(multiboot_address, &kernel);
  terminal_print(&kernel.terminal, "Hello, world!\n");
  terminal_print(&kernel.terminal, "Hello, world!\n");
}
