#pragma once

#include "terminal.h"
#include "bitmaps.h"
#include "graphics.h"

struct terminal {
  size_t x;
  size_t y;
  const struct font_bitmaps *font_bitmaps;
};

static struct terminal terminal;

void terminal_set_font_bitmaps(const struct font_bitmaps *bitmaps) {
  terminal.font_bitmaps = bitmaps;
}

void terminal_print_line(const char *line) {
  size_t line_height = 0;
  for (size_t i = 0; line[i] != '\0'; ++i) {
    const struct bitmap *bitmap =
        &terminal.font_bitmaps->data[(unsigned char)line[i]];
    graphics_draw_bitmap(terminal.x, terminal.y, bitmap);
    terminal.x += bitmap->width;
    line_height = bitmap->height;
  }
  terminal.y += line_height;
}
