#pragma once

#include "panic.h"
#include "uacpi/acpi.h"

#include <stdint.h>

void interrupts_init(struct panic_handler panic_handler,
                     const struct acpi_madt *madt);
struct interrupt_frame;
void interrupt_dispatch(struct interrupt_frame *context);

enum interrupt_trigger {
  INTERRUPT_TRIGGER_EDGE,
  INTERRUPT_TRIGGER_LEVEL,
};

enum interrupt_polarity {
  INTERRUPT_ACTIVE_HIGH,
  INTERRUPT_ACTIVE_LOW,
};

struct interrupt_config {
  enum interrupt_trigger trigger;
  enum interrupt_polarity polarity;
};

struct interrupt_handler;
void interrupt_handler_enable(struct interrupt_handler *handler);
void interrupt_handler_disable(struct interrupt_handler *handler);
