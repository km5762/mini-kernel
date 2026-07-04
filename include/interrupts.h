#include <stdint.h>

void interrupts_init();

enum interrupts_descriptor_flags {
  INTERRUPT_DESCRIPTOR_FLAGS_INTERRUPT_GATE = 0x8E,
  INTERRUPT_DESCRIPTOR_FLAGS_TRAP_GATE,
};

void interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
