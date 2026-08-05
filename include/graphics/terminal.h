#pragma once

#include "font_bitmaps.h"
#include "graphics.h"

#include <stddef.h>
#include <stdint.h>

struct terminal {
  size_t x;
  size_t y;
  size_t line_height;
  struct graphics graphics;
  struct font_bitmaps font_bitmaps;
};

struct terminal terminal_create(const struct graphics *graphics,
                                const struct font_bitmaps *font_bitmaps);
void terminal_print(struct terminal *terminal, const char *line);
