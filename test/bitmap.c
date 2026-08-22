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

void test_bitmap_set_and_clear_single_bit(void) {
  bitmap_word b[2] = {0, 0};

  bitmap_set(b, 3);
  TEST_ASSERT_TRUE(bit_value(b, 3));

  bitmap_clear(b, 3);
  TEST_ASSERT_FALSE(bit_value(b, 3));
}

void test_bitmap_set_range_within_single_word(void) {
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

void test_bitmap_set_range_across_words(void) {
  /* cross a word boundary */
  const size_t start = bitmap_word_bits - 4;
  const size_t size = 10; /* spans into next word */
  bitmap_word b[4] = {0, 0, 0, 0};

  bitmap_set_range(b, start, size);

  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
}

void test_bitmap_clear_range(void) {
  bitmap_word b[3];
  for (size_t i = 0; i < 3; ++i) b[i] = ~(bitmap_word)0u;

  const size_t start = 7;
  const size_t size = 20;

  bitmap_clear_range(b, start, size);

  for (size_t i = 0; i < start; ++i) {
    TEST_ASSERT_TRUE(bit_value(b, i));
  }
  for (size_t i = start; i < start + size; ++i) {
    TEST_ASSERT_FALSE_MESSAGE(bit_value(b, i), "bit in cleared range still set");
  }
}

void test_bitmap_find_first_set_and_none(void) {
  bitmap_word b[4];
  for (size_t i = 0; i < 4; ++i) b[i] = ~(bitmap_word)0u;

  /* no zero bits -> expect -1 */
  TEST_ASSERT_EQUAL_INT(-1, bitmap_find_first_set((struct bitmap_span){b, 4}));

  /* clear a single bit and ensure it's found */
  bitmap_clear_range(b, 10, 1);
  TEST_ASSERT_EQUAL_INT(10, bitmap_find_first_set((struct bitmap_span){b, 4}));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_bitmap_set_and_clear_single_bit);
  RUN_TEST(test_bitmap_set_range_within_single_word);
  RUN_TEST(test_bitmap_set_range_across_words);
  RUN_TEST(test_bitmap_clear_range);
  RUN_TEST(test_bitmap_find_first_set_and_none);

  return UNITY_END();
}
