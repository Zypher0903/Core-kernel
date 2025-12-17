# Core Microkernel - Makefile

ARCH ?= x86_64

# Toolchain
CC := $(ARCH)-elf-gcc
CXX := $(ARCH)-elf-g++
AS := nasm
LD := $(ARCH)-elf-ld
AR := $(ARCH)-elf-ar

# Directories
BUILD_DIR := build
KERNEL_DIR := kernel
INCLUDE_DIR := include
BOOT_DIR := boot
ISO_DIR := $(BUILD_DIR)/iso

# Output
KERNEL_BIN := $(BUILD_DIR)/core.elf
ISO_FILE := $(BUILD_DIR)/core.iso

# Flags
CFLAGS := -std=gnu11 -ffreestanding -O2 -Wall -Wextra \
          -mno-red-zone -mno-sse -mno-sse2 \
          -mcmodel=large -fno-pic -fno-pie \
          -I$(INCLUDE_DIR)

CXXFLAGS := -std=gnu++17 -ffreestanding -O2 -Wall -Wextra \
            -mno-red-zone -mno-sse -mno-sse2 \
            -mcmodel=large -fno-pic -fno-pie \
            -fno-rtti -fno-exceptions \
            -I$(INCLUDE_DIR)

ASFLAGS := -f elf64

LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker.ld

# Source files
BOOT_ASM := $(wildcard $(BOOT_DIR)/*.asm)
KERNEL_C := $(shell find $(KERNEL_DIR) -name '*.c')
KERNEL_CPP := $(shell find $(KERNEL_DIR) -name '*.cpp')
KERNEL_ASM := $(shell find $(KERNEL_DIR) -name '*.asm')

# Object files
BOOT_OBJ := $(patsubst $(BOOT_DIR)/%.asm,$(BUILD_DIR)/boot/%.o,$(BOOT_ASM))
KERNEL_C_OBJ := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel/%.o,$(KERNEL_C))
KERNEL_CPP_OBJ := $(patsubst $(KERNEL_DIR)/%.cpp,$(BUILD_DIR)/kernel/%.o,$(KERNEL_CPP))
KERNEL_ASM_OBJ := $(patsubst $(KERNEL_DIR)/%.asm,$(BUILD_DIR)/kernel/%.o,$(KERNEL_ASM))

ALL_OBJ := $(BOOT_OBJ) $(KERNEL_C_OBJ) $(KERNEL_CPP_OBJ) $(KERNEL_ASM_OBJ)

.PHONY: all clean iso run debug

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(ALL_OBJ)
	@mkdir -p $(dir $@)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJ)
	@echo "Build complete: $@"

$(BUILD_DIR)/boot/%.o: $(BOOT_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(AS) $(ASFLAGS) $< -o $@

iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/core.elf
	@echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "Core Kernel" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/core.elf' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)
	@echo "ISO created: $(ISO_FILE)"

run: iso
	qemu-system-x86_64 -cdrom $(ISO_FILE) -m 512M -serial stdio

debug: iso
	qemu-system-x86_64 -cdrom $(ISO_FILE) -m 512M -serial stdio -s -S

clean:
	rm -rf $(BUILD_DIR)