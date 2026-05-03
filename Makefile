CC      = i686-elf-gcc
LD      = i686-elf-gcc
BUILD  ?= release
CFLAGS  = -std=gnu23 -ffreestanding -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -ffreestanding -nostdlib
ISO_DIR  = isodir
BUILD_DIR = build
SRC_DIR = src
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)

OBJS = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) \
       $(ASM_SOURCES:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)

ifeq ($(BUILD),debug)
CFLAGS   += -O0 -g
LDFLAGS  += -g
ISO_NAME  = kernel-debug.iso
ELF_NAME  = kernel-debug.elf
else
CFLAGS   += -O2
ISO_NAME  = kernel.iso
ELF_NAME  = kernel.elf
endif

QEMUFLAGS ?=
ifeq ($(BUILD),debug)
QEMUFLAGS += -s -S
endif

.PHONY: all iso run gdb clean

all: iso

$(ELF_NAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

iso: $(ELF_NAME)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(ELF_NAME) $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_NAME) $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO_NAME) $(QEMUFLAGS)

gdb:
	$(MAKE) BUILD=debug iso
	i686-elf-gdb kernel-debug.elf -ex "target remote localhost:1234"

clean:
	rm -rf *.o *.elf *.iso $(ISO_DIR) $(BUILD) $(BUILD_DIR)
