#include "asm/paging.h"
#include "cpu/interrupts.h"
#include "graphics/font_bitmaps.h"
#include "graphics/graphics.h"
#include "graphics/terminal.h"
#include "memory/memory_pool.h"
#include "multiboot.h"
#include "panic.h"

#include <stddef.h>
#include <stdint.h>

struct kernel {
  struct graphics graphics;
  struct terminal terminal;
  struct memory_pool memory_pool;
  struct page_pool page_pool;
  struct panic_handler panic_handler;
};

static struct kernel kernel;

extern uintptr_t kernel_physical_end;

static void parse_multiboot(size_t address) {
  const struct multiboot_tag *tag = (struct multiboot_tag *)(address + 8);
  while (tag->type != END) {
    const void *base = (uint8_t *)tag + 8;
    switch (tag->type) {
    case FRAMEBUFFER:;
      const struct framebuffer_info *framebuffer_info =
          (const struct framebuffer_info *)base;
      kernel.graphics = graphics_create(framebuffer_info);
      kernel.terminal = terminal_create(&kernel.graphics, &uni2_terminus16);
      kernel.panic_handler = panic_create_handler_terminal(&kernel.terminal);
      break;
    case MEMORY_MAP:;
      const struct multiboot_memory_map_tag *memory_map_tag =
          (const struct multiboot_memory_map_tag *)base;
      const size_t header_size = sizeof(struct multiboot_tag) +
                                 sizeof(struct multiboot_memory_map_tag);
      const size_t entries =
          (tag->size - header_size) / memory_map_tag->entry_size;
      const struct multiboot_memory_map memory_map = {memory_map_tag->entries,
                                                      entries};
      const uintptr_t max_address = multiboot_find_max_address(&memory_map);
      paging_init(max_address);
      kernel.page_pool = page_pool_create(&memory_map, kernel_physical_end,
                                          kernel.panic_handler);
      kernel.memory_pool =
          memory_pool_create(&kernel.page_pool, kernel.panic_handler);
      break;
    case END:
      break;
    }
    tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
  }
}

void zero_handler() {
  terminal_print(&kernel.terminal, "ZERO HANDLED\n");
  for (;;)
    ;
}

void kernel_main(unsigned long magic, unsigned long multiboot_address) {
  (void)magic;
  parse_multiboot(HIGHER_HALF_ADDRESS(multiboot_address));
  graphics_set_screen(&kernel.graphics, 0x1e1e2e);
  terminal_print(&kernel.terminal, "HELLO\n");
  // interrupts_init();
  // interrupts_set_descriptor(0, zero_handler,
  //                           INTERRUPT_DESCRIPTOR_FLAGS_TRAP_GATE);
  // volatile int a = 1;
  // volatile int b = 0;
  // volatile int c = a / b;
  //
  // (void)c;
}
