#include "memory.h"
#include "array_utilites.h"
#include "multiboot.h"

#include "test/panic.h"
#include "unity.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

static unsigned char buffer[512] = {0};

static uintptr_t address(size_t offset) { return (uintptr_t)buffer + offset; }

void setUp() {}

void tearDown() {}

struct test_context {
  struct memory_map memory_map;
  int panic_counter;
  struct panic_handler panic_handler;
  struct memory memory;
};

static void init_context(struct test_context *ctx,
                         const struct multiboot_memory_map_entry *entries,
                         size_t count) {

  *ctx = (struct test_context){0};

  ctx->memory_map = (struct memory_map){
      .data = entries,
      .size = count,
  };

  ctx->panic_handler = panic_create_handler_test(&ctx->panic_counter);

  ctx->memory = memory_create(&ctx->memory_map, &ctx->panic_handler);
}

#define INIT_CONTEXT(ctx, entries)                                             \
  init_context(&(ctx), (entries), ARRAY_SIZE(entries))

static void assert_no_panic(const struct test_context *ctx) {
  TEST_ASSERT_EQUAL(0, ctx->panic_counter);
}

static void assert_panicked(const struct test_context *ctx) {
  TEST_ASSERT_EQUAL(1, ctx->panic_counter);
}

static void assert_valid_allocation(void *allocation,
                                    const struct memory_map *memory_map) {

  TEST_ASSERT_NOT_NULL(allocation);

  for (size_t i = 0; i < memory_map->size; ++i) {
    const struct multiboot_memory_map_entry *entry = &memory_map->data[i];

    if (entry->type == MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE &&
        (uintptr_t)allocation < (entry->address + entry->size)) {
      return;
    }
  }

  TEST_FAIL();
}

static void empty_memory_map() {
  const struct multiboot_memory_map_entry entries[] = {};

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 2);

  TEST_ASSERT_NULL(allocation);
  assert_panicked(&ctx);
}

static void not_enough_space_for_meta() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
      {address(2), 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 2);

  TEST_ASSERT_NULL(allocation);
  assert_panicked(&ctx);
}

static void allocate_0() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), sizeof(struct memory_free_block),
       MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 0);

  TEST_ASSERT_EQUAL(address(0) + sizeof(struct memory_free_block),
                    (uintptr_t)allocation);

  assert_no_panic(&ctx);
}

static void allocate_perfect_fit() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), sizeof(struct memory_free_block) + 1,
       MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 1);

  assert_valid_allocation(allocation, &ctx.memory_map);
  assert_no_panic(&ctx);
}

static void allocate_perfect_fit_middle() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), 1, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
      {address(0), sizeof(struct memory_free_block) + 1,
       MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
      {address(0), 1, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 1);

  assert_valid_allocation(allocation, &ctx.memory_map);
  assert_no_panic(&ctx);
}

static void allocate_with_1_remaining_byte() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), sizeof(struct memory_free_block) + 2,
       MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 1);

  assert_valid_allocation(allocation, &ctx.memory_map);
  assert_no_panic(&ctx);
}

static void allocate_with_space_remaining() {
  const struct multiboot_memory_map_entry entries[] = {
      {address(0), sizeof(struct memory_free_block) * 2 + 32,
       MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  };

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  void *allocation = memory_allocate(&ctx.memory, 1);

  assert_valid_allocation(allocation, &ctx.memory_map);
  assert_no_panic(&ctx);

  allocation = memory_allocate(&ctx.memory, 1);

  assert_valid_allocation(allocation, &ctx.memory_map);
  assert_no_panic(&ctx);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(empty_memory_map);
  RUN_TEST(not_enough_space_for_meta);
  RUN_TEST(allocate_0);
  RUN_TEST(allocate_perfect_fit);
  RUN_TEST(allocate_perfect_fit_middle);
  RUN_TEST(allocate_with_1_remaining_byte);
  RUN_TEST(allocate_with_space_remaining);

  return UNITY_END();
}
