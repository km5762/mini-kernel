#include "kernel.h"
#include "algorithms/alignment.h"
#include "asm/paging.h"
#include "boot/info.h"
#include "graphics/font_bitmaps.h"
#include "graphics/graphics.h"
#include "graphics/terminal.h"
#include "memory/memory_pool.h"
#include "memory/virtual_address_arena.h"
#include "panic.h"
#include "uacpi/status.h"
#include "uacpi/tables.h"
#include "uacpi/uacpi.h"

#include <stddef.h>
#include <stdint.h>

static struct kernel kernel;

extern unsigned char kernel_physical_end[];

struct interrupt_context {};
void interrupt_dispatch(struct interrupt_context *ctx) {
  // switch (ctx->vector) {
  // case 0x40:
  //   lapic_write(lapic, 0x0B0, 0);
  //   terminal_print(&kernel.terminal, "TIMER TICK\n");
  //   break;
  // default:
  // }
}

void kernel_main(unsigned long magic, unsigned long multiboot_address) {
  (void)magic;
  struct boot_info boot_info = {0};
  boot_info_extract(kernel_physical_to_virtual_address(multiboot_address),
                    &boot_info);
  kernel.direct_map_physical_end =
      paging_init_direct_map(boot_info.memory.max_physical_address);
  const uintptr_t virtual_addresses_start = align_up(
      physical_to_virtual_address(boot_info.memory.max_physical_address),
      PAGE_LARGE_BYTES);
  const size_t virtual_addresses_size =
      KERNEL_VIRTUAL_BASE - virtual_addresses_start;
  kernel.virtual_address_arena = virtual_address_arena_create(
      virtual_addresses_start, virtual_addresses_size);
  kernel.page_pool = page_pool_create(
      &boot_info.memory, (uintptr_t)kernel_physical_end, kernel.panic_handler);
  kernel.memory_pool =
      memory_pool_create(&kernel.page_pool, kernel.panic_handler);
  kernel.graphics = graphics_create(&boot_info.framebuffer);
  kernel.terminal = terminal_create(&kernel.graphics, &uni2_terminus16);
  kernel.panic_handler = panic_create_handler_terminal(&kernel.terminal);

  uacpi_set_kernel(&kernel);
  uacpi_set_rsdp(
      kernel_virtual_to_physical_address((uintptr_t)boot_info.hardware.xsdp));
  static uint8_t early_buf[4096]; // temporary table bookkeeping

  uacpi_status st =
      uacpi_setup_early_table_access(early_buf, sizeof(early_buf));
  if (uacpi_unlikely_error(st)) { /* uacpi_status_to_string(st) */
  }
  //
  // uacpi_table tbl = {0};
  // st = uacpi_table_find_by_signature("MADT", &tbl);
  // if (uacpi_likely_success(st)) {
  //   struct acpi_madt *madt = (struct acpi_madt *)tbl.ptr;
  //   // parse entries...
  //   uacpi_table_unref(&tbl);
  // }

  for (;;)
    ;

  // idt_init();
  // lapic = apic_enable(kernel.panic_handler);
  // interrupts_enable();
  // interrupts_set_descriptor(0, zero_handler,
  //                           INTERRUPT_DESCRIPTOR_FLAGS_TRAP_GATE);
  // volatile int a = 1;
  // volatile int b = 0;
  // volatile int c = a / b;
  //
  // (void)c;
}
