CROSS   ?= aarch64-linux-gnu-
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ARCH_DIR := nemesisKernel/arch/arm64
KERNEL_DIR := nemesisKernel/kernel
MM_DIR := nemesisKernel/mm
DRIVERS_DIR := nemesisKernel/drivers
INC_DIR := nemesisKernel/include

CFLAGS := -ffreestanding -nostdlib -nostartfiles \
          -mgeneral-regs-only -march=armv8-a \
          -Wall -Wextra -O2 -I$(INC_DIR)

LDFLAGS := -T linker.ld -nostdlib

OBJS := \
	$(ARCH_DIR)/start.o \
	$(ARCH_DIR)/uart.o \
	$(ARCH_DIR)/main.o \
	$(ARCH_DIR)/exceptions.o \
	$(ARCH_DIR)/timer.o \
	$(ARCH_DIR)/switch.o \
	$(KERNEL_DIR)/panic.o \
	$(KERNEL_DIR)/print.o \
	$(KERNEL_DIR)/exceptions.o \
	$(KERNEL_DIR)/sched.o \
	$(MM_DIR)/pmm.o \
	$(DRIVERS_DIR)/gpu/virtio_gpu.o

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
		-device virtio-gpu-device \
		-kernel build/nemesisKernel.bin

run-gui: build/nemesisKernel.bin
	qemu-system-aarch64 -M virt -cpu cortex-a57 \
		-device virtio-gpu-device \
		-display gtk \
		-kernel build/nemesisKernel.bin

clean:
	rm -rf build $(OBJS)

.PHONY: all run run-gui clean
