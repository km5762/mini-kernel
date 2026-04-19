#pragma once

#include "bitmaps.h"

#include <stddef.h>
#include <stdint.h>

struct framebuffer_info {
  uint64_t address;
  uint32_t pitch;
  uint32_t width;
  uint32_t height;
  uint8_t bits_per_pixel;
};

void graphics_initialize(const struct framebuffer_info *info);
uint32_t *graphics_pixel(size_t x, size_t y);
void graphics_set_screen(uint32_t color);
void graphics_draw_bitmap(size_t x, size_t y, const struct bitmap *bitmap);
