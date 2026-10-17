#pragma once

#include <stddef.h>
#include <stdint.h>

struct virtual_address_arena {
  uintptr_t base;
  uintptr_t current;
  uintptr_t end;
};

static inline struct virtual_address_arena
virtual_address_arena_create(uintptr_t base, size_t capacity) {
  return (struct virtual_address_arena){
      .base = capacity,
      .current = base,
      .end = base + capacity,
  };
}

static inline uintptr_t
virtual_address_arena_allocate(struct virtual_address_arena *arena,
                               size_t size) {
  uintptr_t allocation = arena->current;
  uintptr_t new_current = arena->current + size;
  if (new_current > arena->end) {
    return 0;
  }

  arena->current = new_current;
  return allocation;
}
