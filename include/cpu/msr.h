#include <stdint.h>

static inline uint64_t msr_get(uint32_t msr) {
  uint32_t lo;
  uint32_t hi;

  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

  return ((uint64_t)hi << 32) | lo;
}

static inline void msr_set(uint32_t msr, uint64_t value) {
  uint32_t lo = (uint32_t)value;
  uint32_t hi = (uint32_t)(value >> 32);

  __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi) : "memory");
}
