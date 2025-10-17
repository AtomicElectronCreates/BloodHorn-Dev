# BloodHorn Coreboot Integration Documentation

## Overview

BloodHorn bootloader has been enhanced with Coreboot firmware platform integration, providing a more flexible and lightweight boot environment compared to traditional UEFI/EDK2 firmware.

## Why Coreboot Integration?

**Even though EDK2/UEFI was "sufficient", BloodHorn added Coreboot support for fundamental reasons:**

### **1. Hardware Control Philosophy**
- **EDK2 Limitation**: UEFI is an abstraction layer that adds complexity between the bootloader and hardware
- **Coreboot Advantage**: Direct hardware initialization provides better control and understanding of the boot process
- **Educational Value**: Understanding low-level hardware initialization is essential for bootloader development

### **2. Performance Benefits**
- **Faster Boot Times**: Coreboot eliminates UEFI overhead during early boot stages
- **Reduced Memory Usage**: Smaller firmware footprint leaves more RAM for the OS
- **Better Hardware Utilization**: Direct hardware control enables optimized initialization

### **3. Flexibility and Customization**
- **EDK2 Rigidity**: UEFI specifications limit customization options
- **Coreboot Freedom**: Source code access allows tailoring firmware to specific needs
- **Mainboard Optimization**: Can optimize for specific hardware configurations

### **4. Security Model**
- **Hardware Root of Trust**: Coreboot enables stronger security foundations
- **Verified Boot**: Better integration with hardware security features
- **TPM Integration**: More direct access to TPM functionality

### **5. Future-Proofing**
- **Beyond UEFI**: Prepares for firmware ecosystems beyond UEFI
- **Research Platform**: Enables experimentation with new boot technologies
- **Cross-Platform**: Better support for non-x86 architectures

## What Coreboot Brings to BloodHorn

### **Hardware Services Handled by Coreboot:**
- CPU initialization and configuration
- Memory controller and mapping
- PCI/PCIe bus enumeration and initialization
- Storage controller support (SATA/AHCI/NVMe)
- USB host controller initialization
- Network interface initialization
- Graphics framebuffer setup
- TPM initialization and configuration
- ACPI table generation
- SMBIOS table generation

### **Bootloader Services Retained:**
- Boot protocol support (Linux, Multiboot, Limine, etc.)
- Filesystem operations for kernel loading
- Boot menu and user interface
- Configuration file parsing
- Lua scripting support
- Plugin system
- Recovery shell
- Security verification of loaded components

## Hybrid Architecture Benefits

### **Best of Both Worlds**
```c
// Automatic detection and hybrid initialization
if (CorebootAvailable) {
    // Use Coreboot for hardware initialization
    CorebootInitGraphics();
    CorebootInitStorage();
    CorebootInitNetwork();
    // Use UEFI for higher-level services
} else {
    // Pure UEFI mode for compatibility
}
```

### **Performance Improvements**
- **Faster Hardware Detection**: Direct hardware probing vs. UEFI abstraction
- **Reduced Boot Time**: Less firmware overhead in boot chain
- **Better Memory Management**: More efficient memory mapping

### **Enhanced Compatibility**
- **Universal Support**: Works with both Coreboot and UEFI firmware
- **Hardware Agnostic**: Supports wide range of mainboards
- **Protocol Preservation**: Maintains all existing boot protocols

## Migration Benefits

### **For Users**
- **Faster Boot**: Noticeably quicker system startup
- **Better Hardware Support**: Access to latest hardware features
- **Enhanced Security**: Stronger boot integrity verification

### **For Developers**
- **Learning Platform**: Understanding of firmware internals
- **Customization**: Ability to modify firmware behavior
- **Research**: Platform for boot technology experimentation

## Implementation Strategy

### **Hybrid Approach**
BloodHorn implements a **hybrid architecture** that:
1. **Detects firmware type** at boot time
2. **Uses Coreboot** for hardware services when available
3. **Falls back to UEFI** for maximum compatibility
4. **Maintains API compatibility** across firmware types

### **Code Organization**
- **Coreboot modules** handle firmware-specific operations
- **Common bootloader logic** works with both firmware types
- **Automatic adaptation** based on available services

## Conclusion

Coreboot integration transforms BloodHorn from a UEFI-only bootloader into a **universal firmware-compatible** boot platform. While EDK2 was "sufficient" for basic functionality, Coreboot integration provides:

- **Superior performance** through direct hardware control
- **Enhanced flexibility** for customization and research
- **Future-proof architecture** for evolving firmware ecosystems
- **Educational value** for understanding boot process internals

