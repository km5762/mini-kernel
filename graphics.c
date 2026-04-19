#include "graphics.h"
#include "font8x8_basic.h"

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

void graphics_draw_character(size_t x, size_t y, char character,
                             uint32_t color) {
  char *bitmap = font8x8_basic[(size_t)character];

  for (size_t row = 0; row < 8; ++row) {
    for (size_t col = 0; col < 8; ++col) {
      if ((bitmap[row] >> col) & 1) {
        *graphics_pixel(x + col, y + row) = color;
      }
    }
  }
}
