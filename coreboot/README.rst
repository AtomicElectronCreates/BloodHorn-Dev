Coreboot Integration
===================

Overview
--------
This module provides integration with Coreboot, an open-source firmware platform.
It allows BloodHorn to function as a Coreboot payload, providing a UEFI-compatible
interface on top of Coreboot's native services.

Features
--------
- Coreboot table parsing
- Memory map translation
- ACPI table handling
- SMBIOS support
- Platform initialization

Components
----------

Coreboot Tables (coreboot_tables.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Coreboot table header parsing
- Memory range enumeration
- Device tree construction
- Resource management

ACPI Support (acpi.c/h)
~~~~~~~~~~~~~~~~~~~~~~
- ACPI table generation
- Power management
- Hardware discovery
- System event handling

SMBIOS (smbios.c/h)
~~~~~~~~~~~~~~~~~~~
- SMBIOS table generation
- System information reporting
- Hardware inventory
- BIOS information

Platform Initialization (platform.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Early hardware initialization
- Console setup
- Memory controller configuration
- Device enumeration

Dependencies
------------
- Coreboot firmware
- x86 or ARM architecture
- Memory management
- Console I/O

Usage
-----
```c
#include "coreboot/coreboot.h"
#include "coreboot/tables.h"

// Initialize Coreboot integration
if (coreboot_init() != CB_SUCCESS) {
    // Handle error
}

// Get system memory map
struct lb_memory *mem = coreboot_get_memory_map();
if (mem) {
    // Process memory regions
    for (int i = 0; i < mem->size; i++) {
        // Handle memory range
    }
}

// Get ACPI tables
void *acpi_rsdp = coreboot_get_rsdp();
if (acpi_rsdp) {
    // Process ACPI tables
}
```

Building
--------
To build BloodHorn as a Coreboot payload:

```bash
make PLATFORM=coreboot TARGET=x86_64
```

Configuration
-------------
Coreboot integration can be configured via Kconfig options:

- `CONFIG_COREBOOT`: Enable Coreboot support
- `CONFIG_COREBOOT_SERIAL`: Serial console support
- `CONFIG_COREBOOT_CBFS`: Coreboot Filesystem support
- `CONFIG_COREBOOT_LOGGING`: Enable debug logging

Debugging
---------
Enable debug output by setting the log level:

```c
coreboot_set_loglevel(COREBOOT_LOG_DEBUG);
```

See Also
--------
- `Coreboot Documentation <https://doc.coreboot.org/>`_
- :doc:`../uefi/README`
- :doc:`../boot/README`
