#include "memory.h"
#include "array_utils.h"
#include "multiboot.h"

#include "unity.h"

void setUp() {}

void tearDown() {}

void test_allocate_entire_node() {
  // const struct multiboot_memory_map_entry entries[] = {
  //     {0, 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0},
  //     {2, 2, MEMORY_MAP_TAG_ENTRY_TYPE_AVAILABLE, 0}};
  // const struct memory_map memory_map = {&entries[0], ARRAY_SIZE(entries)};
  // struct memory memory = memory_create(&memory_map, nullptr);
  // memory_allocate(&memory, 2);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allocate_entire_node);

  return UNITY_END();
}
