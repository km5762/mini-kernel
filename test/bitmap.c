#include "algorithms/bitmap.h"
#include "unity.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static int bit_value(bitmap_word *bitmap, size_t index) {
  const size_t word = index / bitmap_word_bits;
  const size_t bit = index % bitmap_word_bits;
  return (bitmap[word] & (1u << bit)) != 0;
}

void setUp() {}
void tearDown() {}

void set_and_clear_single_bit() {
  bitmap_word b[2] = {0, 0};

  bitmap_set(b, 3);
  TEST_ASSERT_TRUE(bit_value(b, 3));

  bitmap_clear(b, 3);
  TEST_ASSERT_FALSE(bit_value(b, 3));
}

void set_range_within_single_word() {
  bitmap_word b[2] = {0, 0};
  const size_t start = 5;
  const size_t size = 10;

  bitmap_set_range(b, start, size);

  for (size_t i = 0; i < start; ++i) {
    TEST_ASSERT_FALSE(bit_value(b, i));
  }
  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_TRUE_MESSAGE(bit_value(b, i), "bit in range not set");
  }
  for (size_t i = start + size; i < bitmap_word_bits; ++i) {
    TEST_ASSERT_FALSE(bit_value(b, i));
  }
}

void set_range_across_words() {
  /* cross a word boundary */
  const size_t start = bitmap_word_bits - 4;
  const size_t size = 10; /* spans into next word */
  bitmap_word b[4] = {0, 0, 0, 0};

  bitmap_set_range(b, start, size);

  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
}

void clear_range_within_single_word() {
  bitmap_word b[2] = {~0, ~0};
  const size_t start = 5;
  const size_t size = 10;

  bitmap_clear_range(b, start, size);

  for (size_t i = 0; i < start; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_FALSE_MESSAGE(bit_value(b, i), "bit in range not clear");
  }
  for (size_t i = start + size; i < bitmap_word_bits; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
}

void clear_range_across_words() {
  /* cross a word boundary */
  const size_t start = bitmap_word_bits - 4;
  const size_t size = 10; /* spans into next word */
  bitmap_word b[4] = {~0, ~0, ~0, ~0};

  bitmap_clear_range(b, start, size);

  for (size_t i = 0; i < start; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_FALSE_MESSAGE(bit_value(b, i), "bit in range not clear");
  }
  for (size_t i = start + size; i < bitmap_word_bits; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
}

void find_first_set() {
  bitmap_word b[4] = {0, 0, 0, 1};

  TEST_ASSERT_EQUAL(3 * bitmap_word_bits,
                    bitmap_find_first_set((struct bitmap_span){b, 4}));

  bitmap_word b2[4] = {0, 0, 0, 0};

  TEST_ASSERT_EQUAL(-1, bitmap_find_first_set((struct bitmap_span){b2, 4}));
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(set_and_clear_single_bit);
  RUN_TEST(set_range_within_single_word);
  RUN_TEST(set_range_across_words);
  RUN_TEST(clear_range_within_single_word);
  RUN_TEST(clear_range_across_words);
  RUN_TEST(find_first_set);

  return UNITY_END();
}
