// #include "interrupts.h"
// #include "asm/interrupts.h"
// #include "asm/io.h"
// #include "asm/ioapic.h"
// #include "asm/paging.h"
// #include "cpuid_features.h"
// #include "msr.h"
// #include "panic.h"
// #include <cpuid.h>
// #include <stdbool.h>
// #include <stdint.h>
//
// #define IDT_ENTRIES 256
//
// #define LAPIC_REG_LVT_TIMER 0x320
// #define LAPIC_REG_TIMER_DIV 0x3E0
// #define LAPIC_REG_TIMER_INIT_COUNT 0x380
// #define LAPIC_REG_TIMER_CUR_COUNT 0x390
//
// #define LAPIC_LVT_TIMER_PERIODIC (1 << 17)
// #define LAPIC_LVT_MASKED (1 << 16)
//
// #define TIMER_VECTOR 0x40
// #define LAPIC_TIMER_DIV_16 0x3 // divide bus clock by 16
//
// struct idt_entry {
//   uint16_t isr_low;   // The lower 16 bits of the ISR's address
//   uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS
//                       // before calling the ISR
//   uint8_t ist; // The IST in the TSS that the CPU will load into RSP; set to
//                // zero for now
//   uint8_t attributes; // Type and attributes; see the IDT page
//
//   uint16_t
//       isr_mid; // The higher 16 bits of the lower 32 bits of the ISR's address
//   uint32_t isr_high; // The higher 32 bits of the ISR's address
//   uint32_t reserved; // Set to zero
// } __attribute__((packed));
//
// enum interrupt_controller {
//   INTERRUPT_CONTROLLER_IOAPIC,
// };
//
// struct interrupt_controller_ioapic {
//   uint32_t gsi;
// };
//
// struct interrupt_frame {
//   // Registers saved by isr_common_stub
//   uint64_t r15;
//   uint64_t r14;
//   uint64_t r13;
//   uint64_t r12;
//   uint64_t r11;
//   uint64_t r10;
//   uint64_t r9;
//   uint64_t r8;
//   uint64_t rbp;
//   uint64_t rdi;
//   uint64_t rsi;
//   uint64_t rdx;
//   uint64_t rcx;
//   uint64_t rbx;
//   uint64_t rax;
//
//   // Pushed by the ISR stub
//   uint64_t vector;
//   uint64_t error_code;
//
//   // Pushed by the CPU
//   uint64_t rip;
//   uint64_t cs;
//   uint64_t rflags;
//
//   // Only present when the CPU switches stacks
//   uint64_t rsp;
//   uint64_t ss;
// };
//
// struct interrupt_handler {
//   void (*handle)(struct interrupt_handler *handler,
//                  struct interrupt_frame *frame);
//   void *data;
//   enum interrupt_controller controller;
//   union {
//     struct interrupt_controller_ioapic ioapic;
//   };
// };
//
// static struct {
//   struct idt_entry idt[IDT_ENTRIES];
//   uint32_t *lapic;
//   uintptr_t ioapic_address;
//   uint32_t gsi_base;
//   struct interrupt_handler handlers[IDT_ENTRIES];
//   size_t next_handler;
//   struct panic_handler panic_handler;
// } interrupts;
//
// void interrupt_dispatch(struct interrupt_frame *frame) {
//   struct interrupt_handler *handler = &interrupts.handlers[frame->vector];
//
//   if (!handler->handle) {
//     return;
//   }
//
//   handler->handle(handler, frame);
// }
//
// static inline uint32_t lapic_read(uint32_t *lapic, uint32_t reg) {
//   return *(volatile uint32_t *)((uintptr_t)lapic + reg);
// }
//
// static inline void lapic_write(uint32_t *lapic, uint32_t reg, uint32_t value) {
//   *(volatile uint32_t *)((uintptr_t)lapic + reg) = value;
// }
//
// void lapic_timer_arm_test(uint32_t *lapic) {
//   lapic_write(lapic, LAPIC_REG_TIMER_DIV, LAPIC_TIMER_DIV_16);
//   lapic_write(lapic, LAPIC_REG_LVT_TIMER,
//               TIMER_VECTOR | LAPIC_LVT_TIMER_PERIODIC);
//   lapic_write(lapic, LAPIC_REG_TIMER_INIT_COUNT, 0x100000);
// }
//
// static void idt_set_gate(struct idt_entry *idt, uint8_t vector, void *isr,
//                          uint8_t flags) {
//   struct idt_entry *descriptor = &idt[vector];
//
//   descriptor->isr_low = (uint64_t)isr & 0xFFFF;
//   descriptor->kernel_cs = 0x8;
//   descriptor->ist = 0;
//   descriptor->attributes = flags;
//
//   descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
//   descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
//   descriptor->reserved = 0;
// }
//
// static void idt_init(struct idt_entry *idt) {
//   extern void *isr_stub_table[];
//
//   struct {
//     uint16_t limit;
//     struct idt_entry *base;
//   } __attribute__((packed)) idtr = {IDT_ENTRIES, idt};
//
//   for (uint8_t vector = 0; vector < 32; vector++) {
//     idt_set_gate(idtr.base, vector, isr_stub_table[vector], 0x8E);
//   }
//
//   idt_set_gate(idtr.base, 0x40, isr_stub_table[32], 0x8E);
//
//   __asm__ volatile("lidt %0" : : "m"(idtr));
// }
//
// static bool apic_supported() {
//   unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
//
//   __cpuid(1, eax, ebx, ecx, edx);
//
//   return edx & CPUID_FEATURE_EDX_APIC;
// }
//
// static void pic_disable() {
//   const uint8_t mask = 0xff;
//   const uint16_t master_pic_data = 0x21;
//   const uint16_t slave_pic_data = 0xa1;
//   outb(master_pic_data, mask);
//   outb(slave_pic_data, mask);
// }
//
// uint32_t *apic_init(struct panic_handler panic_handler) {
//   ASSERT(apic_supported(), panic_handler);
//   pic_disable();
//   const uintptr_t ia32_apic_base_msr = 0x1b;
//   const uint64_t ia32_apic_base_msr_enable = 1 << 11;
//   const uint32_t lapic_reg_svr = 0xf0;
//   const uint32_t lapic_svr_apic_enable = 1 << 8;
//   const uint32_t lapic_spurious_vector = 0xff;
//
//   uint64_t apic_base = rdmsr(ia32_apic_base_msr);
//   if (!(apic_base & ia32_apic_base_msr_enable)) {
//     wrmsr(ia32_apic_base_msr, apic_base | ia32_apic_base_msr_enable);
//   }
//
//   uintptr_t lapic_physical = apic_base & 0xfffff000;
//   uint32_t *lapic = (uint32_t *)physical_to_virtual_address(lapic_physical);
//   uint32_t svr = lapic_read(lapic, lapic_reg_svr);
//   svr |= lapic_svr_apic_enable;
//   svr = (svr & ~0xffu) | lapic_spurious_vector;
//   lapic_write(lapic, lapic_reg_svr, svr);
//
//   return lapic;
// }
//
// void interrupts_init(struct panic_handler panic_handler) {
//   __asm__ volatile("cli");
//   interrupts.panic_handler = panic_handler;
//   idt_init(interrupts.idt);
//   interrupts.lapic = apic_init(panic_handler);
//   __asm__ volatile("sti");
// }
//
// static struct interrupt_handler *allocate_handler() {
//   if (interrupts.next_handler >= IDT_ENTRIES) {
//     return nullptr;
//   }
//
//   return &interrupts.handlers[interrupts.next_handler++];
// }
//
// struct interrupt_handler *interrupts_register_gsi(uint32_t gsi) {
//   struct interrupt_handler *handler = allocate_handler();
//   handler->controller = INTERRUPT_CONTROLLER_IOAPIC;
//   handler->ioapic = (struct interrupt_controller_ioapic){.gsi = gsi};
//   return handler;
// }
//
// void interrupt_handler_enable(struct interrupt_handler *handler) {
//   switch (handler->controller) {
//   case INTERRUPT_CONTROLLER_IOAPIC:
//     ioapic_write(interrupts.ioapic_address,
//                  handler->ioapic.gsi - interrupts.gsi_base, );
//     break;
//   }
// }
//
// void interrupt_handler_disable(struct interrupt_handler *handler);
