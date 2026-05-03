#include "graphics.h"

#include <stddef.h>
#include <stdint.h>

struct graphics graphics_create(const struct framebuffer_info *info) {
  struct graphics graphics = {(uint32_t *)(uintptr_t)info->address, info->width,
                              info->height, info->pitch / 4};
  return graphics;
}

uint32_t *graphics_pixel(const struct graphics *graphics, size_t x, size_t y) {
  return graphics->framebuffer + y * graphics->pixels_per_row + x;
}

void graphics_set_screen(const struct graphics *graphics, uint32_t color) {
  for (size_t y = 0; y < graphics->height; ++y) {
    uint32_t *row = graphics->framebuffer + y * graphics->pixels_per_row;
    for (size_t x = 0; x < graphics->width; ++x) {
      row[x] = color;
    }
  }
}

static void draw_bitmap_rgba32(const struct graphics *graphics, size_t x,
                               size_t y, const struct bitmap *bitmap) {
  for (size_t row = 0; row < bitmap->height; ++row) {
    for (size_t col = 0; col < bitmap->width; ++col) {
      uint32_t pixel =
          *(uint32_t *)(bitmap->data + row * bitmap->stride + col * 4);
      *graphics_pixel(graphics, x + col, y + row) = pixel;
    }
  }
}

static void draw_bitmap_monochrome(const struct graphics *graphics, size_t x,
                                   size_t y, const struct bitmap *bitmap) {
  for (size_t row = 0; row < bitmap->height; ++row) {
    for (size_t col = 0; col < bitmap->width; ++col) {
      size_t byte_index = row * bitmap->stride + (col / 8);
      size_t bit_index = 7 - (col % 8);
      unsigned char byte = bitmap->data[byte_index];
      if (byte & (1 << bit_index)) {
        *graphics_pixel(graphics, x + col, y + row) = bitmap->color;
      }
    }
  }
}

void graphics_draw_bitmap(const struct graphics *graphics, size_t x, size_t y,
                          const struct bitmap *bitmap) {
  switch (bitmap->color_format) {
  case COLOR_FORMAT_RGBA32:
    draw_bitmap_rgba32(graphics, x, y, bitmap);
    break;
  case COLOR_FORMAT_MONO:
    draw_bitmap_monochrome(graphics, x, y, bitmap);
    break;
  }
}
