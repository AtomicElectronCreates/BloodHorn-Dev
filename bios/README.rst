BIOS Compatibility Layer
=======================

Overview
--------
This module provides compatibility with legacy BIOS systems, allowing BloodHorn to boot on
both UEFI and legacy BIOS platforms. It implements the necessary interfaces to interact
with BIOS services and provides a translation layer between BIOS and UEFI-like functionality.

Features
--------
- BIOS Interrupt Service Routines (ISR) wrappers
- Memory map translation
- Disk access via BIOS INT 13h
- Video mode setting
- A20 gate control

Components
----------

BIOS Services (bios.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~
- Low-level BIOS interrupt handling
- Memory map generation
- Disk I/O services
- System configuration access

Legacy Boot (legacy_boot.asm)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- 16-bit real mode entry point
- Protected mode switching
- Memory map collection
- Boot device detection

VESA Support (vesa.c/h)
~~~~~~~~~~~~~~~~~~~~~~
- VESA BIOS Extensions (VBE) support
- Video mode enumeration and setting
- Framebuffer access
- Backward compatible video modes

Dependencies
------------
- x86 architecture
- BIOS services
- Memory below 1MB
- Real-mode compatible stack

Usage
-----
```c
#include "bios/bios.h"

// Initialize BIOS services
bios_init();

// Get memory map
struct bios_memory_map map;
if (bios_get_memory_map(&map) == 0) {
    // Use memory map
}

// Read from disk
uint8_t buffer[512];
if (bios_disk_read(0x80, 0, 1, buffer) != 0) {
    // Handle error
}
```

Building
--------
This component is conditionally compiled when targeting BIOS-based systems.
The build system automatically includes the necessary assembly files and
configures the appropriate memory model.

Limitations
-----------
- Limited to x86 architecture
- Requires real-mode compatibility
- No support for UEFI Secure Boot
- Limited memory above 1MB in real mode
