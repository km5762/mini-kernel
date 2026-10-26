#include <stdint.h>

static inline void ioapic_write(const uintptr_t apic_base, const uint8_t offset,
                  const uint32_t val) {
  /* tell IOREGSEL where we want to write to */
  *(volatile uint32_t *)(apic_base) = offset;
  /* write the value to IOWIN */
  *(volatile uint32_t *)(apic_base + 0x10) = val;
}

static inline uint32_t ioapic_read(const uintptr_t apic_base, const uint8_t offset) {
  /* tell IOREGSEL where we want to read from */
  *(volatile uint32_t *)(apic_base) = offset;
  /* return the data from IOWIN */
  return *(volatile uint32_t *)(apic_base + 0x10);
}
