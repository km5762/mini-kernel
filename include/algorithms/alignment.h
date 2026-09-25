#include <stddef.h>
#include <stdint.h>

static inline uintptr_t align_up(uintptr_t address, size_t alignment) {
  return (address + alignment - 1) & ~(alignment - 1);
}

static inline uintptr_t align_down(uintptr_t address, size_t alignment) {
  return address & ~(alignment - 1);
}
