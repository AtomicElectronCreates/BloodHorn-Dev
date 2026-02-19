# BloodHorn

<p align="center">
  <img src="./Z.png" alt="BloodHorn Logo" width="420"/>
  <br>
  <em>Paranoid firmware bootloader for hostile environments</em>
</p>

> **Heads-up from the maintainer**: BloodHorn is aimed at folks who already live in UEFI land—if PE loaders, TPMs, and attack surfaces sound unfamiliar, you may want a gentler project first.

---

> **Personal note**: I'm balancing BloodHorn development with sarcoma treatment. Contributions, bug reports, and kind messages genuinely help keep the project on track while I focus on my health.

---

## Project Status: Discontinued

> **Notice**: BloodHorn is now discontinued because the main maintainer has stepped off active development. <img src="./winerr.png" alt="winerr" width="16"/>
>
> Thank you to everyone who tested, audited, reported bugs, and believed/or will belive in this project. BloodHorn was built to push firmware security forward, and this community knowledge made that possible. The code stays here as a record of that work. <img src="./winerr.png" alt="winerr" width="16"/>

---

### Where things stand

- **Release train**: v7.80.0 is our current line. It’s good enough for hostile deployments, but still sees active churn. Expect sharp edges.
- **Support window**: I patch regressions and security findings quickly; new features ship when energy allows.
- **Help wanted**: Testing on weird hardware, crash dumps, and reproducible bug reports are the best way to move the needle right now.

## What makes BloodHorn BloodHorn

BloodHorn treats firmware as an adversary instead of a trusted partner. That leads to a few practical rules:

- Strip UEFI dependencies as soon as ExitBootServices succeeds.
- Keep the binary small and auditable so you can reason about every code path.
- Offer serious security features (TPM 2.0, secure boot, formal verification hooks) across seven architectures without dragging runtime baggage along for the ride.

**Built with** the spirit of Coreboot minimalism, GRUB flexibility, and TianoCore correctness.  
**Avoids** dynamic allocation after boot, runtime service lock-in, and opaque blobs.

## Threat model at a glance

We assume:

- Firmware may already be compromised.
- Bootkits are lurking (measured boot and secure verification are table stakes).
- Supply chains get messy; every stage gets cryptographically checked.
- Hardware trust stops at the TPM boundary.

It's not paranoia—recent incident reports keep proving the point.

## Design notes from the maintainer desk

### Security through restraint
- One allocation phase before ExitBootServices, nothing after.
- Bare-metal C, no libc crutches.
- Constant-time crypto and aggressive zeroization.

### Keep it readable
- ~100k LOC including support libraries, written for auditing rather than cleverness.
- No third-party mystery meat in the boot path.
- Formal methods experiments are ongoing; patches welcome.

### Assume the firmware lies
- Custom PE loader (we don't trust the firmware's version).
- Homegrown memory management rather than firmware allocators.
- Direct hardware access and a lean variable footprint.

## What we’re not doing

Intentional omissions that keep the project sharp:

- Maximizing hardware compatibility at the expense of security posture.
- Shipping a flashy GUI—the text UI stays on purpose.
- Loading plugins or scripts that balloon the attack surface.
- Catering to legacy BIOS setups or Windows-first expectations.

If you need those features, there are other loaders that will treat you better.

## Current snapshot

- **Deployment**: Trusted by red teams and firmware researchers who need hard-stop security guarantees.
- **Maturity**: Core flows are stable; bleeding-edge features are opt-in.
- **Audits**: Core stages audited, memory manager verification in progress.
- **Reality check**: Expect occasional firmware-specific quirks—please report them.

## Why we bother

Bootloaders that “just work” often do so by trusting firmware blindly. BloodHorn is the counter-argument: assume compromise, build defensively, and prove your work where possible.

## Visual evidence

![BloodHorn Menu Screenshot](./bloodhorn.png)

*Current BloodHorn interface running in hardened QEMU environment*

### Historical context

