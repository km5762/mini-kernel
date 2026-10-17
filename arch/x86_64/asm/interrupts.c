// #include "asm/interrupts.h"
// #include "asm/io.h"
// #include "asm/paging.h"
// #include "cpuid_features.h"
// #include "idt.h"
// #include "msr.h"
// #include "panic.h"
// #include <cpuid.h>
// #include <stdbool.h>
// #include <stdint.h>
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
// static inline uint32_t lapic_read(uint32_t *lapic, uint32_t reg) {
//   return *(volatile uint32_t *)((uintptr_t)lapic + reg);
// }
//
// static inline void lapic_write(uint32_t *lapic, uint32_t reg, uint32_t value)
// {
//   *(volatile uint32_t *)((uintptr_t)lapic + reg) = value;
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
// void interrupts_init(struct interrupt_dispatcher *dispatcher,
//                      struct panic_handler panic_handler) {
//   __asm__ volatile("cli");
//   dispatcher->panic_handler = panic_handler;
//   idt_init(dispatcher->idt);
//   dispatcher->lapic = apic_init(panic_handler);
//   __asm__ volatile("sti");
// }
