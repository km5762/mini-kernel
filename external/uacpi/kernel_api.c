#include "uacpi/kernel_api.h"
#include "algorithms/alignment.h"
#include "algorithms/math.h"
#include "asm/paging.h"
#include "graphics/terminal.h"
#include "kernel.h"
#include "memory/virtual_address_arena.h"
#include "page_pool.h"
#include "uacpi/platform/types.h"
#include "uacpi/types.h"

static struct kernel *kernel;
static uacpi_phys_addr rsdp;

void uacpi_set_kernel(struct kernel *k) { kernel = k; }
void uacpi_set_rsdp(uacpi_phys_addr r) { rsdp = r; }

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
  if (out_rsdp_address == UACPI_NULL)
    return UACPI_STATUS_INVALID_ARGUMENT;

  *out_rsdp_address = rsdp;
  return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
  const uintptr_t aligned_address = align_down(addr, PAGE_BYTES);
  const ptrdiff_t difference = addr - aligned_address;
  const size_t pages = MATH_INT_CEILING_DIVIDE(len + difference, PAGE_BYTES);

  if (aligned_address + pages * PAGE_BYTES < kernel->direct_map_physical_end) {
    page_pool_reserve(&kernel->page_pool, aligned_address, pages);
    return (void *)physical_to_virtual_address(addr);
  }

  const uintptr_t virtual_address = virtual_address_arena_allocate(
      &kernel->virtual_address_arena, pages * PAGE_BYTES);
  if (!paging_map_range(aligned_address, virtual_address, pages, 0,
                        &kernel->page_pool)) {
    return UACPI_MAP_FAILED;
  };

  return (void *)virtual_address;
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
  const uintptr_t physical_address = (uintptr_t)addr;
  const uintptr_t aligned_address = align_down(physical_address, PAGE_BYTES);
  const ptrdiff_t difference = physical_address - aligned_address;
  const size_t pages = MATH_INT_CEILING_DIVIDE(len + difference, PAGE_BYTES);

  if (aligned_address + pages * PAGE_BYTES < kernel->direct_map_physical_end) {
    page_pool_free(&kernel->page_pool, aligned_address, pages);
    return;
  }

  return;
}

#ifndef UACPI_FORMATTED_LOGGING
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *message) {
  (void)level;
  terminal_print(&kernel->terminal, message);
}
#else
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *format, ...) {
  (void)level;
  (void)format;
}
#endif

#ifndef UACPI_BAREBONES_MODE

#ifdef UACPI_KERNEL_INITIALIZATION
uacpi_status uacpi_kernel_initialize(uacpi_init_level current_init_lvl) {
  (void)current_init_lvl;
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_deinitialize(void) {}
#endif

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address,
                                          uacpi_handle *out_handle) {
  (void)address;
  if (out_handle != UACPI_NULL)
    *out_handle = UACPI_NULL;
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_pci_device_close(uacpi_handle device) { (void)device; }

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset,
                                    uacpi_u8 *value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset,
                                     uacpi_u16 *value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset,
                                     uacpi_u32 *value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset,
                                     uacpi_u8 value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset,
                                      uacpi_u16 value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset,
                                      uacpi_u32 value) {
  (void)device;
  (void)offset;
  (void)value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len,
                                 uacpi_handle *out_handle) {
  (void)base;
  (void)len;
  if (out_handle != UACPI_NULL)
    *out_handle = UACPI_NULL;
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) { (void)handle; }

uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, uacpi_size offset,
                                   uacpi_u8 *out_value) {
  (void)handle;
  (void)offset;
  (void)out_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset,
                                    uacpi_u16 *out_value) {
  (void)handle;
  (void)offset;
  (void)out_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset,
                                    uacpi_u32 *out_value) {
  (void)handle;
  (void)offset;
  (void)out_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, uacpi_size offset,
                                    uacpi_u8 in_value) {
  (void)handle;
  (void)offset;
  (void)in_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, uacpi_size offset,
                                     uacpi_u16 in_value) {
  (void)handle;
  (void)offset;
  (void)in_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, uacpi_size offset,
                                     uacpi_u32 in_value) {
  (void)handle;
  (void)offset;
  (void)in_value;
  return UACPI_STATUS_UNIMPLEMENTED;
}

