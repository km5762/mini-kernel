#pragma once

#include "color.h"

#include <stddef.h>
#include <stdint.h>

struct bitmap {
  unsigned char *data;
  size_t width;
  size_t height;
  size_t stride;
  enum color_format color_format;
  uint32_t color;
};

void bitmaps_scale(struct bitmap *bitmap, size_t scale);
void bitmaps_set_color(struct bitmap *bitmap, uint32_t color);
