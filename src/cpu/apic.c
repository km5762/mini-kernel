#include "cpu/cpuid_features.h"
#include "cpu/msr.h"
#include "panic.h"

#include <cpuid.h>
#include <stdbool.h>
#include <stdint.h>

static const uintptr_t ia32_apic_base_msr = 0x1B;
static const uint64_t ia32_apic_base_msr_enable = 1 << 11;

bool apic_supported() {
  unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

  __cpuid(1, eax, ebx, ecx, edx);

  return edx & CPUID_FEATURE_EDX_APIC;
}

void apic_enable(struct panic_handler panic_handler) {
  ASSERT(apic_supported(), &panic_handler);
  uint64_t apic_base = msr_get(ia32_apic_base_msr);
  if (!(apic_base & ia32_apic_base_msr_enable)) {
    msr_set(ia32_apic_base_msr, apic_base | ia32_apic_base_msr_enable);
  }
  uintptr_t lapic_address = apic_base & 0xFFFFF000;
}
