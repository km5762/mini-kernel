#pragma once

#include "terminal.h"

struct panic_context {
  const char *file;
  const int line;
  const char *function;
};

struct panic_handler {
  void (*handle)(const char *message, const struct panic_context *context,
                 void *arguments);
  void *data;
};

struct panic_handler create_panic_handler_terminal(struct terminal *terminal);

#define PANIC(message, handler)                                                \
  (panic(message, handler, __FILE__, __LINE__, __func__))

void panic(const char *message, struct panic_handler *handler, const char *file,
           int line, const char *function);
