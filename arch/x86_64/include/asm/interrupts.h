#pragma once

#include <stdint.h>

struct interrupt_handler *interrupts_register_gsi(uint32_t gsi);
