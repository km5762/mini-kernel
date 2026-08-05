#pragma once

#include "graphics/terminal.h"

struct panic_context {
  const char *file;
  const int line;
  const char *function;
};

struct panic_handler {
  void (*handle)(const char *message, const struct panic_context *context,
                 void *data);
  void *data;
};

struct panic_handler panic_create_handler_terminal(struct terminal *terminal);

void panic(const char *message, struct panic_handler *handler, const char *file,
           int line, const char *function);

#define PANIC(message, handler)                                                \
  (panic(message, handler, __FILE__, __LINE__, __func__))

#define ASSERT(condition, handler)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      PANIC("Assertion failed: " #condition, handler);                         \
    }                                                                          \
  } while (0)
