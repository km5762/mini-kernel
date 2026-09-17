#pragma once

#include <stddef.h>

struct memory_arena {
  unsigned char *data;
  size_t capacity;
  unsigned char *current;
  unsigned char *end;
};

struct memory_arena memory_arena_create(unsigned char *data, size_t capacity) {
  return (struct memory_arena){
      .data = data,
      .capacity = capacity,
      .current = data,
      .end = data + capacity,
  };
}

void *memory_arena_allocate(struct memory_arena *arena, size_t size) {
  unsigned char *new_current = arena->current + size;
  if (new_current > arena->end) {
    return nullptr;
  }

  arena->current = new_current;
  return new_current;
}
