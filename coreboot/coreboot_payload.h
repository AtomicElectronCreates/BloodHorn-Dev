/*
 * coreboot_payload.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef COREBOOT_PAYLOAD_H
#define COREBOOT_PAYLOAD_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>

// Coreboot version and build information
#define COREBOOT_VERSION "1.0.0"
#define COREBOOT_BUILD_DATE __DATE__ " " __TIME__

// Coreboot payload entry point signature
typedef VOID (*COREBOOT_PAYLOAD_ENTRY)(VOID* coreboot_table, VOID* payload);

// Coreboot boot parameter structure definitions
#define COREBOOT_BOOT_SIGNATURE 0x12345678
#define COREBOOT_BOOT_FLAG_KERNEL 0x01
#define COREBOOT_BOOT_FLAG_FRAMEBUFFER 0x02
#define COREBOOT_BOOT_FLAG_ACPI 0x04
#define COREBOOT_BOOT_FLAG_SMBIOS 0x08
#define COREBOOT_BOOT_FLAG_SECURE_BOOT 0x10
#define COREBOOT_BOOT_FLAG_TPM_MEASUREMENT 0x20

typedef struct {
    UINT32 signature;           // Boot signature
    UINT32 version;             // Boot parameters version
    UINT64 kernel_base;         // Kernel base address
    UINT64 kernel_size;         // Kernel size in bytes
    UINT32 boot_flags;          // Boot flags
    UINT64 reserved1;          // Reserved for alignment

    // Framebuffer information
    UINT64 framebuffer_addr;    // Framebuffer physical address
    UINT32 framebuffer_width;   // Framebuffer width
    UINT32 framebuffer_height;  // Framebuffer height
    UINT32 framebuffer_bpp;     // Bits per pixel
    UINT32 framebuffer_pitch;   // Bytes per line
    UINT32 framebuffer_red_mask;     // Red mask position/size
    UINT32 framebuffer_green_mask;   // Green mask position/size
    UINT32 framebuffer_blue_mask;    // Blue mask position/size

    // ACPI information
    UINT64 acpi_rsdp;           // ACPI RSDP address
    UINT64 acpi_rsdt;           // ACPI RSDT address

    // SMBIOS information
    UINT64 smbios_entry;        // SMBIOS entry point address

    // Memory information
    UINT64 memory_size;         // Total memory size
    UINT64 memory_map_addr;     // Memory map address
    UINT32 memory_map_entries;  // Number of memory map entries

    // Coreboot table information
    UINT64 coreboot_table_addr; // Coreboot table address
    UINT32 coreboot_version;    // Coreboot version

    // System information
    UINT32 cpu_count;           // Number of CPUs
    UINT64 cpu_frequency;       // CPU frequency in Hz
    UINT32 board_id;            // Board ID
    UINT64 timestamp;           // Boot timestamp

    // Security information
    UINT32 secure_boot_enabled; // Secure boot enabled flag
    UINT64 tpm_pcr_values[24];  // TPM PCR values

    // Reserved for future use
    UINT64 reserved[8];
} COREBOOT_BOOT_PARAMS;

/**
 * Coreboot payload header structure
 */
typedef struct {
    UINT8 signature[8];      // "$COREBOOT"
    UINT32 header_version;   // Header format version
    UINT16 payload_version;  // Payload version
    UINT16 payload_size;     // Size of payload in 512-byte blocks
    UINT32 cmd_line_size;    // Size of command line
    UINT32 checksum;         // Checksum of header
    UINT32 entry_point;      // Offset to entry point from start of header
    UINT32 payload_load_addr; // Load address for payload
    UINT32 payload_compressed_size; // Compressed size if compressed
    UINT32 payload_uncompressed_size; // Uncompressed size
    UINT32 payload_type;     // Payload type (0=elf, 1=raw, 2=compressed)
    UINT32 payload_arch;     // Payload architecture (x86=1, arm=2, etc)
    UINT32 reserved[2];      // Reserved for future use
    CHAR8 cmd_line[0];       // Command line (null-terminated)
} __attribute__((packed)) COREBOOT_PAYLOAD_HEADER;

// Payload types
#define COREBOOT_PAYLOAD_TYPE_ELF 0
#define COREBOOT_PAYLOAD_TYPE_RAW 1
#define COREBOOT_PAYLOAD_TYPE_COMPRESSED 2
#define COREBOOT_PAYLOAD_TYPE_FIT 3

// Payload architectures
#define COREBOOT_PAYLOAD_ARCH_X86 1
#define COREBOOT_PAYLOAD_ARCH_X86_64 2
#define COREBOOT_PAYLOAD_ARCH_ARM 3
#define COREBOOT_PAYLOAD_ARCH_ARM64 4
#define COREBOOT_PAYLOAD_ARCH_RISCV 5
#define COREBOOT_PAYLOAD_ARCH_RISCV64 6

// Constants
#define COREBOOT_PAYLOAD_SIGNATURE "$COREBOOT"
#define PAYLOAD_HEADER_VERSION 2
#define MAX_KERNEL_PATH 256
#define MAX_CMDLINE_SIZE 1024
#define MAX_BOOT_ENTRIES 16

