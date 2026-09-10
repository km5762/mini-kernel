#pragma once

#include <limits.h>
#include <stddef.h>

typedef unsigned int bitmap_word;
static const size_t bitmap_word_bits = sizeof(bitmap_word) * CHAR_BIT;

struct bitmap_span {
  bitmap_word *data;
  size_t size;
};

void bitmap_set_range(bitmap_word *bitmap, size_t start, size_t size);
void bitmap_clear_range(bitmap_word *bitmap, size_t start, size_t size);
void bitmap_set(bitmap_word *bitmap, size_t index);
void bitmap_clear(bitmap_word *bitmap, size_t index);
int bitmap_find_first_set(struct bitmap_span span);
