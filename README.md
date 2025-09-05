# BloodHorn

A bootloader supporting multiple architectures and boot protocols.

## Features
- Architectures: x86_64, ARM64, RISC-V, LoongArch
- Boot protocols: Linux, Multiboot 1/2, Limine, PXE, Chainloading
- Filesystems: FAT32, ext2/3/4
- Security: Secure Boot, TPM 2.0, file verification
- Network: PXE, TFTP, DHCP
- UI: Text and graphical modes
- Extensions: Lua scripting

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

## Documentation
See docs/ and other markdowns for detailed information on protocols and APIs.
## Contributors

- **[PacHash](https://github.com/PacHashs)** - Lead Developer
- **[UnpopularOpinion](https://github.com/unpopularopinionn)** - Main Team

- **[BillNyeTheScienceGuy](https://codeberg.org/BillNyeTheSienceGuy)** - Main Team

---

*BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch for educational purposes and for use in future operating systems.*

## License
MIT
