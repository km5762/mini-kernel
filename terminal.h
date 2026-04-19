#pragma once

#include "font_bitmaps.h"

#include <stddef.h>
#include <stdint.h>

void terminal_set_font_bitmaps(const struct font_bitmaps *bitmaps);
void terminal_print_line(const char *line);
