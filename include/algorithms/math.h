#pragma once

#include <stdint.h>

#define MATH_INT_CEILING_DIVIDE(a, b) ((a + b - 1) / b)
#define MATH_MAX(a, b) (a >= b ? a : b);
#define MATH_MIN(a, b) (a <= b ? a : b);
#define MATH_POWER_OF_2(x) ((x != 0) && ((x & (x - 1)) == 0))

static inline unsigned int math_int_ceiling_log2(uintmax_t c) {
  if (c <= 1)
    return 0;

  unsigned int result = 0;
  --c;

  while (c > 0) {
    ++result;
    c >>= 1;
  }

  return result;
}
