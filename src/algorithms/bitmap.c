#include "algorithms/bitmap.h"
#include "algorithms/bits.h"
#include "memory/utils.h"
#include <limits.h>

static bitmap_word *get_word(unsigned int *bitmap, size_t index) {
  if (bitmap == nullptr) {
    return nullptr;
  }

  const size_t word_index = index / bitmap_word_bits;
  return &bitmap[word_index];
}

void bitmap_set_range(bitmap_word *bitmap, size_t start, size_t size) {
  if (bitmap == nullptr) {
    return;
  }

  bitmap_word *word = get_word(bitmap, start);
  const size_t starting_bits = bitmap_word_bits - (start % bitmap_word_bits);
  *word |= ~0 << (bitmap_word_bits - starting_bits);

  const size_t end = start + size;
  word = get_word(bitmap, end);
  const size_t ending_bits = end % bitmap_word_bits;
  *word = WITH_BITS_SET(*word, ending_bits);

  const size_t set_start = start + starting_bits;
  const size_t set_end = end - ending_bits;
  const size_t set_size = set_end > set_start ? (set_end - set_start) / 8 : 0;
  memory_set(get_word(bitmap, set_start), ~0, set_size);
}

void bitmap_set(bitmap_word *bitmap, size_t index) {
  if (bitmap == nullptr) {
    return;
  }

  bitmap_word *word = get_word(bitmap, index);
  const size_t bit = index % bitmap_word_bits;
  *word |= 1u << bit;
}

void bitmap_clear(bitmap_word *bitmap, size_t index) {
  if (bitmap == nullptr) {
    return;
  }

  bitmap_word *word = get_word(bitmap, index);
  const size_t bit = index % bitmap_word_bits;
  *word &= ~(1u << bit);
}

int bitmap_find_first_set(struct bitmap_span span) {
  for (size_t i = 0; i < span.size; ++i) {
    if (span.data[i] == 0) {
      continue;
    }
    const int bit = __builtin_ctz(span.data[i]);
    return bit + i * bitmap_word_bits;
  }
  return -1;
}
