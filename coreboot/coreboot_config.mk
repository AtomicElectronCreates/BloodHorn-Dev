# BloodHorn Coreboot Configuration File
# This file contains configuration options for Coreboot firmware integration

# Coreboot source configuration
COREBOOT_VERSION=v4.22
COREBOOT_SOURCE_URL=https://github.com/coreboot/coreboot.git
COREBOOT_CONFIG=mainboard/google/octopus

# Payload configuration
PAYLOAD_NAME=BloodHorn
PAYLOAD_BINARY=bloodhorn.elf
PAYLOAD_CMDLINE="console=ttyS0 root=/dev/sda1"

# Build options
BUILD_TYPE=release
ENABLE_DEBUG=false
ENABLE_TESTS=false

# Hardware configuration
CPU_MICROCODE_UPDATE=true
SMM_SUPPORT=true
ACPI_SUPPORT=true
SMBIOS_SUPPORT=true

# Graphics configuration
FRAMEBUFFER_KEEP_VESA_MODE=false
FRAMEBUFFER_NATIVE_RESOLUTION=1920x1080

# Security configuration
TPM_SUPPORT=true
SECURE_BOOT=true
MEASURED_BOOT=true

# Device support
USB_SUPPORT=true
SATA_SUPPORT=true
NVME_SUPPORT=true
NETWORK_SUPPORT=true

# Mainboard specific options
MAINBOARD_VENDOR=google
MAINBOARD_MODEL=octopus
MAINBOARD_VARIANT=octopus

# Build directories
BUILD_DIR=build
OUTPUT_DIR=output
TOOLCHAIN_DIR=/opt/coreboot-sdk

# Compiler options
CC=gcc
CFLAGS=-O2 -pipe -Wall -Werror
LDFLAGS=-Wl,--gc-sections

# Coreboot make options
COREBOOT_MAKE_OPTS="
    CPUS=$(nproc)
    CONFIG_PAYLOAD_FILE=$(BUILD_DIR)/$(PAYLOAD_BINARY)
    CONFIG_PAYLOAD_CMDLINE=\"$(PAYLOAD_CMDLINE)\"
"
