#pragma once

#include "graphics/terminal.h"
#include <stdarg.h>

enum log_level {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
};

struct log_context {
  const char *file;
  int line;
  const char *function;
};

struct log_handler {
  void (*handle)(void *data, enum log_level level,
                 const struct log_context *ctx, const char *fmt, va_list args);
  void *data;
};

#define LOG_INFO(handler, ...)                                                 \
  log_write(handler, LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(handler, ...)                                              \
  log_write(handler, LOG_LEVEL_WARNING, __FILE__, __LINE__, __func__,          \
            __VA_ARGS__)
#define LOG_ERROR(handler, ...)                                                \
  log_write(handler, LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

void log_write(struct log_handler handler, enum log_level level,
               const char *file, int line, const char *function,
               const char *fmt, ...);

struct log_handler log_create_handler_terminal(struct terminal *terminal);
