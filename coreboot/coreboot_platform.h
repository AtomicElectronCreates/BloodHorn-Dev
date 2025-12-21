/*
 * coreboot_platform.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef COREBOOT_PLATFORM_H
#define COREBOOT_PLATFORM_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>

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

// Coreboot system information structure
typedef struct {
    CHAR8* version;
    CHAR8* extra_version;
    CHAR8* build;
    CHAR8* compile_time;
    CHAR8* compiler;
    CHAR8* assembler;
    UINT16 board_id;
} COREBOOT_SYSINFO;

// Platform initialization functions
BOOLEAN EFIAPI CorebootPlatformInit(VOID);
BOOLEAN EFIAPI CorebootParseTable(VOID);

// System information functions
CONST COREBOOT_SYSINFO* EFIAPI CorebootGetSysinfo(VOID);
BOOLEAN EFIAPI CorebootIsPresent(VOID);
VOID EFIAPI CorebootPrintInfo(VOID);

// Memory management functions
CONST COREBOOT_MEM_ENTRY* EFIAPI CorebootGetMemoryMap(OUT UINT32* Count);
UINT64 EFIAPI CorebootGetTotalMemory(VOID);
BOOLEAN EFIAPI CorebootFindLargestMemoryRegion(OUT UINT64* Addr, OUT UINT64* Size);

// Graphics functions
BOOLEAN EFIAPI CorebootInitGraphics(VOID);
CONST COREBOOT_FB* EFIAPI CorebootGetFramebuffer(VOID);
VOID* EFIAPI CorebootGetFramebufferAddress(VOID);
BOOLEAN EFIAPI CorebootGetFramebufferInfo(OUT UINT32* Width, OUT UINT32* Height, OUT UINT32* Bpp);

// Hardware initialization functions (handled by Coreboot)
BOOLEAN EFIAPI CorebootInitPci(VOID);
BOOLEAN EFIAPI CorebootInitStorage(VOID);
BOOLEAN EFIAPI CorebootInitUsb(VOID);
BOOLEAN EFIAPI CorebootInitNetwork(VOID);
BOOLEAN EFIAPI CorebootInitTpm(VOID);

// Table access functions
VOID* EFIAPI CorebootGetAcpiRsdp(VOID);
VOID* EFIAPI CorebootGetSmbiosEntryPoint(VOID);

// Power management functions
VOID EFIAPI CorebootReboot(VOID);
VOID EFIAPI CorebootShutdown(VOID);

// Validation functions
BOOLEAN EFIAPI CorebootValidateFirmware(VOID);

// Information access functions
CONST CHAR8* EFIAPI CorebootGetBuildInfo(VOID);
CONST CHAR8* EFIAPI CorebootGetCompileTime(VOID);

#endif // COREBOOT_PLATFORM_H
