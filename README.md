# BloodHorn

<p align="center">
  <img src="Z.png" alt="BloodHorn Logo" width="500"/>
  <br>
  <em>Fast, secure, and reliable system bootstrapping</em>
</p>

![Code Quality](./quality_badge.svg)
##### Note: this is a mirror of bloodhorn's main repo in codeberg . all updates will go to codeberg before this github mirror .
- you can see the main codeberg repo at : https://codeberg.org/PacHashs/BloodHorn
- THIS REPO is to test PR expermients . it's not an upstream 
---
## About BloodHorn
BloodHorn is a modern, high-performance bootloader built on the EDK2 framework with Coreboot integration. Designed for speed, security, and reliability, it provides a robust foundation for system initialization across multiple architectures.

## Important Safety Notice

BloodHorn is a low-level system component that interacts directly with hardware. Incorrect use may result in system damage, data loss, or security vulnerabilities.

### Intended Audience
- System firmware developers
- Security researchers
- Embedded systems engineers
- Advanced users with recovery capabilities

### Technical Requirements
- UEFI/BIOS and system architecture knowledge
- Low-level programming experience (C, assembly) or at least knowing fundumentals of dealing with low level components
- Secure boot and firmware security understanding
- Hardware recovery tools and procedures

### System Impact
BloodHorn operates at the hardware level with the following characteristics:
- Multi-architecture support (x86_64, ARM, RISC-V, LoongArch)
- EDK2 Framework Integration
- Coreboot Payload Support
- Security features including TPM and secure boot
- Modular plugin architecture

Note: The BloodHorn team is not responsible for any hardware damage or data loss resulting from the use of this software.

## Project History

### Origins (2016)
The BloodHorn project originated from an open-source bootloader prototype discovered in 2016. The initial codebase was developed by an anonymous contributor and shared on a technical forum in 4chan. The current maintainer recognized its potential and began maintaining and expanding the project.

### Development Timeline
- **2016-2018**: Core functionality refinement and initial testing
- **2019-2021**: Major architectural improvements and feature additions
- **2022-2023**: Codebase modernization and security enhancements
- **2024-Present**: Active maintenance and community development

### Project Philosophy
BloodHorn is developed with a focus on:
- Technical excellence
- Minimal resource usage
- Clean, maintainable code
- Practical functionality over marketing

## Support Me

