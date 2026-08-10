#include "memory/utils.h"

void *memory_set(void *ptr, uint8_t value, size_t n) {
  uint8_t *cursor = ptr;

  for (size_t i = 0; i < n; ++i) {
    *cursor = value;
    ++cursor;
  }

  return ptr;
}

void *memory_copy(void *dest, const void *src, size_t n) {
  const uint8_t *source = src;
  uint8_t *destination = dest;

  for (size_t i = 0; i < n; ++i) {
    destination[i] = source[i];
  }

  return dest;
}

void *memory_zero(void *ptr, size_t n) { return memory_set(ptr, 0, n); }

void *memset(void *ptr, int value, size_t n)
    __attribute__((alias("memory_set")));
void *memcpy(void *dest, const void *src, size_t n)
    __attribute__((alias("memory_copy")));
