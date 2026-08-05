TARGET_CC := i686-elf-gcc
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

C_SRCS   := $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS := $(shell find $(SRC_DIR) -name '*.s')

TARGET_OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(TARGET_DIR)/%.o) \
	$(ASM_SRCS:$(SRC_DIR)/%.s=$(TARGET_DIR)/%.o)

HOST_OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(HOST_DIR)/%.o)

TEST_SRCS := \
    $(filter-out $(TEST_UTIL_DIR)/%,$(shell find $(TEST_DIR) -name '*.c'))

TEST_BINS := \
	$(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_ROOT)/test/%)

TEST_UTIL_SRCS := \
    $(shell find $(TEST_UTIL_DIR) -name '*.c')

TEST_UTIL_OBJS := \
	$(TEST_UTIL_SRCS:$(TEST_UTIL_DIR)/%.c=$(BUILD_ROOT)/test/utilities/%.o)

UNITY_OBJ := $(BUILD_ROOT)/test/unity.o

COMMON_CFLAGS := -std=gnu2x -Wall -Wextra -Iinclude
LDFLAGS := -T linker.ld -ffreestanding -nostdlib
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
TARGET_CFLAGS := $(COMMON_CFLAGS) -ffreestanding
HOST_CFLAGS := \
	$(COMMON_CFLAGS) \
	-I$(UNITY_DIR) \

ELF := $(BUILD_ROOT)/$(ELF_NAME)
ISO := $(BUILD_ROOT)/$(ISO_NAME)

.PHONY: all iso run gdb test clean

all: iso

$(ELF): $(TARGET_OBJS)
	$(TARGET_CC) $(LDFLAGS) -o $@ $^ -lgcc

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(TARGET_DIR)/%.o: $(SRC_DIR)/%.s
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

$(ISO): $(ELF)
	@mkdir -p $(ISO_DIR)/boot/grub

	cp $< $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg

	grub-mkrescue -o $@ $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO) $(QEMUFLAGS)

clean:
	$(RM) -r $(BUILD_ROOT)
