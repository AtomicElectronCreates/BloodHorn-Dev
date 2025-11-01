Filesystem Implementation
========================

Overview
--------
This module provides filesystem support for the BloodHorn bootloader, enabling access to various
filesystem formats during the boot process. It includes support for multiple filesystem types
and provides a unified interface for filesystem operations.

Supported Filesystems
---------------------

EXT2/3/4
~~~~~~~~
- Full read support for EXT2/3/4 filesystems
- Handles block groups, inodes, and directory entries
- Supports both 32-bit and 64-bit variants

FAT12/16/32
~~~~~~~~~~~
- Complete read support for all FAT variants
- Handles long filenames (VFAT)
- Supports both BIOS Parameter Block (BPB) and Extended BIOS Parameter Block (EBPB)

ISO 9660
~~~~~~~~
- Read support for ISO 9660 (including Joliet and Rock Ridge extensions)
- Handles both Mode 1 and Mode 2 form 1/2 CD-ROM formats
- Supports El Torito bootable CD/DVD images

Core Components
---------------

Filesystem Common (fs_common.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Provides common filesystem utilities
- Implements path resolution and namei operations
- Handles filesystem caching and buffer management

Mount Management (fs_mount.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Manages mounted filesystems
- Handles mount points and path resolution
- Provides volume management functions

File Utilities (file_utils.h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Common file operation helpers
- Path manipulation utilities
- File attribute handling

Dependencies
------------
- UEFI Disk I/O Protocol
- UEFI File Protocol
- Memory allocation services

Building
--------
This component is built as part of the main BloodHorn build process. No additional configuration
is needed unless adding support for new filesystem types.

Usage Example
-------------
```c
#include "fs/fs_common.h"
#include "fs/fs_mount.h"

// Mount a filesystem
struct fs_mount *mount = fs_mount("fs0:", 0);
if (!mount) {
    // Handle error
}

// Open a file
struct file *file = fs_open("fs0:/boot/kernel.elf", O_RDONLY);
if (!file) {
    // Handle error
}

// Read from file
uint8_t buffer[512];
size_t bytes_read = fs_read(file, buffer, sizeof(buffer));

// Clean up
fs_close(file);
fs_umount(mount);
```

Adding New Filesystems
----------------------
To add support for a new filesystem:
1. Create new source files (e.g., `myfs.c` and `myfs.h`)
2. Implement the `struct filesystem_ops` interface
3. Register the filesystem using `fs_register_filesystem()`
4. Update the build system if needed

Documentation
-------------
See individual header files for detailed API documentation:
- ``fs_common.h``: Core filesystem interfaces
- ``fs_mount.h``: Mount management
- ``ext2.h``: EXT2/3/4 implementation
- ``fat32.h``: FAT12/16/32 implementation
- ``iso9660.h``: ISO 9660 implementation
