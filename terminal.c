#include "terminal.h"
#include "graphics.h"

struct terminal {
  size_t x;
  size_t y;
  uint32_t color;
};

static struct terminal terminal;

void terminal_init(uint32_t color) { terminal.color = color; }

void terminal_print_line(const char *line) {
  for (size_t i = 0; line[i] != '\0'; ++i) {
    graphics_draw_character(terminal.x, terminal.y, line[i], terminal.color);
    terminal.x += 8;
  }
  terminal.y += 8;
}