#ifdef UACPI_NATIVE_MMIO
uacpi_u8 uacpi_kernel_mmio_read8(void *addr) {
  (void)addr;
  return 0;
}

uacpi_u16 uacpi_kernel_mmio_read16(void *addr) {
  (void)addr;
  return 0;
}

uacpi_u32 uacpi_kernel_mmio_read32(void *addr) {
  (void)addr;
  return 0;
}

uacpi_u64 uacpi_kernel_mmio_read64(void *addr) {
  (void)addr;
  return 0;
}

void uacpi_kernel_mmio_write8(void *addr, uacpi_u8 value) {
  (void)addr;
  (void)value;
}

void uacpi_kernel_mmio_write16(void *addr, uacpi_u16 value) {
  (void)addr;
  (void)value;
}

void uacpi_kernel_mmio_write32(void *addr, uacpi_u32 value) {
  (void)addr;
  (void)value;
}

void uacpi_kernel_mmio_write64(void *addr, uacpi_u64 value) {
  (void)addr;
  (void)value;
}
#endif

void *uacpi_kernel_alloc(uacpi_size size) {
  (void)size;
  return UACPI_NULL;
}

#ifdef UACPI_NATIVE_ALLOC_ZEROED
void *uacpi_kernel_alloc_zeroed(uacpi_size size) {
  (void)size;
  return UACPI_NULL;
}
#endif

#ifndef UACPI_SIZED_FREES
void uacpi_kernel_free(void *mem) { (void)mem; }
#else
void uacpi_kernel_free(void *mem, uacpi_size size_hint) {
  (void)mem;
  (void)size_hint;
}
#endif

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) { return 0; }

void uacpi_kernel_stall(uacpi_u8 usec) { (void)usec; }

void uacpi_kernel_sleep(uacpi_u64 msec) { (void)msec; }

uacpi_handle uacpi_kernel_create_mutex(void) { return UACPI_NULL; }

void uacpi_kernel_free_mutex(uacpi_handle mutex) { (void)mutex; }

uacpi_handle uacpi_kernel_create_event(void) { return UACPI_NULL; }

void uacpi_kernel_free_event(uacpi_handle event) { (void)event; }

uacpi_thread_id uacpi_kernel_get_thread_id(void) { return (uacpi_thread_id)1; }

uacpi_interrupt_state uacpi_kernel_disable_interrupts(void) { return 0; }

void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state) {
  (void)state;
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle mutex, uacpi_u16 timeout) {
  (void)mutex;
  (void)timeout;
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_release_mutex(uacpi_handle mutex) { (void)mutex; }

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle event, uacpi_u16 timeout) {
  (void)event;
  (void)timeout;
  return UACPI_FALSE;
}

void uacpi_kernel_signal_event(uacpi_handle event) { (void)event; }

void uacpi_kernel_reset_event(uacpi_handle event) { (void)event; }

uacpi_status
uacpi_kernel_handle_firmware_request(uacpi_firmware_request *request) {
  (void)request;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx,
    uacpi_handle *out_irq_handle) {
  (void)irq;
  (void)handler;
  (void)ctx;
  if (out_irq_handle != UACPI_NULL)
    *out_irq_handle = UACPI_NULL;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status
uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler handler,
                                         uacpi_handle irq_handle) {
  (void)handler;
  (void)irq_handle;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock(void) { return UACPI_NULL; }

void uacpi_kernel_free_spinlock(uacpi_handle lock) { (void)lock; }

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle lock) {
  (void)lock;
  return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle lock, uacpi_cpu_flags flags) {
  (void)lock;
  (void)flags;
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type,
                                        uacpi_work_handler handler,
                                        uacpi_handle ctx) {
  (void)type;
  (void)handler;
  (void)ctx;
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

#endif
