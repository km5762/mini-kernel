#pragma once

#include "terminal.h"

struct panic_sink {
  void (*write)(const char *message, void *context);
  void *context;
};

struct panic_sink create_panic_sink_terminal(struct terminal *terminal);

#define PANIC(message, sink)                                                   \
  (panic(message, sink, __FILE__, __LINE__, __func__))

[[noreturn]]
void panic(const char *message, struct panic_sink *sink, const char *file,
           int line, const char *function);
