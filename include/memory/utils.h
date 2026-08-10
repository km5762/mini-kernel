#pragma once

#include <stddef.h>
#include <stdint.h>

void *memory_set(void *ptr, uint8_t value, size_t n);
void *memory_copy(void *dest, const void *src, size_t n);
void *memory_zero(void *ptr, size_t n);
