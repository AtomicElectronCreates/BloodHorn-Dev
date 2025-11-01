Configuration System
==================

Overview
--------
The `config` directory contains configuration files and utilities for building
and customizing the BloodHorn bootloader. This includes compiler flags,
feature toggles, and platform-specific settings.

Configuration Files
------------------
- `target.txt` - Target platform configuration
- `tools_def.txt` - Toolchain definitions
- `build_rule.txt` - Build rules and file type handling
- `components/` - Component-specific configurations
- `platform/` - Platform-specific settings
- `uefi/` - UEFI-specific configurations
- `bios/` - Legacy BIOS configurations

Build Configuration
------------------
### Target Configuration (`target.txt`)
Defines the target platform, architecture, and toolchain:
```
ACTIVE_PLATFORM = BloodHorn/BloodHorn.dsc
TARGET_ARCH = X64
TOOL_CHAIN_TAG = GCC5
MAX_CONCURRENT_THREAD_NUMBER = 8
BUILD_RULE_CONF = conf/build_rule.txt
```

### Toolchain Configuration (`tools_def.txt`)
Defines compiler and linker flags for different toolchains:
```
*_GCC5_*_CC_FLAGS = -Wall -Werror -O2
*_GCC5_*_DLINK_FLAGS = -Wl,--gc-sections
```

### Build Rules (`build_rule.txt`)
Specifies how different file types are processed during the build:
```
[AcpiTable]
    .asl, .aslc -> .aml : AcpiTable

[Library]
    .c, .h -> .obj : C
    .S, .asm -> .obj : ASM
```

Platform Configuration
---------------------
### UEFI Configuration (`uefi/`)
- `Pcds/` - Platform Configuration Database settings
- `Drivers/` - UEFI driver configurations
- `Library/` - UEFI library configurations

### BIOS Configuration (`bios/`)
- `LegacyBios/` - Legacy BIOS support
- `LegacyBoot/` - Legacy boot configurations
- `OptionRom/` - Option ROM configurations

Component Configuration
----------------------
- `Network/` - Network stack configuration
- `Security/` - Security feature settings
- `Storage/` - Storage stack configuration
- `Graphics/` - Graphics and display settings

Usage
-----
### Building with Custom Configuration
```bash
# Set the target configuration
build -p BloodHorn.dsc -t GCC5 -a X64 -b RELEASE

# Override specific settings
build -p BloodHorn.dsc -t GCC5 -a X64 -b RELEASE \
    -D NETWORK_ENABLE=TRUE \
    -D SECURE_BOOT_ENABLE=TRUE
```

### Adding New Configuration
1. Create a new `.dsc` or `.dec` file in the appropriate directory
2. Define your configuration options
3. Update the main platform configuration to include your new file

Environment Variables
-------------------
- `BLOODHORN_CONFIG` - Path to configuration directory
- `BLOODHORN_TARGET` - Target platform (default: DEBUG)
- `BLOODHORN_ARCH` - Target architecture (default: X64)
- `BLOODHORN_TOOLCHAIN` - Toolchain to use (default: GCC5)

See Also
--------
- :doc:`../Edk2BHModules/README` - EDK2 modules documentation
- :doc:`../boot/README` - Bootloader documentation
