#include "interrupts.h"
#include <stdbool.h>
#include <stdint.h>

struct idt_entry {
  uint16_t isr_low;   // The lower 16 bits of the ISR's address
  uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS
                      // before calling the ISR
  uint8_t ist; // The IST in the TSS that the CPU will load into RSP; set to
               // zero for now
  uint8_t attributes; // Type and attributes; see the IDT page

  uint16_t
      isr_mid; // The higher 16 bits of the lower 32 bits of the ISR's address
  uint32_t isr_high; // The higher 32 bits of the ISR's address
  uint32_t reserved; // Set to zero
} __attribute__((packed));

#define IDT_MAX_DESCRIPTORS 256
__attribute__((aligned(0x10))) static struct idt_entry idt[256];

struct idtr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

static struct idtr idtr;

void exception_handler() {
  __asm__ volatile("cli; hlt"); // Completely hangs the computer
}

void interrupts_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
  struct idt_entry *descriptor = &idt[vector];

  descriptor->isr_low = (uint64_t)isr & 0xFFFF;
  descriptor->kernel_cs = 0x8;
  descriptor->ist = 0;
  descriptor->attributes = flags;

  descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
  descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
  descriptor->reserved = 0;
}

extern void *isr_stub_table[];
static bool vectors[IDT_MAX_DESCRIPTORS];

void interrupts_init() {
  idtr.base = (uintptr_t)&idt[0];
  idtr.limit = (uint16_t)sizeof(struct idt_entry) * IDT_MAX_DESCRIPTORS;

  for (uint8_t vector = 0; vector < 32; vector++) {
    interrupts_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    vectors[vector] = true;
  }

  __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
  __asm__ volatile("sti");                   // set the interrupt flag
}
