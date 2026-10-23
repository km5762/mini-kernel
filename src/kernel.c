#include "kernel.h"
#include "algorithms/alignment.h"
#include "asm/paging.h"
#include "boot/info.h"
#include "debug/logging.h"
#include "graphics/font_bitmaps.h"
#include "graphics/graphics.h"
#include "graphics/terminal.h"
#include "memory/memory_pool.h"
#include "memory/virtual_address_arena.h"
#include "panic.h"
#include "uacpi/acpi.h"
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
  kernel.log_handler = log_create_handler_terminal(&kernel.terminal);
  uacpi_set_kernel(&kernel);
  uacpi_set_rsdp(
      kernel_virtual_to_physical_address((uintptr_t)boot_info.hardware.xsdp));
  static uint8_t early_buf[4096]; // temporary table bookkeeping

  uacpi_status st =
      uacpi_setup_early_table_access(early_buf, sizeof(early_buf));
  if (uacpi_unlikely_error(st)) { /* uacpi_status_to_string(st) */
    LOG_ERROR(kernel.log_handler, "%s", uacpi_status_to_string(st));
    return;
  }

  uacpi_table tbl = {0};
  st = uacpi_table_find_by_signature("APIC", &tbl);
  if (uacpi_unlikely_error(st)) {
    LOG_ERROR(kernel.log_handler, "%s", uacpi_status_to_string(st));
    return;
  }

  struct acpi_madt *madt = (struct acpi_madt *)tbl.ptr;
  uint64_t lapic_base = madt->local_interrupt_controller_address;
  bool pic_present = madt->flags & ACPI_PCAT_COMPAT;

  uint8_t *cur = (uint8_t *)madt->entries;
  uint8_t *end = (uint8_t *)madt + madt->hdr.length;

  while (cur + sizeof(struct acpi_entry_hdr) <= end) {
    struct acpi_entry_hdr *hdr = (struct acpi_entry_hdr *)cur;
    if (hdr->length < sizeof(*hdr) || cur + hdr->length > end)
      break; // malformed, bail out

    LOG_INFO(kernel.log_handler, "%d", hdr->type);

    switch (hdr->type) {
    case ACPI_MADT_ENTRY_TYPE_LAPIC: {
      struct acpi_madt_lapic *e = (struct acpi_madt_lapic *)hdr;
      break;
    }
    case ACPI_MADT_ENTRY_TYPE_IOAPIC: {
      struct acpi_madt_ioapic *e = (struct acpi_madt_ioapic *)hdr;
      break;
    }
    case ACPI_MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE: {
      struct acpi_madt_interrupt_source_override *e =
          (struct acpi_madt_interrupt_source_override *)hdr;
      LOG_INFO(kernel.log_handler, "source: %d, gsi: %d", e->source, e->gsi);
      break;
    }
    case ACPI_MADT_ENTRY_TYPE_LAPIC_ADDRESS_OVERRIDE: {
      struct acpi_madt_lapic_address_override *e =
          (struct acpi_madt_lapic_address_override *)hdr;
      break;
    }
    case ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC: {
      struct acpi_madt_x2apic *e = (struct acpi_madt_x2apic *)hdr;
      break;
    }
    case ACPI_MADT_ENTRY_TYPE_LAPIC_NMI:
    case ACPI_MADT_ENTRY_TYPE_NMI_SOURCE:
    case ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC_NMI:
      // configure NMI pins if you care
      break;
    default:
      break; // skip unknown types safely via hdr->length
    }

    cur += hdr->length;
  }

  uacpi_table_unref(&tbl);

  while (true)
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
