#include "panic.h"
#include "test/panic.h"

static void handle(const char *, const struct panic_context *, void *data) {
  int *counter = data;
  ++*counter;
}

struct panic_handler panic_create_handler_test(int *counter) {
  struct panic_handler handler = {handle, counter};
  return handler;
}
