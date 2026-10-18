#include "debug/logging.h"
#include "graphics/terminal.h"
#include "printf/printf.h"
#include <stdarg.h>

void log_write(struct log_handler handler, enum log_level level,
               const char *file, int line, const char *function,
               const char *fmt, ...) {
  struct log_context ctx = {
      .file = file,
      .line = line,
      .function = function,
  };

  va_list args;
  va_start(args, fmt);
  handler.handle(handler.data, level, &ctx, fmt, args);
  va_end(args);
}

static void terminal_out(char c, void *arg) {
  terminal_put((struct terminal *)arg, c);
}

static const char *level_name(enum log_level level) {
  switch (level) {
  case LOG_LEVEL_INFO:
    return "INFO";
  case LOG_LEVEL_WARNING:
    return "WARN";
  case LOG_LEVEL_ERROR:
    return "ERROR";
  }
  return "?";
}

static void log_handler_terminal(void *data, enum log_level level,
                                 const struct log_context *ctx, const char *fmt,
                                 va_list args) {
  struct terminal *terminal = data;

  fctprintf(terminal_out, terminal, "[%s] %s:%d %s: ", level_name(level),
            ctx->file, ctx->line, ctx->function);

  vfctprintf(terminal_out, terminal, fmt, args);

  terminal_out('\n', terminal);
}

struct log_handler log_create_handler_terminal(struct terminal *terminal) {
  return (struct log_handler){
      .handle = log_handler_terminal,
      .data = terminal,
  };
}
