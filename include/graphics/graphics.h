#pragma once

#include "bitmaps.h"
#include "boot/info.h"

#include <stddef.h>
#include <stdint.h>

struct graphics {
  uint32_t *framebuffer;
  uint32_t width;
  uint32_t height;
  size_t pixels_per_row;
};

struct graphics graphics_create(const struct framebuffer_info *info);
uint32_t *graphics_pixel(const struct graphics *graphics, size_t x, size_t y);
void graphics_set_screen(const struct graphics *graphics, uint32_t color);
void graphics_draw_bitmap(const struct graphics *graphics, size_t x, size_t y,
                          const struct bitmap *bitmap);
