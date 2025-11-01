Boot Manager
===========

Overview
--------
The boot manager is responsible for the initial system boot process, including loading
the operating system kernel, handling boot configuration, and providing a user interface
for boot selection and configuration.

Features
--------
- Multi-boot support
- Configuration file parsing
- Boot menu interface
- Kernel loading and handoff
- Memory management during boot
- Device tree handling (for ARM/ARM64)

Components
----------

Boot Loader (boot.S, boot.c)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- First-stage boot loader (16/32-bit)
- Protected mode entry
- Memory detection
- Filesystem initialization

Boot Manager (bootmgr.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~
- Boot entry management
- Configuration parsing
- Environment variables
- Default boot selection

Kernel Loader (kernel.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~
- ELF/PE/COFF executable loading
- Relocation and symbol resolution
- Boot parameter preparation
- Handoff to kernel

Memory Management (mm.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~
- Early memory allocation
- Page table setup
- Memory region management
- Reserved memory tracking

Dependencies
------------
- Filesystem access
- Console I/O
- Memory management
- Platform-specific initialization

Usage
-----
```c
#include "boot/bootmgr.h"
#include "boot/kernel.h"

// Initialize boot manager
bootmgr_init();

// Load and execute kernel
struct boot_params params = {
    .cmdline = "console=ttyS0,115200",
    .initrd = NULL,
    .initrd_size = 0
};

if (load_kernel("/boot/vmlinuz", &params) == 0) {
    // Kernel loaded successfully
    boot_kernel(¶ms);
}
```

Configuration
-------------
The boot manager reads configuration from `/boot/grub/grub.cfg` or `/boot/grub.cfg`.
Example configuration:

```
timeout=5
default=0

title BloodHorn
    kernel /boot/kernel root=/dev/sda1
    initrd /boot/initrd.img
```

Building
--------
The boot manager is built as part of the main BloodHorn build process.
Platform-specific components are automatically selected based on the target architecture.

Debugging
---------
Enable debug output by setting the `BOOT_DEBUG` environment variable:

```bash
BOOT_DEBUG=1 qemu-system-x86_64 -kernel bloodhorn.efi
```

See Also
--------
- :doc:`../uefi/README`
- :doc:`../fs/README`
- :doc:`../security/README`
