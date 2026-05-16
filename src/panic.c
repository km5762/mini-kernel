#include "panic.h"

static void panic_handler_terminal(const char *message,
                                   const struct panic_context *context,
                                   void *data) {
  struct terminal *terminal = data;

  terminal_print(terminal,
                 "\n================ KERNEL PANIC ================\n");

  terminal_print(terminal, "Message: ");
  terminal_print(terminal, message);
  terminal_print(terminal, "\n");

  terminal_print(terminal, "Location: ");
  terminal_print(terminal, context->file);
  terminal_print(terminal, ":");

  terminal_print(terminal, " (");
  terminal_print(terminal, context->function);
  terminal_print(terminal, ")\n");

  terminal_print(terminal, "==============================================\n");

  for (;;) {
    __asm__ volatile("hlt");
  }
}

struct panic_handler panic_create_handler_terminal(struct terminal *terminal) {
  struct panic_handler handler = {panic_handler_terminal, terminal};
  return handler;
}

void panic(const char *message, struct panic_handler *handler, const char *file,
           int line, const char *function) {
  const struct panic_context context = {file, line, function};
  handler->handle(message, &context, handler->data);
}
