#pragma once

#include "debug/location.h"
#include "graphics/terminal.h"
#include <stdarg.h>

enum log_level {
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
};

struct log_handler {
  void (*handle)(void *data, enum log_level level,
                 const struct debug_location *ctx, const char *fmt,
                 va_list args);
  void *data;
};

#define LOG_INFO(handler, ...)                                                 \
  log_write(handler, LOG_LEVEL_INFO,                                           \
            &(struct debug_location){__FILE__, __LINE__, __func__},            \
            __VA_ARGS__)
#define LOG_WARNING(handler, ...)                                              \
  log_write(handler, LOG_LEVEL_WARNING,                                        \
            &(struct debug_location){__FILE__, __LINE__, __func__},            \
            __VA_ARGS__)
#define LOG_ERROR(handler, ...)                                                \
  log_write(handler, LOG_LEVEL_ERROR,                                          \
            &(struct debug_location){__FILE__, __LINE__, __func__},            \
            __VA_ARGS__)

void log_write(struct log_handler handler, enum log_level level,
               const struct debug_location *location, const char *fmt, ...);
void log_write_va(struct log_handler handler, enum log_level level,
                  const struct debug_location *location, const char *fmt,
                  va_list args);

struct log_handler log_create_handler_terminal(struct terminal *terminal);