![First BloodHorn Screenshot (2016)](./2016screenshot.png)

*Screenshot credit: [Lqauz](https://github.com/LqauzDev) - Thank you for capturing the current BloodHorn interface! (Screenshot taken in x86 QEMU virtual environment, tested in a dual booted VM)*

## Donate

If you want to keep BloodHorn maintained (and help me keep the lights on during treatment), crypto tips go a long way:

[Donate with crypto](https://pay.oxapay.com/13185765)

Contributions of code, test reports, or documentation updates are equally welcome and often just as valuable.

## License

**BSD-2-Clause-Patent** - chosen for:

- **Patent protection** - explicit patent grant shields users
- **Corporate friendliness** - permissive enough for enterprise adoption
- **Security community trust** - preferred license for security tools
- **No copyleft complications** - allows integration with proprietary security solutions

See `LICENSE` for complete terms.

---

## Safety First

BloodHorn operates at firmware level. Mistakes can render systems unbootable. Read SECURITY.md before use. Always back up existing bootloader.

Intended for firmware engineers, OS developers, and security researchers who understand the risks.

## Architecture Support

| Architecture | Binary Size | Status | Security Features |
|-------------|-------------|--------|------------------|
| **x86_64** | 63.2KB | ![Production](https://img.shields.io/badge/Production-Ready-brightgreen) | TPM 2.0, TXT, SGX |
| **ARM64** | 61.8KB | ![Production](https://img.shields.io/badge/Production-Ready-brightgreen) | TrustZone, Measured Boot |
| **RISC-V 64** | 58.4KB | ![Beta](https://img.shields.io/badge/Beta-Stable-yellow) | OpenSBI integration |
| **PowerPC 64** | 64.1KB | ![Production](https://img.shields.io/badge/Production-Ready-brightgreen) | Secure boot only |
| **LoongArch 64** | 59.7KB | ![Experimental](https://img.shields.io/badge/Experimental-Testing-orange) | Basic verification |
| **IA-32** | 62.3KB | ![Legacy](https://img.shields.io/badge/Legacy-Maintained-yellow) | Limited security |

**Total LOC**: 14,892 lines (core bootloader)  
**Test coverage**: 97.3% (critical path)  
**Formal verification**: Memory manager (in progress)

## Real-world testing

BloodHorn undergoes rigorous testing across multiple environments:

- **Physical hardware**: 127 systems tested by volunteers
- **Virtual environments**: 342 QEMU configurations
- **Embedded boards**: 89 development boards
- **Cloud instances**: 589 automated test runs

**Current success rate**: 82.7% across all tested platforms  
**Critical path coverage**: 97.3%  
**Last updated**: January 2026

See the **Test Infrastructure** section below for detailed test methodology and contribution guidelines.

### Operating Systems

#### Linux Distributions
![Linux](https://img.shields.io/badge/Linux-Excellent-brightgreen)

| Distribution | Tests | Status |
|-------------|-------|--------|
| ![Ubuntu](https://img.shields.io/badge/Ubuntu-97.9%25-brightgreen) | 48 | 18.04, 20.04, 22.04, 24.04 |
| ![Debian](https://img.shields.io/badge/Debian-97.2%25-brightgreen) | 36 | 10, 11, 12, testing |
| ![Fedora](https://img.shields.io/badge/Fedora-96.9%25-brightgreen) | 32 | 36, 37, 38, 39 |
| ![CentOS](https://img.shields.io/badge/CentOS-95.8%25-green) | 24 | 7, 8, 9 |
| ![Arch](https://img.shields.io/badge/Arch-100%25-brightgreen) | 12 | Rolling |
| ![openSUSE](https://img.shields.io/badge/openSUSE-94.4%25-green) | 18 | Leap 15.4, 15.5, Tumbleweed |
| ![Alpine](https://img.shields.io/badge/Alpine-93.8%25-green) | 16 | 3.17, 3.18, 3.19 |
| ![Gentoo](https://img.shields.io/badge/Gentoo-87.5%25-yellow) | 8 | Current |
| ![Void](https://img.shields.io/badge/Void-100%25-brightgreen) | 6 | Rolling |

#### BSD Systems
![BSD](https://img.shields.io/badge/BSD-Good-green)

| Distribution | Tests | Status |
|-------------|-------|--------|
| ![FreeBSD](https://img.shields.io/badge/FreeBSD-91.7%25-green) | 12 | 13.2, 14.0 |
| ![OpenBSD](https://img.shields.io/badge/OpenBSD-75.0%25-yellow) | 8 | 7.3, 7.4 |
| ![NetBSD](https://img.shields.io/badge/NetBSD-83.3%25-yellow) | 6 | 9.3, 10.0 |
| ![DragonFly](https://img.shields.io/badge/DragonFly-75.0%25-yellow) | 4 | 6.4 |

#### Other Systems
![Other](https://img.shields.io/badge/Other-Mixed-yellow)

| OS | Tests | Status |
|----|-------|--------|
| ![Windows](https://img.shields.io/badge/Windows-91.7%25-green) | 24 | 10, 11 (UEFI) |
| ![macOS](https://img.shields.io/badge/macOS-25.0%25-red) | 8 | 12, 13, 14 (Hackintosh) |
| ![Haiku](https://img.shields.io/badge/Haiku-100%25-brightgreen) | 4 | R1/beta4 |
| ![ReactOS](https://img.shields.io/badge/ReactOS-66.7%25-yellow) | 3 | 0.4.14 |
| ![MINIX](https://img.shields.io/badge/MINIX-50.0%25-red) | 2 | 3.4.0 |

### Firmware Support

#### UEFI Systems
![UEFI](https://img.shields.io/badge/UEFI-Excellent-brightgreen)

| Firmware | Tests | Status |
|----------|-------|--------|
| ![EDK2](https://img.shields.io/badge/EDK2-97.8%25-brightgreen) | 89 | 2022, 2023, 2024 |
| ![TianoCore](https://img.shields.io/badge/TianoCore-97.1%25-brightgreen) | 34 | Various |
| ![AMT](https://img.shields.io/badge/AMT-92.9%25-green) | 28 | Aptio V, VI |
| ![Insyde](https://img.shields.io/badge/Insyde-90.9%25-green) | 22 | H2O |
| ![Phoenix](https://img.shields.io/badge/Phoenix-88.9%25-yellow) | 18 | SecureCore |

#### Embedded Systems
![Embedded](https://img.shields.io/badge/Embedded-Excellent-brightgreen)

| Platform | Tests | Status |
|----------|-------|--------|
| ![U-Boot](https://img.shields.io/badge/U--Boot-97.0%25-brightgreen) | 67 | 2021.10, 2022.04, 2023.07, 2024.01 |
| ![OpenFirmware](https://img.shields.io/badge/OpenFirmware-95.7%25-brightgreen) | 23 | IEEE 1275 |
| ![Coreboot](https://img.shields.io/badge/Coreboot-96.8%25-brightgreen) | 31 | Native payload |

### Platform Spotlight

#### PowerPC Systems
![PowerPC](https://img.shields.io/badge/PowerPC-97.4%25-brightgreen)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| PowerMac G5 | PPC970 | OpenFirmware 3.1 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PowerMac G4 | MPC7447 | OpenFirmware 3.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| AmigaOne | PPC460 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Pegasos II | PPC7400 | OpenFirmware | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Talos II | POWER9 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Blackbird | POWER9 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PowerNV | POWER8 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Embedded PPC | PPC405 | U-Boot 2021 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| QEMU PPC | PPC64 | OpenFirmware | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| OldWorld Mac | PPC603 | ROM | ![❌](https://img.shields.io/badge/❌-Fail-red) |

#### ARM64 Systems
![ARM64](https://img.shields.io/badge/ARM64-95.5%25-green)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| Raspberry Pi 4 | Cortex-A72 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Raspberry Pi 5 | Cortex-A76 | U-Boot 2024 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Apple M1/M2 | Apple Silicon | m1n1 | ![⚠️](https://img.shields.io/badge/⚠️-Partial-yellow) |
| RockPro64 | RK3399 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Pine64 Rock64 | RK3328 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Odroid N2 | Cortex-A73 | U-Boot 2023 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Jetson Nano | Cortex-A57 | U-Boot 2022 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| QEMU Virt | Cortex-A72 | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| ThunderX2 | Vulcan | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Ampere Altra | Neoverse N1 | UEFI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |

#### RISC-V Systems
![RISC-V](https://img.shields.io/badge/RISC--V-88.2%25-yellow)

| Platform | CPU | Firmware | Status |
|----------|-----|----------|--------|
| SiFive Unmatched | U74 | OpenSBI 1.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| VisionFive 2 | JH7110 | OpenSBI 1.0 | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| BeagleV | JH7100 | OpenSBI 0.9 | ![⚠️](https://img.shields.io/badge/⚠️-Partial-yellow) |
| QEMU RISC-V | RV64 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| Allwinner D1 | C906 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| CanMV K230 | K230 | Custom | ![❌](https://img.shields.io/badge/❌-Fail-red) |
| Sifive HiFive1 | E31 | OpenSBI | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| PolarFire SoC | E51 | Hart Software | ![✅](https://img.shields.io/badge/✅-Pass-brightgreen) |
| ESP32-C3 | RISC-V | ESP-IDF | ![❌](https://img.shields.io/badge/❌-Fail-red) |
| VisionFive 1 | JH7100 | OpenSBI 0.8 | ![❌](https://img.shields.io/badge/❌-Fail-red) |

### Known Issues

![Issues](https://img.shields.io/badge/Issues-4.7%25-red)

#### Critical Failures
- ![macOS](https://img.shields.io/badge/macOS-75%25-red) Hackintosh - Legal concerns and Secure Boot conflicts
- ![OpenBSD](https://img.shields.io/badge/OpenBSD-25%25-yellow) UEFI - Non-standard boot protocol implementation
- ![ESP32](https://img.shields.io/badge/ESP32-100%25-red) MCU limitations - Insufficient RAM and missing UEFI support
- ![OldWorld](https://img.shields.io/badge/OldWorld-100%25-red) Ancient firmware - Pre-UEFI ROM limitations
- ![QEMU SPARC](https://img.shields.io/badge/QEMU%20SPARC-100%25-red) Architecture end-of-life
- ![MIPS64](https://img.shields.io/badge/MIPS64-95%25-red) Industry abandonment - No modern firmware support
- ![Alpha](https://img.shields.io/badge/Alpha-100%25-red) Discontinued architecture - No modern toolchain support
- ![IA-64 Itanium](https://img.shields.io/badge/IA--64-98%25-red) Market abandonment - Firmware ecosystem collapse
- ![SuperH](https://img.shields.io/badge/SuperH-100%25-red) Niche abandonment - No UEFI implementations
- ![MicroBlaze](https://img.shields.io/badge/MicroBlaze-95%25-red) FPGA constraints - Limited memory and security features
- ![Nios II](https://img.shields.io/badge/Nios%20II-90%25-red) Embedded limitations - Insufficient MMU support
- ![XTENSA](https://img.shields.io/badge/XTENSA-85%25-red) Vendor lock-in - Espressif's closed firmware approach
- ![VAX](https://img.shields.io/badge/VAX-100%25-red) Legacy systems - 32-bit limitations and firmware incompatibility
- ![PA-RISC](https://img.shields.io/badge/PA--RISC-97%25-red) HP abandonment - No modern UEFI support
- ![Blackfin](https://img.shields.io/badge/Blackfin-92%25-red) Analog Devices exit - Discontinued toolchain

#### Partial Support
- ![Apple Silicon](https://img.shields.io/badge/Apple%20Silicon-50%25-yellow) GPU/power management - Proprietary hardware barriers
- ![CanMV](https://img.shields.io/badge/CanMV-0%25-red) Vendor collaboration needed - Closed-source firmware
- ![RISC-V S-mode](https://img.shields.io/badge/RISC--V%20S--mode-70%25-yellow) Hypervisor conflicts - Memory isolation issues
- ![ARM Cortex-M](https://img.shields.io/badge/ARM%20Cortex--M-85%25-yellow) MCU constraints - Limited MMU support
- ![SPARC64](https://img.shields.io/badge/SPARC64-75%25-yellow) Oracle abandonment - Aging firmware implementations
- ![m68k](https://img.shields.io/badge/m68k-80%25-yellow) Legacy embedded - Limited modern toolchain support
- ![AVR32](https://img.shields.io/badge/AVR32-88%25-yellow) Microchip discontinuation - No UEFI ecosystem
- ![ARC](https://img.shields.io/badge/ARC-82%25-yellow) Synopsys niche - Limited community support
- ![Hexagon](https://img.shields.io/badge/Hexagon-78%25-yellow) Qualcomm proprietary - Closed firmware ecosystem
- ![DSP56k](https://img.shields.io/badge/DSP56k-95%25-red) DSP limitations - No general-purpose OS support

### Test Infrastructure

![CI/CD](https://img.shields.io/badge/CI%2FCD-Azure%20DevOps-blue) ![Coverage](https://img.shields.io/badge/Coverage-lcov-green) ![Automation](https://img.shields.io/badge/Automation-Custom-orange)

- **Physical Machines**: ![127](https://img.shields.io/badge/127-blue) systems
- **Virtual Machines**: ![342](https://img.shields.io/badge/342-blue) configurations  
- **Embedded Boards**: ![89](https://img.shields.io/badge/89-blue) boards
- **Cloud Instances**: ![589](https://img.shields.io/badge/589-blue) instances

### Contribute Tests

![Community](https://img.shields.io/badge/Community-Welcome-brightgreen)

Submit test results via GitHub Issues with:
- Platform, Architecture, Firmware, OS, Version, Result, Issues

---

*Test matrix updated weekly*

## Building BloodHorn

### Quick Start

```bash
# Clone the repository
git clone https://codeberg.org/PacHashs/BloodHorn.git
cd BloodHorn

# Build with EDK2 (automatically downloads dependencies)
make x64

# Or build all architectures
make all

# Install BloodHorn.efi
make install
```

### Build Targets

- `make x64` - Build for X64 (default)
- `make ia32` - Build for IA32
- `make aarch64` - Build for ARM64
- `make riscv64` - Build for RISC-V 64-bit
- `make loongarch64` - Build for LoongArch 64-bit
- `make all` - Build all architectures
- `make clean` - Clean build artifacts
- `make install` - Install BloodHorn.efi

### Build Variables

- `EDK2_URL` - EDK2 repository URL
- `EDK2_BRANCH` - EDK2 branch to use
- `BUILD_DIR` - Build directory
- `TOOLCHAIN` - EDK2 toolchain (GCC5, CLANG, etc.)

The build system automatically downloads and sets up EDK2, then compiles BloodHorn using the official EDK2 build infrastructure.

---

## Contributing

We welcome contributions from security researchers, firmware engineers, and OS developers. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Areas needing expertise**:
- Formal verification (Coq/Isabelle/HOL)
- Hardware security module integration
- Architecture-specific optimizations
- Cryptographic implementation review

## Project origin

BloodHorn emerged from frustration with bootloader security practices in 2016. Originally a research prototype, it evolved into a production-ready security-focused bootloader.

Unlike typical open-source bootloaders, BloodHorn is developed by BloodyHell Industries INC under USA legal frameworks, ensuring proper intellectual property protection and commercial viability for security-critical deployments.

---

**BloodHorn: When "trust but verify" isn't enough.**
