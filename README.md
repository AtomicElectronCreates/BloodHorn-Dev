# BloodHorn
<p align="center">
  <img src="Zeak.png" alt="Project Logo" width="200">
</p>

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![EDK2 Compatible](https://img.shields.io/badge/EDK2-Compatible-blue)](https://github.com/tianocore/tianocore.github.io/)
[![UEFI](https://img.shields.io/badge/UEFI-Secure%20Boot-0078D7)](https://uefi.org/)
[![Architectures](https://img.shields.io/badge/Arch-x86__64%20%7C%20ARM64%20%7C%20RISC--V-0078D7)](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface)
[![Build Status](https://github.com/Listedroot/BloodHorn/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Listedroot/BloodHorn/actions)

BloodHorn is a modern bootloader supporting Linux, Multiboot, Limine, PXE, and multiple CPU architectures.

---
## Build Requirements

- GCC or Clang
- NASM
- EDK2 (for UEFI builds)
- Python 3.6+

## Security Notice
**BloodHorn is a security-sensitive project. Always build from source and review code before use. Never trust pre-built binaries for bootloaders.**

## Quick Build

### Prerequisites (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential uuid-dev nasm acpica-tools \
    python3-full qemu-system-x86 ovmf git gcc-aarch64-linux-gnu \
    gcc-riscv64-linux-gnu
```

### Building with EDK2
```bash
# Clone EDK2
git clone --depth 1 https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init

# Build base tools
make -C BaseTools

# Set up environment
export EDK_TOOLS_PATH=$(pwd)/BaseTools
export PACKAGES_PATH=$(pwd):$(pwd)/..
. edksetup.sh

# Build BloodHorn (from the repository root)
cp -r ../BloodHorn .
build -a X64 -p BloodHorn.dsc -t GCC5

# Run in QEMU
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom BloodHorn.iso
```


## Features

- **Multi-Architecture Support**: x86_64, ARM64, RISC-V, and more
- **Multiple Boot Protocols**: Linux, Multiboot2, Limine, PXE
- **Modern C++ Networking Stack**:
  - IPv4 networking with ARP and DHCP
  - PXE boot support
  - TFTP file transfers
  - Secure boot verification
- **UEFI Secure Boot** compatible
- **Modular Design**: Easy to extend and customize
- **Interactive Boot Menu**: GUI and text mode interfaces

## Networking Features

BloodHorn includes a modern C++ networking stack for network booting and remote management:

```cpp
// Example: Network boot a kernel
#include "net/network.hpp"

void boot_from_network() {
    using namespace BloodHorn::Net;
    
    // Create and initialize network interface
    auto iface = NetworkInterface::create();
    PXEClient client(std::move(iface));
    
    // Discover network configuration
    if (auto ec = client.discoverNetwork()) {
        // Handle error
        return;
    }
    
    // Download and boot a kernel
    client.bootKernel(
        "/boot/kernel.efi", 
        "/boot/initrd.img", 
        "console=ttyS0"
    );
}
```

### Supported Network Operations

- **Address Management**:
  - IPv4 address parsing and formatting
  - MAC address handling
  - Network configuration (IP, netmask, gateway, DNS)

- **Protocols**:
  - ARP for address resolution
  - DHCP for automatic configuration
  - TFTP for file transfers
  - PXE for network booting

- **Security**:
  - Secure boot verification
  - File integrity checking with SHA-512
  - TPM 2.0 support for secure measurements

## Building with Networking Support

To enable networking features, ensure you have the required dependencies:

```bash
# Install additional networking dependencies
sudo apt install -y libsodium-dev libssl-dev

# Build with networking support
build -p BloodHorn/BloodHorn.dsc -a X64 -t GCC5 -D NETWORK_ENABLE=1
```

## Configuration

Network settings can be configured in `boot/config.ini`:

```ini
[network]
enable_networking = true
use_dhcp = true
static_ip = 192.168.1.100
netmask = 255.255.255.0
gateway = 192.168.1.1
dns_server = 8.8.8.8
```

## Security Considerations

- Always verify the integrity of network-loaded kernels and modules
- Use secure boot to prevent unauthorized code execution
- Consider using TPM measurements for remote attestation
- Validate all network responses and implement timeouts

## Architectures

- IA-32 (x86)
- x86-64
- aarch64 (arm64)
- riscv64
- loongarch64


## Boot Protocols

- Linux
- Multiboot 1
- Multiboot 2
- [Limine](https://github.com/limine-bootloader/limine-protocol/blob/trunk/PROTOCOL.md)
- [BloodChain](docs/BloodChain-Protocol.md) 
- Chainloading
- PXE


## Configuration

- INI, JSON, and environment variable support
- Theme and language options (see CONFIG.md)
- Place config files in the root of the boot partition



## Why We Don't Provide Binaries

BloodHorn does not release pre-built binaries. This is to ensure maximum security, transparency, and trust:

- Building from source guarantees you know exactly what code is running on your system.
- Bootloaders are highly security-sensitive; you should always verify and build your own binary.
- Different systems and firmware may require different build options or configurations.
- This approach encourages review, customization, and community contributions.

## ✨ Contributors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/PacHashs">
        <img src="https://avatars.githubusercontent.com/PacHashs" width="120px;" alt="My avatar"/>
        <br />
        <sub><b>PacHash</b></sub>
      </a>
      <br />
      Lead Developer 
    </td>
    <td align="center">
      <a href="https://github.com/unpopularopinionn">
        <img src="https://avatars.githubusercontent.com/unpopularopinionn" width="120px;" alt="Homie avatar"/>
        <br />
        <sub><b>UnpopularOpinion</b></sub>
      </a>
      <br />
      Main Team
    </td>
  </tr>
</table>


See INSTALL.md and USAGE.md for build and usage instructions.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

#### *BloodHorn was inspired by modern bootloaders, but all code is original and written from scratch and it's made originally for fun and to be used in my future operating systems!*
