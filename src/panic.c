#include "panic.h"

static void panic_sink_terminal_write(const char *message, void *context) {
  struct terminal *terminal = context;
  terminal_print(terminal, message);
}

struct panic_sink create_panic_sink_terminal(struct terminal *terminal) {
  struct panic_sink sink = {panic_sink_terminal_write, terminal};
  return sink;
}

static void sink_write(struct panic_sink *sink, const char *msg) {
  sink->write(msg, sink->context);
}

void panic(const char *message, struct panic_sink *sink, const char *file,
           int line, const char *function) {

  sink_write(sink, "\n================ KERNEL PANIC ================\n");

  sink_write(sink, "Message: ");
  sink_write(sink, message);
  sink_write(sink, "\n");

  sink_write(sink, "Location: ");
  sink_write(sink, file);
  sink_write(sink, ":");

  sink_write(sink, " (");
  sink_write(sink, function);
  sink_write(sink, ")\n");

  sink_write(sink, "==============================================\n");

  for (;;) {
    __asm__ volatile("hlt");
  }
}