If you like this project and want to support me, you can donate through [Liberapay](https://liberapay.com/Listedroot/donate).

[![Build Status](https://ci.codeberg.org/api/badges/PacHashs/BloodHorn/status.svg)](https://ci.codeberg.org/PacHashs/BloodHorn)

[![Donate using Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/Listedroot/donate)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-x86__64%20%7C%20ARM64%20%7C%20RISC--V%20%7C%20LoongArch-blue)](https://codeberg.org/PacHashs/BloodHorn)

## Getting Started

### Prerequisites
- Git
- Python 3.7 or later
- C/C++ toolchain (GCC/Clang on Linux/macOS, MSVC on Windows)
- EDK2 (pre-installed and configured)

### Building from Source

#### Prerequisites
- Git
- Python 3.7 or later
- C/C++ toolchain (GCC/Clang on Linux/macOS, MSVC on Windows)
- EDK2 BaseTools (included in Edk2BHModules)
- NASM (for assembly components)
- iASL (for ACPI support)

#### Linux/macOS
```bash
git clone --recursive https://codeberg.org/PacHashs/BloodHorn.git
cd BloodHorn
# Build BaseTools if not already built
make -C Edk2BHModules/BaseTools
# Set up build environment
source edksetup.sh
# Build BloodHorn (Release)
build -p BloodHorn.dsc -b RELEASE -t GCC5 -a X64 -n $(nproc)
# Or for Debug build with Coreboot support
# build -p BloodHorn.dsc -b DEBUG -t GCC5 -a X64 -n $(nproc) -D COREBOOT_ENABLED=1
```

#### Windows
```cmd
git clone --recursive https://codeberg.org/PacHashs/BloodHorn.git
cd BloodHorn
:: Build BaseTools if not already built
cd Edk2BHModules\BaseTools
nmake
cd ..\..
:: Set up environment (run from Visual Studio x64 Native Tools Command Prompt)
:: Build BloodHorn (Release)
build -p BloodHorn.dsc -b RELEASE -t VS2019 -a X64
:: Or for Debug build with Coreboot support
:: build -p BloodHorn.dsc -b DEBUG -t VS2019 -a X64 -D COREBOOT_ENABLED=1
```

Output files:
- `Build/BloodHorn/RELEASE_<TOOLCHAIN>/X64/BloodHorn.efi`
- `Build/BloodHorn/DEBUG_<TOOLCHAIN>/X64/BloodHorn.efi`

## Installation

### Linux/macOS
```bash
# Mount EFI partition (if not already mounted)
mount /dev/sdX1 /mnt/efi

# Install BloodHorn
cp Build/BloodHorn/RELEASE_GCC5/X64/BloodHorn.efi /mnt/efi/EFI/BOOT/BOOTX64.EFI
```

### Windows (Admin)
```cmd
:: Install to default boot location
copy Build\BloodHorn\RELEASE_VS2019\X64\BloodHorn.efi C:\EFI\Microsoft\Boot\bootmgfw.efi
```

> **Note:** Always back up your existing bootloader before installation.

## Project Structure

```
BloodHorn/
├── BloodHorn.dsc       # Main build configuration
├── BloodHorn.fdf       # FD image layout
├── BloodHorn.inf       # Module definition
├── coreboot/           # Coreboot integration
├── uefi/              # UEFI implementation
├── security/          # TPM and security features
├── fs/                # Filesystem support
├── net/               # Network booting
└── plugins/           # Extensible modules
```

## Features

- **Architectures**: x86_64, ARM64, RISC-V, LoongArch
- **Security**: Secure Boot, TPM 2.0, file verification
- **Firmware**: Coreboot + UEFI hybrid
- **Configuration**: INI/JSON/UEFI variables
- **UI**: Text and graphical interfaces with localization
- **Performance**: Optimized boot times
   ```

4. **Basic Configuration**:
   Create a minimal `bloodhorn.ini` at your EFI partition root:
   ```ini
   [boot]
   default = linux
   menu_timeout = 5
   language = en
   font_path = /fonts/ter-16n.psf

   [linux]
   kernel = /boot/vmlinuz
   initrd = /boot/initrd.img
   cmdline = root=/dev/sda1 ro quiet
- **Recovery Mode**: For when you thought "rm -rf /" was a good idea at 3 AM
- **Boot Speed**: Faster than you can say "I'll just check Reddit while I wait"

## Coreboot Integration

BloodHorn features comprehensive Coreboot firmware integration through the CorebootPayloadPkg. When built with Coreboot support, it provides:

- **Automatic Detection**: Automatically detects when running as a Coreboot payload
- **Hybrid Initialization**: Leverages Coreboot's hardware initialization while maintaining UEFI compatibility
- **Memory Management**: Proper handling of Coreboot memory maps and tables
- **Hardware Access**: Direct access to Coreboot-provided hardware information
- **Performance**: Optimized boot path when running on Coreboot

### Building with Coreboot Support

To enable Coreboot payload support, use the `COREBOOT_ENABLED` flag during build:

```bash
build -p BloodHorn.dsc -b RELEASE -t GCC5 -a X64 -D COREBOOT_ENABLED=1
```

### CorebootPayloadPkg

The CorebootPayloadPkg provides the following features:
- Coreboot table parsing and validation
- Memory map translation between Coreboot and UEFI formats
- Hardware information extraction from Coreboot tables
- Integration with the main BloodHorn bootloader

For more details, see the [CorebootPayloadPkg documentation](Edk2BHModules/CorebootPayloadPkg/README.rst).

## FAQ

### What is the size of BloodHorn?
The BloodHorn source code is approximately 14 MiB. After compilation, the bootloader binary can grow up to 30 MiB, depending on the enabled features, optimizations, and target architecture. The size may vary based on the build configuration and included modules.

### How do I contribute to BloodHorn?
Contributions are welcome! Please see our contribution guidelines in the docs/ directory and feel free to open issues or submit pull requests on our Codeberg repository.

### Does BloodHorn support Secure Boot?
Yes, BloodHorn includes comprehensive Secure Boot support with TPM 2.0 integration and cryptographic verification of all loaded modules.

### Does BloodHorn support Coreboot firmware?
Yes, BloodHorn features comprehensive Coreboot firmware integration with automatic detection and hybrid initialization. When running as a Coreboot payload, it leverages Coreboot's direct hardware control for enhanced performance while maintaining UEFI compatibility for broader system support.

## Contributors

- **[PacHash](https://github.com/PacHashs)** - Lead Developer
- **[BillNyeTheScienceGuy](https://codeberg.org/BillNyeTheSienceGuy)** - Main Team

---

*BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch for educational purposes and for use in future operating systems.*

## License
licensed under BSD-2-Clause-Patent License
