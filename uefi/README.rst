UEFI Implementation
==================

Overview
--------
This module provides the core UEFI implementation for the BloodHorn bootloader. It handles the
initialization of UEFI runtime services, boot services, and provides a foundation for other
components to interact with the UEFI firmware.

Key Components
--------------

UEFI Core (uefi.c)
~~~~~~~~~~~~~~~~~~
- Initializes UEFI runtime and boot services
- Handles system table and handle protocol operations
- Manages memory allocation and memory map
- Provides utility functions for UEFI operations

Graphics (graphics.c)
~~~~~~~~~~~~~~~~~~~~~
- Handles UEFI Graphics Output Protocol (GOP)
- Manages display initialization and modes
- Provides basic framebuffer operations
- Implements text and primitive drawing functions

Dependencies
------------
- EDK2 UEFI headers
- Base UEFI protocols (SystemTable, BootServices, RuntimeServices)

Building
--------
This component is automatically built as part of the main BloodHorn build process. It requires
the EDK2 build environment to be properly set up.

Usage
-----
```c
#include "uefi/uefi.h"
#include "uefi/graphics.h"

// Example: Initialize UEFI services
efi_status status = uefi_init();
if (EFI_ERROR(status)) {
    // Handle error
}

// Example: Initialize graphics
status = graphics_init();
if (EFI_ERROR(status)) {
    // Handle error
}
```

Documentation
-------------
For detailed API documentation, see the header files in this directory.

- ``uefi.h``: Main UEFI interface definitions
- ``graphics.h``: Graphics-related functions and types
