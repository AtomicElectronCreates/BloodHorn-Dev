32-bit Architecture Support
==========================

Overview
--------
The `Arch32` directory contains architecture-specific code for 32-bit platforms,
including boot protocol implementations and low-level hardware access routines.

Supported Architectures
----------------------
- x86 (IA-32)
- ARMv7
- MIPS32
- PowerPC
- RISC-V 32-bit

Components
----------

Boot Protocols
~~~~~~~~~~~~~
- `multiboot1.c/h` - Multiboot 1 protocol implementation
- `multiboot2.c/h` - Multiboot 2 protocol implementation
- `linux.c/h` - Linux boot protocol
- `chainload.c/h` - Chain loading support
- `limine.c/h` - Limine boot protocol

Architecture-Specific Code
~~~~~~~~~~~~~~~~~~~~~~~~~
- `ia32.c/h` - x86 (IA-32) specific code
- `aarch64.c/h` - ARM64 support (for 32-bit ARM compatibility)
- `riscv64.c/h` - RISC-V 64 support (for 32-bit compatibility)
- `loongarch64.c/h` - LoongArch 64 support (for 32-bit compatibility)

BloodChain
~~~~~~~~~~
Custom boot protocol implementation for BloodHorn, providing:
- Advanced memory management
- Device tree support
- Secure boot integration
- Multi-architecture support

Dependencies
------------
- Platform-specific assembly code
- Compiler intrinsics
- Low-level hardware access
- Boot protocol specifications

Usage
-----
```c
#include "multiboot1.h"
#include "ia32.h"

void boot_kernel(multiboot_info_t *mbi) {
    // Initialize architecture
    ia32_init();
    
    // Set up memory map
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        // Process memory map
    }
    
    // Boot the kernel
    multiboot_boot_kernel(mbi);
}
```

Building
--------
The architecture-specific code is built as part of the main bootloader build process.
To build for a specific architecture:

```bash
# For x86
make ARCH=ia32

# For ARM
make ARCH=arm

# For RISC-V
make ARCH=riscv
```

Porting to New Architectures
---------------------------
To add support for a new 32-bit architecture:
1. Create `<arch>.c` and `<arch>.h` files
2. Implement required architecture functions
3. Add build system support
4. Update platform detection

Documentation
-------------
- `multiboot1.h` - Multiboot 1 specification
- `multiboot2.h` - Multiboot 2 specification
- `ia32.h` - x86 architecture reference
- `aarch64.h` - ARM architecture reference

See Also
--------
- :doc:`../README` - Bootloader documentation
- :doc:`../libb/README` - Core boot library
