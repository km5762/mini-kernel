#pragma once

#include "asm/interrupts.h"
#include "debug/logging.h"
#include "graphics/graphics.h"
#include "graphics/terminal.h"
#include "memory/memory_pool.h"
#include "memory/virtual_address_arena.h"

struct kernel {
  struct graphics graphics;
  struct terminal terminal;
  struct memory_pool memory_pool;
  struct page_pool page_pool;
  struct panic_handler panic_handler;
  struct log_handler log_handler;
  // struct interrupt_dispatcher interrupt_dispatcher;
  struct virtual_address_arena virtual_address_arena;
  uintptr_t direct_map_physical_end;
};
