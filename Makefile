ARCH ?= x86_64
TARGET_CC := x86_64-elf-gcc
HOST_CC   := gcc

BUILD ?= release

SRC_DIR    := src
BUILD_DIR  := build
BUILD_ROOT := $(BUILD_DIR)/$(BUILD)
ISO_DIR    := $(BUILD_ROOT)/iso

TARGET_DIR := $(BUILD_ROOT)/target
HOST_DIR   := $(BUILD_ROOT)/host

TEST_DIR      := test
TEST_UTIL_DIR := $(TEST_DIR)/utilities

UNITY_DIR  := unity

ARCH_DIR := arch/$(ARCH)

COMMON_C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
HOST_C_SRCS := $(filter-out $(SRC_DIR)/kernel.c,$(COMMON_C_SRCS))
ARCH_C_SRCS := $(shell find $(ARCH_DIR) -name '*.c')
ARCH_ASM_SRCS := $(shell find $(ARCH_DIR) -name '*.S')

TARGET_OBJS := \
	$(COMMON_C_SRCS:$(SRC_DIR)/%.c=$(TARGET_DIR)/%.o) \
	$(ARCH_C_SRCS:$(ARCH_DIR)/%.c=$(TARGET_DIR)/arch/$(ARCH)/%.o) \
	$(ARCH_ASM_SRCS:$(ARCH_DIR)/%.S=$(TARGET_DIR)/arch/$(ARCH)/%.o)

HOST_OBJS := \
	$(HOST_C_SRCS:$(SRC_DIR)/%.c=$(HOST_DIR)/%.o)

TEST_SRCS := \
    $(filter-out $(TEST_UTIL_DIR)/%,$(shell find $(TEST_DIR) -name '*.c'))

TEST_BINS := \
	$(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_ROOT)/test/%)

TEST_UTIL_SRCS := \
    $(shell find $(TEST_UTIL_DIR) -name '*.c')

TEST_UTIL_OBJS := \
	$(TEST_UTIL_SRCS:$(TEST_UTIL_DIR)/%.c=$(BUILD_ROOT)/test/utilities/%.o)

UNITY_OBJ := $(BUILD_ROOT)/test/unity.o

COMMON_CFLAGS := -std=gnu2x -Wall -Wextra -I$(ARCH_DIR) -Iinclude
LDFLAGS := -ffreestanding -nostdlib
ifeq ($(BUILD),debug)
	COMMON_CFLAGS += -O0 -g
	LDFLAGS       += -g

	ELF_NAME := kernel-debug.elf
	ISO_NAME := kernel-debug.iso

	QEMUFLAGS := -s -S
else
	COMMON_CFLAGS += -O2

	ELF_NAME := kernel.elf
	ISO_NAME := kernel.iso
endif
TARGET_CFLAGS := $(COMMON_CFLAGS) -ffreestanding -mcmodel=large -mno-red-zone -MMD -MP
HOST_CFLAGS := \
	$(COMMON_CFLAGS) \
	-I$(UNITY_DIR) \
	-MMD -MP -fno-builtin

ELF := $(BUILD_ROOT)/$(ELF_NAME)
ISO := $(BUILD_ROOT)/$(ISO_NAME)
LINKER_SCRIPT := $(BUILD_ROOT)/linker.ld

.PHONY: all iso run gdb test clean

all: iso

$(ELF): $(TARGET_OBJS) $(LINKER_SCRIPT)
	$(TARGET_CC) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(TARGET_OBJS) -lgcc

$(LINKER_SCRIPT): linker.ld $(ARCH_DIR)/asm/paging.h
	@mkdir -p $(@D)
	$(TARGET_CC) -E -P -x c $(COMMON_CFLAGS) -DLD_SCRIPT $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(TARGET_DIR)/arch/$(ARCH)/%.o: $(ARCH_DIR)/%.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(TARGET_DIR)/arch/$(ARCH)/%.o: $(ARCH_DIR)/%.S
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(HOST_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(UNITY_OBJ): $(UNITY_DIR)/unity.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(BUILD_ROOT)/test/utilities/%.o: $(TEST_UTIL_DIR)/%.c
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(BUILD_ROOT)/test/%: \
	$(TEST_DIR)/%.c \
	$(HOST_OBJS) \
	$(TEST_UTIL_OBJS) \
	$(UNITY_OBJ)

	@mkdir -p $(@D)

	$(HOST_CC) \
		$(HOST_CFLAGS) \
		$^ \
		-o $@

test: $(TEST_BINS)
	@for t in $^; do ./$$t; done

iso: $(ISO)

$(ISO): $(ELF) grub.cfg
	@mkdir -p $(ISO_DIR)/boot/grub

	cp $< $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg

	grub-mkrescue -o $@ $(ISO_DIR)

run: iso
	qemu-system-x86_64 -cdrom $(ISO) $(QEMUFLAGS)

clean:
	$(RM) -r $(BUILD_DIR)/*

DEPS := \
	$(TARGET_OBJS:.o=.d) \
	$(HOST_OBJS:.o=.d) \
	$(TEST_UTIL_OBJS:.o=.d) \
	$(UNITY_OBJ:.o=.d)

-include $(DEPS)
