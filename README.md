# BloodHorn

A bootloader supporting multiple architectures and boot protocols.

## Support Me

If you like this project and want to support me, you can donate through [Liberapay](https://liberapay.com/Listedroot/donate).

[![Build Status](https://ci.codeberg.org/api/badges/PacHashs/BloodHorn/status.svg)](https://ci.codeberg.org/PacHashs/BloodHorn)

[![Donate using Liberapay](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/Listedroot/donate)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-x86__64%20%7C%20ARM64%20%7C%20RISC--V%20%7C%20LoongArch-blue)](https://codeberg.org/PacHashs/BloodHorn)

## Project Background
BloodHorn started as an experimental bootloader project aimed at simplifying the process of building and testing modern firmware-based systems.
It was initially developed by Alexander Roman. Aka PacHashs as part of research into cross-architecture initialization and early-boot design.

The idea behind BloodHorn came from the growing need for a clean, modular, and adaptable boot environment that supports multiple architectures and modern build systems without relying on legacy codebases.
Most existing bootloaders are either heavily tied to a single platform, too complex to extend, or outdated in terms of design and structure.
BloodHorn was created to address those limitations by focusing on clarity, maintainability, and modern development practices.

While the project is still evolving, its long-term goal is to become a flexible and developer-friendly foundation for both experimental and production operating systems.
It’s built with an emphasis on EDK2 standards, architecture abstraction, and transparent build configuration, allowing contributors to focus more on system logic and less on build friction.

**Recent Development:** BloodHorn has been enhanced with comprehensive Coreboot firmware platform integration, providing a hybrid boot environment that leverages Coreboot's direct hardware control while maintaining UEFI compatibility for maximum flexibility and performance.

BloodHorn isn’t meant to compete with GRUB or existing loaders .it’s meant to serve as a modern reference implementation for anyone interested in understanding or developing firmware-level code in a clean and organized environment.
## Features
- Architectures: x86_64, ARM64, RISC-V, LoongArch
- Boot protocols: Linux, Multiboot 1/2, Limine, PXE, Chainloading, BloodChain
- Filesystems: FAT32, ext2/3/4
- Security: Secure Boot, TPM 2.0, file verification
- Network: PXE, TFTP, DHCP
- UI: Text and graphical modes
- Extensions: Lua scripting
- **Firmware Support:** UEFI and Coreboot hybrid architecture with automatic detection
- **Hardware Integration:** Direct hardware control via Coreboot + UEFI service compatibility

## Building
1. Install EDK2 and required toolchain
2. Run: build -p BloodHorn/BloodHorn.dsc
3. Output will be in Build/BloodHorn/

## Usage
1. Copy BloodHorn.efi to your EFI partition
2. Configure your bootloader to load it
3. Place configuration files in the root of your boot partition:
   - config.ini for INI format
   - config.json for JSON format
   - Environment variables are also supported

**Coreboot Integration:** BloodHorn automatically detects and adapts to Coreboot firmware when running as a payload, providing enhanced hardware control and performance while maintaining UEFI compatibility.

## Documentation
See docs/ and other markdowns for detailed information on protocols and APIs.

**Coreboot Integration:** For detailed information about BloodHorn's Coreboot firmware integration, see `docs/COREBOOT_INTEGRATION.md`.

## FAQ

### What is the size of BloodHorn?
BloodHorn source code is approximately 500 KB before compilation. After building, the bootloader binary ranges from 1.5 to 2 MB depending on enabled features and architecture.

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
MIT
