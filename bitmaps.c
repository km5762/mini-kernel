#include "bitmaps.h"

void bitmaps_scale(struct bitmap *bitmap, size_t scale) {
  size_t scaled_pixel_size = scale * scale;
  bitmap->width = bitmap->width * scaled_pixel_size;
  bitmap->height = bitmap->height * scaled_pixel_size;
}
void bitmaps_set_color(struct bitmap *bitmap, uint32_t color) {
  bitmap->color_format = COLOR_FORMAT_MONO;
  bitmap->color = color;
}
