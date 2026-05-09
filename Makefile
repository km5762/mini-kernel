TARGET_CC := i686-elf-gcc
HOST_CC   := gcc

BUILD ?= release

SRC_DIR    := src
BUILD_DIR  := build
ISO_DIR    := $(BUILD_DIR)/iso

TARGET_DIR := $(BUILD_DIR)/target
HOST_DIR   := $(BUILD_DIR)/host
TEST_DIR   := test
UNITY_DIR  := unity

C_SRCS   := $(wildcard $(SRC_DIR)/*.c)
ASM_SRCS := $(wildcard $(SRC_DIR)/*.s)

TARGET_OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(TARGET_DIR)/%.o) \
	$(ASM_SRCS:$(SRC_DIR)/%.s=$(TARGET_DIR)/%.o)

HOST_OBJS := \
	$(C_SRCS:$(SRC_DIR)/%.c=$(HOST_DIR)/%.o)

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/test/%)

UNITY_OBJ := $(BUILD_DIR)/test/unity.o

COMMON_CFLAGS := -std=gnu2x -Wall -Wextra -Iinclude

TARGET_CFLAGS := $(COMMON_CFLAGS) -ffreestanding
HOST_CFLAGS   := $(COMMON_CFLAGS)

LDFLAGS := -T linker.ld -ffreestanding -nostdlib

ifeq ($(BUILD),debug)
	TARGET_CFLAGS += -O0 -g
	LDFLAGS       += -g
	ELF_NAME := kernel-debug.elf
	ISO_NAME := kernel-debug.iso
	QEMUFLAGS := -s -S
else
	TARGET_CFLAGS += -O2
	ELF_NAME := kernel.elf
	ISO_NAME := kernel.iso
endif

ELF := $(BUILD_DIR)/$(ELF_NAME)
ISO := $(BUILD_DIR)/$(ISO_NAME)

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

$(BUILD_DIR)/test/%: $(TEST_DIR)/%.c $(HOST_OBJS) $(UNITY_OBJ)
	@mkdir -p $(@D)
	$(HOST_CC) $(HOST_CFLAGS) $^ -o $@ -Iunity

iso: $(ISO)

$(ISO): $(ELF)
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $< $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO) $(QEMUFLAGS)

test: $(TEST_BINS)
	@for t in $^; do ./$$t; done

clean:
	$(RM) -r $(BUILD_DIR)
