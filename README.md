# BloodHorn

<p align="center">
  <img src="./Z.png" alt="BloodHorn Logo" width="420"/>
  <br>
  <em>Fast, secure, and reliable system bootloader</em>
</p>

[![Quality](quality_badge.svg)](./) [![Build Status](https://ci.codeberg.org/api/badges/PacHashs/BloodHorn/status.svg)](https://ci.codeberg.org/PacHashs/BloodHorn) [![Platform](https://img.shields.io/badge/platform-x86__64%20%7C%20ARM64%20%7C%20RISC--V%20%7C%20LoongArch-blue)](https://codeberg.org/PacHashs/BloodHorn) [![License](https://img.shields.io/badge/License-BSD--2--Clause--Patent-blue.svg)](LICENSE)
## About
BloodHorn is a compact, extensible bootloader built with EDK2 and optional Coreboot payload support. It targets UEFI and legacy workflows on multiple architectures and emphasizes secure module verification and maintainability.

## Safety & Audience
BloodHorn operates at firmware level and can affect device boot and data. Read `SECURITY.md` before use and back up any existing bootloaders.

Intended audience: firmware engineers, OS developers, and security researchers.

## CI
We run CI builds and tests on the public pipeline. See `.woodpecker.yml` for details and the CI dashboard for current status.

## Support
If you find BloodHorn useful, see `CONTRIBUTING.md` or support via the project's sponsorship links.

## Dependencies

BloodHorn uses the following external dependencies as Git submodules:

- **FreeType** (`boot/freetype`): Font rendering library for TTF/OTF support
  - Repository: https://gitlab.freedesktop.org/freetype/freetype.git
  - Initialized with `git submodule update --init`

- **Edk2BH** (`Edk2BH`): Modified EDK2 firmware framework
  - Repository: https://codeberg.org/BloodyHell-Industries-INC/Edk2BH.git
  - Custom EDK2 modifications for BloodHorn bootloader
  - Initialized with `git submodule update --init`

- **cc-runtime** (`cc-runtime`): Freestanding compiler runtime library
  - Repository: https://codeberg.org/OSDev/cc-runtime.git
  - Subset of LLVM's compiler-rt libgcc-compatibility functions for bare-metal environments
  - Credits: Compressed and packaged by **mintsuki** for easy integration
  - Original: LLVM compiler-rt project and maintainers
  - Initialized with `git submodule update --init`

## Build and installation (from source)

Note: This repository does not include prebuilt binaries. Build from source; see `INSTALL.md` for platform-specific instructions.

### Example build (Ubuntu/Debian)

```bash
# Install common dependencies
sudo apt update
sudo apt install -y build-essential nasm uuid-dev acpica-tools python3 qemu-system-x86 ovmf xorriso gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu

# Clone and set up EDK2
git clone --depth 1 https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init
make -C BaseTools
export EDK_TOOLS_PATH=$(pwd)/BaseTools
export PACKAGES_PATH=$(pwd):$(pwd)/..
. edksetup.sh

# Prepare and build BloodHorn (from edk2 root)
cp -r ../BloodHorn .
cd BloodHorn
git submodule update --init  # Initialize FreeType dependency
cd ..
build -a X64 -p BloodHorn.dsc -b RELEASE -t GCC5
```

Artifacts: `Build/BloodHorn/RELEASE_GCC5/X64/BloodHorn.efi` (UEFI) and BIOS artifacts under `Build/.../bios/`.

Installation (UEFI example)

```bash
sudo mount /dev/sda1 /mnt/efi   # adjust device
sudo mkdir -p /mnt/efi/EFI/BloodHorn
sudo cp Build/BloodHorn/RELEASE_GCC5/X64/BloodHorn.efi /mnt/efi/EFI/BloodHorn/BloodHorn.efi
sudo umount /mnt/efi
```

For Windows installation, USB image creation, PXE/HTTP boot, and detailed build options, see `INSTALL.md`.

## Layout
Top-level folders include `coreboot/`, `uefi/`, `security/`, `fs/`, and `net/`.

## Features

- Multi-architecture support (x86_64, ARM64, RISC-V, LoongArch)
- Secure Boot and TPM 2.0 support
- Coreboot payload support and UEFI integration

## History

BloodHorn started as an early bootloader prototype in 2016 and has since evolved into a cross-architecture payload with a focus on security and maintainability.

![First BloodHorn Screenshot (2016)](./2016screenshot.png)

## Configuration
Create a minimal `bloodhorn.ini` next to `BloodHorn.efi` on the EFI partition:
## FAQ

- What architectures are supported? - x86_64, ARM64, RISC-V, LoongArch.
- Is Secure Boot supported? - Yes. BloodHorn includes Secure Boot support and optional TPM verification.
- How to contribute? - See `CONTRIBUTING.md` and open issues or PRs on the project repo.

## Contributors

See `maintainers.py` and the project contributors list.

---

![BSD License](bsd.svg)

BloodHorn is licensed under the BSD-2-Clause-Patent License. See `LICENSE` for details.
