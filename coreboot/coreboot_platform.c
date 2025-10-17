/**
 * Coreboot Platform Integration for BloodHorn Bootloader
 *
 * This module provides Coreboot firmware platform support, replacing
 * UEFI/EDK2 initialization with native Coreboot firmware services.
 *
 * Coreboot handles:
 * - Hardware initialization (CPU, memory, chipset)
 * - Device discovery and basic initialization
 * - Memory management and mapping
 * - ACPI/SMBIOS table generation
 * - Graphics framebuffer initialization
 * - PCI/USB/Storage/Network initialization
 * - TPM and Secure Boot support
 *
 * BloodHorn bootloader focuses on:
 * - Boot protocol support (Linux, Multiboot, etc.)
 * - Filesystem operations for loading kernels
 * - Boot menu and configuration
 * - High-level boot logic
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>

// Coreboot firmware table structures
#define COREBOOT_TABLE_MAGIC 0x43425442  // "CBTB"

typedef struct {
    UINT32 tag;
    UINT32 size;
} COREBOOT_TABLE_ENTRY;

typedef struct {
    UINT32 magic;
    UINT32 header_bytes;
    UINT32 header_checksum;
    UINT32 table_bytes;
    UINT32 table_checksum;
    COREBOOT_TABLE_ENTRY entries[0];
} COREBOOT_TABLE_HEADER;

// Coreboot table tags
#define CB_TAG_MEMORY     0x0001
#define CB_TAG_HWRPB      0x0002
#define CB_TAG_MAINBOARD  0x0003
#define CB_TAG_VERSION    0x0004
#define CB_TAG_EXTRA_VERSION 0x0005
#define CB_TAG_BUILD      0x0006
#define CB_TAG_COMPILE_TIME 0x0007
#define CB_TAG_COMPILER   0x0008
#define CB_TAG_LINKER     0x0009
#define CB_TAG_ASSEMBLER  0x000a
#define CB_TAG_FRAMEBUFFER 0x0012
#define CB_TAG_SERIAL     0x0020
#define CB_TAG_CONSOLE    0x0021
#define CB_TAG_FORWARD    0x0022
#define CB_TAG_LINK       0x0023
#define CB_TAG_CBMEM_CONSOLE 0x0024
#define CB_TAG_MRC_CACHE  0x0025
#define CB_TAG_VBNV       0x0026
#define CB_TAG_VBOOT_WORKBUF 0x0027
#define CB_TAG_DMA       0x0030
#define CB_TAG_BOARD_ID   0x0040

// Memory map entry structure
typedef struct {
    UINT64 addr;
    UINT64 size;
    UINT32 type;
} COREBOOT_MEM_ENTRY;

// Memory types
#define CB_MEM_RAM       1
#define CB_MEM_RESERVED  2
#define CB_MEM_ACPI      3
#define CB_MEM_NVS       4
#define CB_MEM_UNUSABLE  5
#define CB_MEM_DISABLED  6

// Framebuffer structure
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

// Global Coreboot platform state
STATIC COREBOOT_TABLE_HEADER* cb_header = NULL;
STATIC COREBOOT_FB* cb_framebuffer = NULL;
STATIC COREBOOT_MEM_ENTRY* cb_memory_map = NULL;
STATIC UINT32 cb_memory_map_entries = 0;

// Coreboot system information
typedef struct {
    CHAR8* version;
    CHAR8* extra_version;
    CHAR8* build;
    CHAR8* compile_time;
    CHAR8* compiler;
    CHAR8* assembler;
    UINT16 board_id;
} COREBOOT_SYSINFO;

STATIC COREBOOT_SYSINFO cb_sysinfo = {0};

/**
 * Initialize Coreboot platform
 * This replaces UEFI initialization with Coreboot firmware services
 */
BOOLEAN
EFIAPI
CorebootPlatformInit (
  VOID
  )
{
    // Find Coreboot table in low memory
    for (UINT32 addr = 0x500; addr < 0x1000; addr += 16) {
        cb_header = (COREBOOT_TABLE_HEADER*)(UINTN)addr;

        // Verify magic and checksums
        if (cb_header->magic == COREBOOT_TABLE_MAGIC) {
            UINT32 checksum = 0;
            UINT8* ptr = (UINT8*)cb_header;

            // Calculate header checksum
            for (UINT32 i = 0; i < cb_header->header_bytes; i++) {
                checksum += ptr[i];
            }
            checksum = (UINT8)(0x100 - (checksum & 0xFF));

            if (checksum != cb_header->header_checksum) {
                continue; // Checksum mismatch
            }

            // Verify table checksum
            checksum = 0;
            for (UINT32 i = 0; i < cb_header->table_bytes; i++) {
                checksum += ptr[cb_header->header_bytes + i];
            }
            checksum = (UINT8)(0x100 - (checksum & 0xFF));

            if (checksum != cb_header->table_checksum) {
                continue; // Checksum mismatch
            }

            break; // Found valid Coreboot table
        }
    }

    if (!cb_header) {
        return FALSE; // Coreboot table not found
    }

    // Parse Coreboot table entries
    return CorebootParseTable();
}