This integration positions BloodHorn as a **modern, adaptable bootloader** suitable for both traditional UEFI systems and advanced Coreboot firmware environments, providing the best of both worlds in a single, production-ready solution.

## What is Coreboot?

Coreboot is an open-source firmware platform that replaces the traditional BIOS/UEFI firmware on x86 systems. It provides:

- **Hardware Initialization**: Direct hardware control and initialization
- **Memory Management**: Efficient memory mapping and allocation
- **Device Support**: Native support for storage, network, graphics, and other devices
- **Security**: Built-in TPM and Secure Boot support
- **Performance**: Faster boot times and reduced firmware footprint

## BloodHorn Coreboot Integration

### Architecture Changes

The integration replaces UEFI/EDK2 services with Coreboot platform services:

#### Hardware Services Handled by Coreboot:
- CPU initialization and configuration
- Memory controller and mapping
- PCI/PCIe bus enumeration and initialization
- Storage controller support (SATA/AHCI/NVMe)
- USB host controller initialization
- Network interface initialization
- Graphics framebuffer setup
- TPM initialization and configuration
- ACPI table generation
- SMBIOS table generation

#### Bootloader Services Retained:
- Boot protocol support (Linux, Multiboot, Limine, etc.)
- Filesystem operations for kernel loading
- Boot menu and user interface
- Configuration file parsing
- Lua scripting support
- Plugin system
- Recovery shell
- Security verification of loaded components

### Coreboot Platform Module

The `coreboot/` directory contains:

- **`coreboot_platform.c`**: Coreboot platform initialization and interface
- **`coreboot_platform.h`**: Coreboot platform API definitions
- **`coreboot_payload.c`**: Coreboot payload interface implementation
- **`coreboot_payload.h`**: Payload interface definitions
- **`build_coreboot.sh`**: Build script for Coreboot integration
- **`coreboot_config.mk`**: Coreboot build configuration
- **`coreboot_defconfig`**: Coreboot configuration file

### Key Features

#### Memory Management
```c
// Get total available RAM
UINT64 total_memory = CorebootGetTotalMemory();

// Find largest RAM region for kernel loading
UINT64 largest_addr, largest_size;
if (CorebootFindLargestMemoryRegion(&largest_addr, &largest_size)) {
    // Use largest memory region for kernel
}
```

#### Graphics Support
```c
// Initialize graphics using Coreboot framebuffer
if (CorebootInitGraphics()) {
    // Graphics available - get framebuffer info
    UINT32 width, height, bpp;
    CorebootGetFramebufferInfo(&width, &height, &bpp);

    VOID* framebuffer = CorebootGetFramebufferAddress();
    // Use framebuffer for boot menu display
}
```

#### Hardware Information
```c
// Get Coreboot system information
CONST COREBOOT_SYSINFO* sysinfo = CorebootGetSysinfo();
if (sysinfo->version) {
    Print(L"Coreboot version: %a\n", sysinfo->version);
}

// Check if running under Coreboot
if (CorebootIsPresent()) {
    // Coreboot firmware detected
}
```

## Building BloodHorn with Coreboot Support

### Prerequisites

1. **EDK2 Development Environment**: Standard EDK2 build tools
2. **Coreboot Source**: Clone Coreboot repository
3. **Cross-compilation Tools**: GCC for target architecture

### Build Process

1. **Setup Coreboot**:
   ```bash
   cd coreboot/
   git clone https://github.com/coreboot/coreboot.git
   cd coreboot
   git checkout 4.22  # Use stable version
   ```

2. **Configure Coreboot**:
   ```bash
   cd coreboot/
   cp ../coreboot_defconfig .config
   make oldconfig
   ```

3. **Build BloodHorn**:
   ```bash
   cd ../
   build -p BloodHorn/BloodHorn.dsc -a X64 -b RELEASE
   ```

4. **Create Payload**:
   ```bash
   cd coreboot/
   ./build_coreboot.sh
   ```

### Coreboot Configuration

The `coreboot_defconfig` file configures Coreboot for BloodHorn:

```makefile
# Mainboard configuration
CONFIG_MAINBOARD_DIR="mainboard/google/octopus"

# BloodHorn payload configuration
CONFIG_PAYLOAD_ELF=y
CONFIG_PAYLOAD_FILE="../build/bloodhorn.elf"
CONFIG_PAYLOAD_CMDLINE="console=ttyS0 root=/dev/sda1 ro quiet splash"

# Hardware support
CONFIG_USB=y
CONFIG_SATA=y
CONFIG_NVME=y
CONFIG_PCIEXP=y

# Graphics support
CONFIG_FRAMEBUFFER_KEEP_VESA_MODE=n
CONFIG_MAINBOARD_HAS_NATIVE_VGA_INIT=y

# Security features
CONFIG_TPM=y
CONFIG_VBOOT=y
CONFIG_VBOOT_VERIFY_FIRMWARE=y
```

