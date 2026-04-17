CC=i686-elf-gcc
LD=i686-elf-gcc
CFLAGS=-std=gnu11 -ffreestanding -O2 -Wall -Wextra
LDFLAGS=-T linker.ld -ffreestanding -O2 -nostdlib

all: kernel.bin

kernel.bin: boot.o kernel.o
	$(LD) $(LDFLAGS) -o kernel.bin boot.o kernel.o -lgcc

boot.o: boot.s
	$(CC) $(CFLAGS) -c boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

clean:
	rm -rf *.o kernel.bin