/**
 * Parse Coreboot table entries
 */
BOOLEAN
EFIAPI
CorebootParseTable (
  VOID
  )
{
    if (!cb_header) {
        return FALSE;
    }

    COREBOOT_TABLE_ENTRY* entry = cb_header->entries;

    for (UINT32 i = 0; i < 128; i++) { // Reasonable limit
        if (entry->tag == 0) {
            break; // End of entries
        }

        switch (entry->tag) {
            case CB_TAG_MEMORY:
                cb_memory_map = (COREBOOT_MEM_ENTRY*)((UINT8*)entry + sizeof(*entry));
                cb_memory_map_entries = entry->size / sizeof(COREBOOT_MEM_ENTRY);
                break;

            case CB_TAG_FRAMEBUFFER:
                cb_framebuffer = (COREBOOT_FB*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_VERSION:
                cb_sysinfo.version = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_EXTRA_VERSION:
                cb_sysinfo.extra_version = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_BUILD:
                cb_sysinfo.build = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_COMPILE_TIME:
                cb_sysinfo.compile_time = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_COMPILER:
                cb_sysinfo.compiler = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_ASSEMBLER:
                cb_sysinfo.assembler = (CHAR8*)((UINT8*)entry + sizeof(*entry));
                break;

            case CB_TAG_BOARD_ID:
                cb_sysinfo.board_id = *(UINT16*)((UINT8*)entry + sizeof(*entry));
                break;
        }

        entry = (COREBOOT_TABLE_ENTRY*)((UINT8*)entry + entry->size);
    }

    return TRUE;
}

/**
 * Get Coreboot system information
 */
CONST COREBOOT_SYSINFO*
EFIAPI
CorebootGetSysinfo (
  VOID
  )
{
    return &cb_sysinfo;
}

/**
 * Get memory map from Coreboot
 */
CONST COREBOOT_MEM_ENTRY*
EFIAPI
CorebootGetMemoryMap (
  OUT UINT32* Count
  )
{
    if (Count) {
        *Count = cb_memory_map_entries;
    }
    return cb_memory_map;
}

/**
 * Get framebuffer information from Coreboot
 */
CONST COREBOOT_FB*
EFIAPI
CorebootGetFramebuffer (
  VOID
  )
{
    return cb_framebuffer;
}

/**
 * Check if running under Coreboot
 */
BOOLEAN
EFIAPI
CorebootIsPresent (
  VOID
  )
{
    return (cb_header != NULL) && (cb_header->magic == COREBOOT_TABLE_MAGIC);
}

/**
 * Get total available RAM from Coreboot memory map
 */
UINT64
EFIAPI
CorebootGetTotalMemory (
  VOID
  )
{
    if (!cb_memory_map) {
        return 0;
    }

    UINT64 total = 0;
    for (UINT32 i = 0; i < cb_memory_map_entries; i++) {
        if (cb_memory_map[i].type == CB_MEM_RAM) {
            total += cb_memory_map[i].size;
        }
    }

    return total;
}

/**
 * Find largest available RAM region
 */
BOOLEAN
EFIAPI
CorebootFindLargestMemoryRegion (
  OUT UINT64* Addr,
  OUT UINT64* Size
  )
{
    if (!cb_memory_map || !Addr || !Size) {
        return FALSE;
    }

    UINT64 largest_addr = 0;
    UINT64 largest_size = 0;

    for (UINT32 i = 0; i < cb_memory_map_entries; i++) {
        if (cb_memory_map[i].type == CB_MEM_RAM &&
            cb_memory_map[i].size > largest_size) {
            largest_addr = cb_memory_map[i].addr;
            largest_size = cb_memory_map[i].size;
        }
    }

    if (largest_size == 0) {
        return FALSE;
    }

    *Addr = largest_addr;
    *Size = largest_size;
    return TRUE;
}

/**
 * Initialize graphics using Coreboot framebuffer
 */
BOOLEAN
EFIAPI
CorebootInitGraphics (
  VOID
  )
{
    if (!cb_framebuffer) {
        return FALSE;
    }

    // Graphics initialization handled by Coreboot
    // BloodHorn can use the framebuffer directly
    return TRUE;
}

/**
 * Get framebuffer address for graphics
 */
VOID*
EFIAPI
CorebootGetFramebufferAddress (
  VOID
  )
{
    if (cb_framebuffer) {
        return (VOID*)(UINTN)cb_framebuffer->physical_address;
    }
    return NULL;
}

/**
 * Get framebuffer dimensions
 */
BOOLEAN
EFIAPI
CorebootGetFramebufferInfo (
  OUT UINT32* Width,
  OUT UINT32* Height,
  OUT UINT32* Bpp
  )
{
    if (!cb_framebuffer) {
        return FALSE;
    }

    if (Width) *Width = cb_framebuffer->x_resolution;
    if (Height) *Height = cb_framebuffer->y_resolution;
    if (Bpp) *Bpp = cb_framebuffer->bits_per_pixel;

    return TRUE;
}

/**
 * Initialize PCI bus (handled by Coreboot)
 */
BOOLEAN
EFIAPI
CorebootInitPci (
  VOID
  )
{
    // PCI bus initialization is handled by Coreboot firmware
    // BloodHorn can enumerate devices if needed
    return TRUE;
}

/**
 * Initialize storage devices (handled by Coreboot)
 */
BOOLEAN
EFIAPI
CorebootInitStorage (
  VOID
  )
{
    // Storage controller initialization handled by Coreboot
    // BloodHorn can access storage devices through standard interfaces
    return TRUE;
}

/**
 * Initialize USB host controllers (handled by Coreboot)
 */
BOOLEAN
EFIAPI
CorebootInitUsb (
  VOID
  )
{
    // USB initialization handled by Coreboot
    // BloodHorn can access USB devices if needed
    return TRUE;
}

/**
 * Initialize network interfaces (handled by Coreboot)
 */
BOOLEAN
EFIAPI
CorebootInitNetwork (
  VOID
  )
{
    // Network interface initialization handled by Coreboot
    // BloodHorn can use network devices for PXE boot
    return TRUE;
}

/**
 * Initialize TPM (handled by Coreboot)
 */
BOOLEAN
EFIAPI
CorebootInitTpm (
  VOID
  )
{
    // TPM initialization and configuration handled by Coreboot
    // BloodHorn can use TPM for measurements
    return TRUE;
}

/**
 * Get ACPI tables from Coreboot
 */
VOID*
EFIAPI
CorebootGetAcpiRsdp (
  VOID
  )
{
    // Coreboot generates ACPI tables
    // BloodHorn can find RSDP in standard locations
    // For now, return NULL - bootloader should find it itself
    return NULL;
}

/**
 * Get SMBIOS tables from Coreboot
 */
VOID*
EFIAPI
CorebootGetSmbiosEntryPoint (
  VOID
  )
{
    // Coreboot generates SMBIOS tables
    // BloodHorn can find SMBIOS entry point in standard locations
    // For now, return NULL - bootloader should find it itself
    return NULL;
}

/**
 * Reboot system using Coreboot services
 */
VOID
EFIAPI
CorebootReboot (
  VOID
  )
{
    // Use Coreboot's reset mechanism
    // For now, use standard reboot mechanism
    __asm__ volatile ("int $0x19"); // BIOS reboot
}

/**
 * Shutdown system using Coreboot services
 */
VOID
EFIAPI
CorebootShutdown (
  VOID
  )
{
    // Use Coreboot's power management
    // For now, use ACPI shutdown mechanism
    __asm__ volatile ("outw %w0, %w1" : : "a" (0x2000), "d" (0xB004)); // ACPI shutdown
}

/**
 * Print Coreboot system information
 */
VOID
EFIAPI
CorebootPrintInfo (
  VOID
  )
{
    if (!CorebootIsPresent()) {
        return;
    }

    // Print version information
    if (cb_sysinfo.version) {
        // Use bootloader's print function
        // This would be called from the main bootloader
    }
}

/**
 * Validate Coreboot firmware integrity
 */
BOOLEAN
EFIAPI
CorebootValidateFirmware (
  VOID
  )
{
    // Coreboot handles its own firmware validation
    // BloodHorn can verify the coreboot payload if needed
    return TRUE;
}

/**
 * Get Coreboot build information
 */
CONST CHAR8*
EFIAPI
CorebootGetBuildInfo (
  VOID
  )
{
    return cb_sysinfo.build;
}

/**
 * Get Coreboot compile time
 */
CONST CHAR8*
EFIAPI
CorebootGetCompileTime (
  VOID
  )
{
    return cb_sysinfo.compile_time;
}
