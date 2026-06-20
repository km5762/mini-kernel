#include "memory.h"
#include "array_utilites.h"
#include "multiboot.h"

#include "test/panic.h"
#include "unity.h"

#include <inttypes.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define BLOCK_COUNT 16
#define GAP_SIZE 1024

static unsigned char buffer[BLOCK_SIZE * BLOCK_COUNT + GAP_SIZE * BLOCK_COUNT] =
    {0};

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

struct live_allocation {
  void *ptr;
  size_t size;
};

#define MAX_LIVE_ALLOCATIONS 4096

static const size_t edge_sizes[] = {1,   2,    3,    7,    8,    15,   16,  31,
                                    32,  63,   64,   127,  128,  255,  256, 511,
                                    512, 1023, 1024, 2048, 4095, 4096, 4097};

static size_t random_allocation_size(void) {
  int r = rand() % 100;

  if (r < 70) {
    return edge_sizes[rand() % (sizeof(edge_sizes) / sizeof(edge_sizes[0]))];
  }

  return 1 + (rand() % 8192);
}

static void assert_no_live_overlap(const struct live_allocation *live,
                                   size_t count) {
  for (size_t i = 0; i < count; ++i) {
    uintptr_t a0 = (uintptr_t)live[i].ptr;
    uintptr_t a1 = a0 + live[i].size;

    for (size_t j = i + 1; j < count; ++j) {
      uintptr_t b0 = (uintptr_t)live[j].ptr;
      uintptr_t b1 = b0 + live[j].size;

      TEST_ASSERT_FALSE(a0 < b1 && b0 < a1);
    }
  }
}

#define FUZZ_ITERATIONS 100000000

static void fuzz() {
  struct multiboot_memory_map_entry entries[BLOCK_COUNT] = {0};
  for (size_t i = 0; i < BLOCK_COUNT; ++i) {
    entries[i] = (struct multiboot_memory_map_entry){
        .size = BLOCK_SIZE,
        .address = address(i * (BLOCK_SIZE + GAP_SIZE)),
        .type = MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE,
        .reserved = 0,
    };
  }

  struct test_context ctx;
  INIT_CONTEXT(ctx, entries);

  const char *seed_env = getenv("FUZZ_SEED");
  unsigned int seed = seed_env ? (unsigned int)strtoul(seed_env, NULL, 10)
                               : (unsigned int)time(NULL);
  srand(seed);
  printf("Seed: %u\n", seed);

  struct live_allocation live[MAX_LIVE_ALLOCATIONS] = {0};
  size_t live_count = 0;
  size_t peak_live_count = 0;
  for (size_t i = 0; i < FUZZ_ITERATIONS; ++i) {
    bool do_alloc = (live_count == 0) || (rand() % 100 < 60);

    if (do_alloc) {
      if (live_count == MAX_LIVE_ALLOCATIONS) {
        do_alloc = false;
      } else {
        size_t size = random_allocation_size();
        void *p = memory_allocate(&ctx.memory, size);
        assert_no_panic(&ctx);

        if (p != nullptr) {
          live[live_count++] = (struct live_allocation){.ptr = p, .size = size};
          if (live_count > peak_live_count) {
            peak_live_count = live_count;
          }

          for (size_t j = 0; j + 1 < live_count; ++j) {
            TEST_ASSERT_FALSE(live[j].ptr == p);
          }

          assert_no_live_overlap(live, live_count);
        }
      }
    }

    if (!do_alloc) {
      size_t idx = rand() % live_count;
      memory_free(&ctx.memory, live[idx].ptr);
      assert_no_panic(&ctx);

      live[idx] = live[live_count - 1];
      live[live_count - 1] = (struct live_allocation){0};
      --live_count;
    }
  }

  printf("Current live allocations: %zu\n", live_count);
  printf("Peak live allocations: %zu\n", peak_live_count);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(fuzz);

  return UNITY_END();
}