// Boot entry structure
typedef struct {
    CHAR16 title[64];         // Boot entry title
    CHAR16 path[MAX_KERNEL_PATH]; // Kernel path
    CHAR16 cmdline[MAX_CMDLINE_SIZE]; // Command line arguments
    UINT32 entry_type;        // Entry type (linux, multiboot, etc)
    UINT32 boot_flags;        // Boot-specific flags
    UINT64 timeout;           // Boot timeout in seconds
    BOOLEAN is_default;       // Is this the default entry
} COREBOOT_BOOT_ENTRY;

// Boot entry types
#define COREBOOT_BOOT_TYPE_LINUX 1
#define COREBOOT_BOOT_TYPE_MULTIBOOT 2
#define COREBOOT_BOOT_TYPE_MULTIBOOT2 3
#define COREBOOT_BOOT_TYPE_LIMINE 4
#define COREBOOT_BOOT_TYPE_CHAINLOAD 5
#define COREBOOT_BOOT_TYPE_PXE 6
#define COREBOOT_BOOT_TYPE_BLOODCHAIN 7
#define COREBOOT_BOOT_TYPE_RECOVERY 8
#define COREBOOT_BOOT_TYPE_REBOOT 9

// Forward declarations for Coreboot platform functions
BOOLEAN EFIAPI CorebootPlatformInit(VOID);
CONST COREBOOT_MEM_ENTRY* EFIAPI CorebootGetMemoryMap(OUT UINT32* Count);
CONST COREBOOT_FB* EFIAPI CorebootGetFramebuffer(VOID);
BOOLEAN EFIAPI CorebootInitGraphics(VOID);
EFI_STATUS EFIAPI CorebootInitStorage(VOID);
EFI_STATUS EFIAPI CorebootInitNetwork(VOID);
EFI_STATUS EFIAPI CorebootInitTpm(VOID);
VOID EFIAPI CorebootReboot(VOID);
VOID EFIAPI CorebootPrintSystemInfo(VOID);
UINT64 EFIAPI CorebootGetTotalMemory(VOID);

// Forward declarations for memory entries and framebuffer
typedef struct {
    UINT64 addr;
    UINT64 size;
    UINT32 type;
} COREBOOT_MEM_ENTRY;

typedef struct {
    UINT64 physical_address;
    UINT32 x_resolution;
    UINT32 y_resolution;
    UINT32 bytes_per_line;
    UINT8 bits_per_pixel;
    UINT8 red_mask_pos;
    UINT8 red_mask_size;
    UINT8 green_mask_pos;
    UINT8 green_mask_size;
    UINT8 blue_mask_pos;
    UINT8 blue_mask_size;
    UINT8 reserved_mask_pos;
    UINT8 reserved_mask_size;
} COREBOOT_FB;

// Memory types
#define CB_MEM_RAM       1
#define CB_MEM_RESERVED  2
#define CB_MEM_ACPI      3
#define CB_MEM_NVS       4
#define CB_MEM_UNUSABLE  5
#define CB_MEM_DISABLED  6

// Payload entry points
VOID EFIAPI BloodhornPayloadEntry(IN VOID* coreboot_table, IN VOID* payload);
VOID EFIAPI BloodhornMain(VOID);

// Initialization functions
EFI_STATUS EFIAPI InitializeUefiEnvironment(VOID);
EFI_STATUS EFIAPI InitializeHardwareAbstraction(VOID);
EFI_STATUS EFIAPI InitializeSecuritySubsystem(VOID);
EFI_STATUS EFIAPI InitializeConsole(VOID);

// Boot parameter setup
EFI_STATUS EFIAPI SetupBootParameters(OUT COREBOOT_BOOT_PARAMS* boot_params);
EFI_STATUS EFIAPI LoadBootConfiguration(VOID);
VOID EFIAPI UseDefaultBootConfiguration(VOID);

// Boot menu and kernel loading functions
VOID EFIAPI BloodhornBootMenu(VOID);
BOOLEAN EFIAPI BloodhornLoadKernel(IN CONST CHAR8* kernel_path, OUT VOID** kernel_buffer, OUT UINT32* kernel_size);
BOOLEAN EFIAPI BloodhornExecuteKernel(IN VOID* kernel_buffer, IN UINT32 kernel_size);

// Boot function implementations
EFI_STATUS EFIAPI BloodhornBootLinux(VOID);
EFI_STATUS EFIAPI BloodhornBootMultiboot2(VOID);
EFI_STATUS EFIAPI BloodhornBootLimine(VOID);
EFI_STATUS EFIAPI BloodhornBootChainload(VOID);
EFI_STATUS EFIAPI BloodhornBootPxe(VOID);
EFI_STATUS EFIAPI BloodhornBootBloodchain(VOID);
EFI_STATUS EFIAPI BloodhornBootRecovery(VOID);
EFI_STATUS EFIAPI BloodhornReboot(VOID);

// Payload creation and validation
VOID EFIAPI CreateBloodhornPayload(IN CONST CHAR8* output_file, IN CONST CHAR8* input_binary, IN CONST CHAR8* cmdline);
BOOLEAN EFIAPI ValidatePayloadHeader(IN CONST COREBOOT_PAYLOAD_HEADER* header);
VOID EFIAPI GetPayloadInfo(IN CONST COREBOOT_PAYLOAD_HEADER* header);

// Boot parameter validation
STATIC BOOLEAN EFIAPI ValidateBootParameters(IN COREBOOT_BOOT_PARAMS* boot_params);

// Utility functions
EFI_STATUS EFIAPI get_root_dir(OUT EFI_FILE_HANDLE* root_dir);

#endif // COREBOOT_PAYLOAD_H
