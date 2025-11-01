Boot Library (libb)
==================

Overview
--------
The `libb` directory contains the core boot library that provides essential services
for the BloodHorn bootloader. It offers a hardware-agnostic interface for system
initialization and management.

Core Headers
------------
- `bloodhorn.h` - Main library header and initialization
- `types.h` - Common type definitions and macros
- `status.h` - Status codes and error handling
- `system.h` - System control and information
- `memory.h` - Memory management functions
- `graphics.h` - Graphics and display handling
- `input.h` - Input device handling
- `fs.h` - Filesystem abstraction layer
- `time.h` - Time-related functions
- `debug.h` - Debugging and logging utilities
- `bootinfo.h` - Boot information structures

Key Features
------------
- Hardware abstraction layer
- Memory management
- Graphics and text output
- Input device handling
- Filesystem access
- Debugging support
- Boot information management

Usage
-----
```c
#include <bloodhorn/bloodhorn.h>
#include <bloodhorn/bootinfo.h>

int main() {
    // Initialize the library
    bh_status_t status = bh_initialize();
    if (status != BH_STATUS_SUCCESS) {
        // Handle error
        return -1;
    }

    // Get boot information
    const bh_bootinfo_t *bootinfo = bh_get_bootinfo();
    
    // Use library functions...
    
    // Clean up
    bh_terminate();
    return 0;
}
```

Dependencies
------------
- Standard C library
- Platform-specific hardware access
- UEFI or BIOS services (depending on platform)

Building
--------
The library is built as part of the main bootloader build process. It can also be built
as a standalone library by defining `BUILD_LIBB`:

```bash
gcc -DBUILD_LIBB -Iinclude -c bloodhorn.c -o bloodhorn.o
ar rcs libbloodhorn.a bloodhorn.o
```

Documentation
-------------
For detailed API documentation, see the header files in the `include/bloodhorn` directory.

See Also
--------
- :doc:`../README` - Bootloader documentation
- :doc:`../Arch32/README` - 32-bit architecture support
