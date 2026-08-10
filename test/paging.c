#include "multiboot.h"
#include "paging.h"

#include "unity.h"

#include <inttypes.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void setUp() {}

void tearDown() {}

/*
 * These tests assume a 4 KiB page size.
 */
#define PAGE_BITS 4096u
#define BITMAP_BITS (sizeof(unsigned int) * 8u)
#define MAX_PAGES (32768u * BITMAP_BITS)

/*
 * Helpers
 */

static struct multiboot_memory_map_entry
memory_entry(uint64_t address, uint64_t size,
             enum multiboot_memory_map_entry_type type) {
  return (struct multiboot_memory_map_entry){
      .address = address,
      .size = size,
      .type = type,
      .reserved = 0,
  };
}

static struct memory_map
make_memory_map(const struct multiboot_memory_map_entry *entries,
                size_t count) {
  return (struct memory_map){
      .data = entries,
      .size = count * sizeof(*entries),
  };
}

/*
 * Initialization
 */

void test_pages_init_empty_map(void) {
  struct memory_map map = {
      .data = NULL,
      .size = 0,
  };

  struct pages pages = pages_init(&map);

  for (size_t i = 0; i < 32768; ++i) {
    TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[i]);
  }
}

void test_pages_init_marks_unavailable_memory(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
      memory_entry(PAGE_BITS, PAGE_SIZE, 0),
  };

  struct memory_map map = make_memory_map(entries, 2);
  struct pages pages = pages_init(&map);

  /*
   * First page is available, second page is unavailable.
   */
  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & 1u);
  TEST_ASSERT_NOT_EQUAL(0, pages.bitmap[0] & (1u << 1));
}

void test_pages_init_marks_available_memory(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 4, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & 0xFu);
}

void test_pages_init_handles_multiple_regions(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),

      memory_entry(PAGE_BITS * 2, PAGE_BITS * 2, 0),

      memory_entry(PAGE_BITS * 4, PAGE_BITS * 3,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 3);
  struct pages pages = pages_init(&map);

  /*
   * Pages 0-1: free
   * Pages 2-3: unavailable
   * Pages 4-6: free
   */
  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & (1u << 0));
  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & (1u << 1));

  TEST_ASSERT_NOT_EQUAL(0, pages.bitmap[0] & (1u << 2));
  TEST_ASSERT_NOT_EQUAL(0, pages.bitmap[0] & (1u << 3));

  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & (1u << 4));
  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & (1u << 5));
  TEST_ASSERT_EQUAL_UINT(0, pages.bitmap[0] & (1u << 6));
}

/*
 * Allocation
 */

void test_pages_allocate_returns_first_available_page(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(PAGE_BITS * 10, PAGE_BITS,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t address = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 10, address);
}

void test_pages_allocate_returns_different_pages(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 3, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t first = pages_allocate(&pages);
  uintptr_t second = pages_allocate(&pages);
  uintptr_t third = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, first);
  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS, second);
  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 2, third);

  TEST_ASSERT_NOT_EQUAL(first, second);
  TEST_ASSERT_NOT_EQUAL(first, third);
  TEST_ASSERT_NOT_EQUAL(second, third);
}

void test_pages_allocate_marks_page_allocated(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t address = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, address);

  /*
   * The page should now be marked allocated.
   */
  TEST_ASSERT_NOT_EQUAL(0, pages.bitmap[0] & 1u);
}

void test_pages_allocate_skips_unavailable_pages(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS, 0),
      memory_entry(PAGE_BITS, PAGE_BITS,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 2);
  struct pages pages = pages_init(&map);

  uintptr_t address = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS, address);
}

void test_pages_allocate_skips_allocated_pages(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t first = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, first);

  uintptr_t second = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS, second);
}

void test_pages_allocate_returns_failure_when_no_pages_available(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS, 0),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t address = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, address);
}

void test_pages_allocate_returns_failure_when_all_pages_are_allocated(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  TEST_ASSERT_EQUAL_UINT64(0, pages_allocate(&pages));
  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS, pages_allocate(&pages));
  TEST_ASSERT_EQUAL_UINT64(0, pages_allocate(&pages));
}

/*
 * Free
 */