## Running BloodHorn as Coreboot Payload

### Payload Interface

BloodHorn implements the standard Coreboot payload interface:

```c
VOID EFIAPI BloodhornPayloadEntry(
  IN VOID* coreboot_table,  // Coreboot table pointer
  IN VOID* payload         // Payload address
);
```

The payload entry point:
1. Initializes Coreboot platform interface
2. Sets up graphics, storage, and network devices
3. Displays boot menu
4. Loads and executes selected kernel

### Memory Layout

Coreboot provides a memory map that BloodHorn uses for:
- Kernel loading in available RAM regions
- Framebuffer access for graphics
- Stack and heap allocation
- Device memory mapping

## Security Integration

### TPM Support
Coreboot handles TPM initialization, BloodHorn uses it for:
- Boot component measurement
- Secure boot verification
- Key management

### Secure Boot
Coreboot provides firmware-based secure boot, BloodHorn adds:
- Kernel signature verification
- Filesystem integrity checking
- Measured boot support

## Platform Compatibility

### Supported Hardware
- **Mainboards**: Any Coreboot-supported x86 mainboard
- **CPUs**: x86_64 and compatible processors
- **Memory**: DDR3/DDR4 support via Coreboot
- **Storage**: SATA/AHCI, NVMe via Coreboot drivers
- **Graphics**: Native VGA, GOP support
- **Network**: Ethernet controllers supported by Coreboot

### Boot Protocols
All existing boot protocols continue to work:
- Linux kernel (bzImage, EFI stub)
- Multiboot/Multiboot2
- Limine protocol
- Chainloading (GRUB, other bootloaders)
- PXE network boot
- BloodChain protocol

## Troubleshooting

### Common Issues

1. **Coreboot Table Not Found**
   - Verify Coreboot firmware is properly installed
   - Check mainboard compatibility
   - Ensure payload is correctly built

2. **Graphics Issues**
   - Verify framebuffer configuration in Coreboot
   - Check GOP support for target hardware
   - Ensure correct video mode selection

3. **Storage Detection**
   - Verify storage controller support in Coreboot
   - Check device enumeration
   - Confirm filesystem support

### Debug Information

Enable debug mode for detailed logging:

```makefile
CONFIG_COLLECT_TIMESTAMPS=y
CONFIG_DEBUG_COREBOOT=y
```

## Migration from UEFI

### Key Differences

| Feature | UEFI/EDK2 | Coreboot |
|---------|-----------|----------|
| Hardware Init | UEFI Services | Coreboot Direct |
| Graphics | GOP Protocol | Framebuffer Direct |
| Memory | UEFI Memory Map | Coreboot Memory Map |
| Storage | UEFI FS Protocol | Coreboot Block Device |
| Network | UEFI Network Stack | Coreboot Network Driver |
| Security | UEFI Auth Variables | Coreboot TPM/Secure Boot |

### Code Changes Required

1. **Replace UEFI Protocol Usage**:
   ```c
   // Old UEFI approach
   gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, &Gop);

   // New Coreboot approach
   VOID* framebuffer = CorebootGetFramebufferAddress();
   ```

2. **Update Memory Management**:
   ```c
   // Use Coreboot memory map instead of UEFI services
   CONST COREBOOT_MEM_ENTRY* mem_map = CorebootGetMemoryMap(&count);
   ```

3. **Adapt Device Access**:
   ```c
   // Use Coreboot device interfaces
   if (CorebootInitStorage()) {
       // Access storage devices directly
   }
   ```

## Performance Benefits

- **Faster Boot Times**: Reduced firmware initialization overhead
- **Lower Memory Usage**: Smaller firmware footprint
- **Better Hardware Control**: Direct hardware access
- **Simplified Architecture**: Less abstraction layers

## Future Enhancements

- **ARM64 Support**: Extend to ARM64 mainboards
- **Advanced Graphics**: Native graphics driver integration
- **Enhanced Security**: Hardware-based root of trust
- **Network Boot**: Improved PXE implementation
- **Plugin Architecture**: Extended plugin system for Coreboot

## Support and Development

For issues specific to Coreboot integration:
1. Verify Coreboot firmware compatibility
2. Check BloodHorn build configuration
3. Review hardware-specific requirements
4. Consult Coreboot documentation for mainboard support

This integration makes BloodHorn a modern, efficient bootloader suitable for both traditional UEFI systems and Coreboot-based firmware environments.
