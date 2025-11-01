/** @file
  Coreboot Payload Library Interface

  Provides services to interact with Coreboot when running as a payload.

  Copyright (c) 2025, BloodHorn Developers. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef COREBOOT_PAYLOAD_LIB_H_
#define COREBOOT_PAYLOAD_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

// Coreboot table signature
#define CB_TABLE_SIGNATURE  SIGNATURE_32('L', 'B', 'I', 'O')

// Coreboot memory types
typedef enum {
  CB_MEM_RAM = 1,
  CB_MEM_RESERVED,
  CB_MEM_ACPI,
  CB_MEM_NVS,
  CB_MEM_UNUSABLE,
  CB_MEM_VENDOR_RSVD = 16,
  CB_MEM_TABLE = 16
} CB_MEMORY_TYPE;

// Coreboot table header
#pragma pack(1)
typedef struct {
  UINT32  Signature;
  UINT32  HeaderSize;
  UINT32  HeaderCrc32;
  UINT32  TableSize;
  UINT32  TableCrc32;
  UINT32  TableEntries;
} CB_TABLE_HEADER;

// Coreboot memory range
typedef struct {
  UINT64  Start;
  UINT64  Size;
  UINT32  Type;
  UINT32  Reserved;
} CB_MEMORY_RANGE;

// Coreboot framebuffer information
typedef struct {
  UINT64  PhysicalAddress;
  UINT32  XResolution;
  UINT32  YResolution;
  UINT32  BytesPerLine;
  UINT8   BitsPerPixel;
  UINT8   RedMaskPos;
  UINT8   RedMaskSize;
  UINT8   GreenMaskPos;
  UINT8   GreenMaskSize;
  UINT8   BlueMaskPos;
  UINT8   BlueMaskSize;
  UINT8   ReservedMaskPos;
  UINT8   ReservedMaskSize;
} CB_FRAMEBUFFER;

// Coreboot framebuffer tag
typedef struct {
  UINT32          Tag;
  UINT32          Size;
  CB_FRAMEBUFFER  Framebuffer;
} CB_TAG_FRAMEBUFFER;

/**
  Initialize the Coreboot Payload Library

  @retval EFI_SUCCESS           The library was initialized successfully
  @retval EFI_NOT_FOUND         Coreboot table not found
  @retval EFI_CRC_ERROR         Coreboot table checksum error
**/
EFI_STATUS
EFIAPI
CorebootPayloadInitialize (
  VOID
  );

/**
  Get the Coreboot framebuffer information

  @param[out] FbInfo     Pointer to store framebuffer information
  
  @retval EFI_SUCCESS    Framebuffer information retrieved successfully
  @retval EFI_NOT_FOUND  Framebuffer information not available
**/
EFI_STATUS
EFIAPI
CorebootGetFramebuffer (
  OUT CB_FRAMEBUFFER **FbInfo
  );

/**
  Get the Coreboot memory map

  @param[out] MemoryMap      Pointer to store memory map array
  @param[out] MemoryMapSize  Number of entries in the memory map
  
  @retval EFI_SUCCESS        Memory map retrieved successfully
  @retval EFI_NOT_FOUND      Memory map not available
**/
EFI_STATUS
EFIAPI
CorebootGetMemoryMap (
  OUT CB_MEMORY_RANGE  **MemoryMap,
  OUT UINTN            *MemoryMapSize
  );

/**
  Find a Coreboot table entry by tag

  @param[in]  Tag           Tag to search for
  @param[out] Entry         Pointer to store the found entry
  @param[out] EntrySize     Size of the found entry
  
  @retval EFI_SUCCESS       Entry found successfully
  @retval EFI_NOT_FOUND     Entry not found
**/
EFI_STATUS
EFIAPI
CorebootFindTable (
  IN  UINT32  Tag,
  OUT VOID    **Entry,
  OUT UINT32  *EntrySize OPTIONAL
  );

#endif // COREBOOT_PAYLOAD_LIB_H_