void test_pages_free_makes_page_available_again(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t address = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, address);

  /*
   * Allocation should now fail.
   */
  TEST_ASSERT_EQUAL_UINT64(0, pages_allocate(&pages));

  pages_free(&pages, address);

  /*
   * The same page should now be available.
   */
  TEST_ASSERT_EQUAL_UINT64(0, pages_allocate(&pages));
}

void test_pages_free_allows_reuse_of_page(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, PAGE_BITS * 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t first = pages_allocate(&pages);
  uintptr_t second = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(0, first);
  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS, second);

  pages_free(&pages, first);

  uintptr_t third = pages_allocate(&pages);

  TEST_ASSERT_EQUAL_UINT64(first, third);
}

/*
 * Allocation across bitmap words
 */

void test_pages_allocate_crosses_bitmap_word_boundary(void) {
  /*
   * This makes all pages up to the first bit of the second
   * bitmap word available.
   */
  const size_t pages_per_word = sizeof(unsigned int) * 8u;

  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, (pages_per_word + 1) * PAGE_BITS,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  for (size_t i = 0; i < pages_per_word; ++i) {
    TEST_ASSERT_EQUAL_UINT64(i * PAGE_BITS, pages_allocate(&pages));
  }

  /*
   * This allocation must come from the next bitmap word.
   */
  TEST_ASSERT_EQUAL_UINT64(pages_per_word * PAGE_BITS,
                           pages_allocate(&pages));
}

void test_pages_free_crosses_bitmap_word_boundary(void) {
  const size_t pages_per_word = sizeof(unsigned int) * 8u;

  struct multiboot_memory_map_entry entries[] = {
      memory_entry(0, (pages_per_word + 1) * PAGE_BITS,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  uintptr_t addresses[pages_per_word + 1];

  for (size_t i = 0; i < pages_per_word + 1; ++i) {
    addresses[i] = pages_allocate(&pages);
  }

  pages_free(&pages, addresses[pages_per_word]);

  TEST_ASSERT_EQUAL_UINT64(pages_per_word * PAGE_BITS,
                           pages_allocate(&pages));
}

/*
 * Non-page-aligned memory regions
 */

void test_pages_init_handles_region_starting_on_page_boundary(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(PAGE_BITS * 4, PAGE_BITS * 2,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 4, pages_allocate(&pages));

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 5, pages_allocate(&pages));
}

void test_pages_init_handles_region_ending_on_page_boundary(void) {
  struct multiboot_memory_map_entry entries[] = {
      memory_entry(PAGE_BITS * 4, PAGE_BITS * 2,
                   MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE),
  };

  struct memory_map map = make_memory_map(entries, 1);
  struct pages pages = pages_init(&map);

  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 4, pages_allocate(&pages));
  TEST_ASSERT_EQUAL_UINT64(PAGE_BITS * 5, pages_allocate(&pages));

  TEST_ASSERT_EQUAL_UINT64(0, pages_allocate(&pages));
}

/*
 * Test runner
 */

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_pages_init_empty_map);
  RUN_TEST(test_pages_init_marks_unavailable_memory);
  RUN_TEST(test_pages_init_marks_available_memory);
  RUN_TEST(test_pages_init_handles_multiple_regions);

  RUN_TEST(test_pages_allocate_returns_first_available_page);
  RUN_TEST(test_pages_allocate_returns_different_pages);
  RUN_TEST(test_pages_allocate_marks_page_allocated);
  RUN_TEST(test_pages_allocate_skips_unavailable_pages);
  RUN_TEST(test_pages_allocate_skips_allocated_pages);
  RUN_TEST(test_pages_allocate_returns_failure_when_no_pages_available);
  RUN_TEST(test_pages_allocate_returns_failure_when_all_pages_are_allocated);

  RUN_TEST(test_pages_free_makes_page_available_again);
  RUN_TEST(test_pages_free_allows_reuse_of_page);

  RUN_TEST(test_pages_allocate_crosses_bitmap_word_boundary);
  RUN_TEST(test_pages_free_crosses_bitmap_word_boundary);

  RUN_TEST(test_pages_init_handles_region_starting_on_page_boundary);
  RUN_TEST(test_pages_init_handles_region_ending_on_page_boundary);

  return UNITY_END();
}
