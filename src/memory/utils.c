#include "memory/utils.h"

void *memory_set(void *ptr, uint8_t value, size_t n) {
  uint8_t *cursor = ptr;

  for (size_t i = 0; i < n; ++i) {
    *cursor = value;
    ++cursor;
  }

  return ptr;
}

void *memory_zero(void *ptr, size_t n) { return memory_set(ptr, 0, n); }
