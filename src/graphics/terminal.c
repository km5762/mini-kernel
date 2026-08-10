#include "graphics/terminal.h"
#include "graphics/bitmaps.h"
#include "graphics/font_bitmaps.h"
#include "graphics/graphics.h"

struct terminal terminal_create(const struct graphics *graphics,
                                const struct font_bitmaps *font_bitmaps) {
  struct terminal terminal = {0, 0, font_bitmaps[0].data->height, *graphics,
                              *font_bitmaps};
  return terminal;
}

static void draw_glyph(struct terminal *terminal, char glyph) {
  if (glyph == '\n') {
    terminal->y += terminal->line_height;
    terminal->x = 0;
    return;
  }
  const struct bitmap *bitmap =
      &terminal->font_bitmaps.data[(unsigned char)glyph];
  graphics_draw_bitmap(&terminal->graphics, terminal->x, terminal->y, bitmap);
  terminal->x += bitmap->width;
}

void terminal_print(struct terminal *terminal, const char *line) {
  for (size_t i = 0; line[i] != '\0'; ++i) {
    draw_glyph(terminal, line[i]);
  }
}
