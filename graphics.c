#include "graphics.h"

#include <stddef.h>
#include <stdint.h>

static struct framebuffer_info framebuffer_info;
static size_t pixels_per_row;
static uint32_t *framebuffer;

void graphics_initialize(const struct framebuffer_info *info) {
  framebuffer_info = *info;
  framebuffer = (uint32_t *)info->address;
  pixels_per_row = framebuffer_info.pitch / 4;
}

uint32_t *graphics_pixel(size_t x, size_t y) {
  return framebuffer + y * pixels_per_row + x;
}

void graphics_set_screen(uint32_t color) {
  for (size_t y = 0; y < framebuffer_info.height; ++y) {
    uint32_t *row = framebuffer + y * pixels_per_row;
    for (size_t x = 0; x < framebuffer_info.width; ++x) {
      row[x] = color;
    }
  }
}

static void draw_bitmap_rgba32(size_t x, size_t y,
                               const struct bitmap *bitmap) {
  for (size_t row = 0; row < bitmap->height; ++row) {
    for (size_t col = 0; col < bitmap->width; ++col) {
      uint32_t pixel =
          *(uint32_t *)(bitmap->data + row * bitmap->stride + col * 4);
      *graphics_pixel(x + col, y + row) = pixel;
    }
  }
}

static void draw_bitmap_monochrome(size_t x, size_t y,
                                   const struct bitmap *bitmap) {
  for (size_t row = 0; row < bitmap->height; ++row) {
    for (size_t col = 0; col < bitmap->width; ++col) {
      size_t byte_index = row * bitmap->stride + (col / 8);
      size_t bit_index = 7 - (col % 8);
      unsigned char byte = bitmap->data[byte_index];
      if (byte & (1 << bit_index)) {
        *graphics_pixel(x + col, y + row) = bitmap->color;
      }
    }
  }
}

void graphics_draw_bitmap(size_t x, size_t y, const struct bitmap *bitmap) {
  switch (bitmap->color_format) {
  case COLOR_FORMAT_RGBA32:
    draw_bitmap_rgba32(x, y, bitmap);
    break;
  case COLOR_FORMAT_MONO:
    draw_bitmap_monochrome(x, y, bitmap);
    break;
  }
}
