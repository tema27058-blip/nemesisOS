CROSS   ?= aarch64-linux-gnu-
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ARCH_DIR := nemesisKernel/arch/arm64
KERNEL_DIR := nemesisKernel/kernel
INC_DIR := nemesisKernel/include

CFLAGS := -ffreestanding -nostdlib -nostartfiles \
          -mgeneral-regs-only -march=armv8-a \
          -Wall -Wextra -O2 -I$(INC_DIR)

LDFLAGS := -T linker.ld -nostdlib

OBJS := \
	$(ARCH_DIR)/start.o \
	$(ARCH_DIR)/uart.o \
	$(ARCH_DIR)/main.o \
	$(KERNEL_DIR)/panic.o \
	$(KERNEL_DIR)/print.o

all: build/nemesisKernel.bin

build/nemesisKernel.elf: $(OBJS) linker.ld
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

build/nemesisKernel.bin: build/nemesisKernel.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

run: build/nemesisKernel.bin
	qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic \
		-kernel build/nemesisKernel.bin

clean:
	rm -rf build $(OBJS)

.PHONY: all run clean
