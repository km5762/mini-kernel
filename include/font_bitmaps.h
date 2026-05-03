#pragma once

#include "bitmaps.h"

#include <stddef.h>

struct font_bitmaps {
  const struct bitmap *data;
  size_t size;
};

extern const struct font_bitmaps uni2_terminus16;
