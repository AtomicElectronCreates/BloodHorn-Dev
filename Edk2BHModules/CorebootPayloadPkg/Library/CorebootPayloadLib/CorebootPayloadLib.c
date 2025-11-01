/** @file
  Coreboot Payload Library Implementation

  Copyright (c) 2025, BloodHorn Developers. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/CorebootPayloadLib.h>

// Static pointer to the Coreboot table
STATIC CB_TABLE_HEADER *mCorebootTable = NULL;

/**
  Calculate CRC32 for Coreboot table

  @param[in]  Data      Pointer to the data
  @param[in]  Length    Length of the data
  
  @return The calculated CRC32 value
**/
STATIC
UINT32
CalculateCrc32 (
  IN CONST VOID  *Data,
  IN UINTN       Length
  )
{
  UINT32  Crc = 0xFFFFFFFF;
  UINT8   *Ptr = (UINT8 *)Data;
  
  while (Length-- != 0) {
    Crc ^= *Ptr++;
    for (UINTN i = 0; i < 8; i++) {
      if (Crc & 1) {
        Crc = (Crc >> 1) ^ 0xEDB88320;
      } else {
        Crc >>= 1;
      }
    }
  }
  
  return ~Crc;
}

/**
  Find the Coreboot table in memory

  @retval EFI_SUCCESS       Table found
  @retval EFI_NOT_FOUND     Table not found
**/
STATIC
EFI_STATUS
FindCorebootTable (
  VOID
  )
{
  UINTN  Ebda;
  UINTN  Address;
  
  // Check if table address was provided via PCD
  Address = PcdGet32 (PcdCorebootTableAddress);
  if (Address != 0) {
    mCorebootTable = (CB_TABLE_HEADER *)(UINTN)Address;
    if (mCorebootTable->Signature == CB_TABLE_SIGNATURE) {
      return EFI_SUCCESS;
    }
  }
  
  // Search in EBDA (0x40E:0x0)
  Ebda = (*(UINT16 *)(UINTN)0x40E) << 4;
  for (Address = Ebda; Address < Ebda + 0x400; Address += 16) {
    mCorebootTable = (CB_TABLE_HEADER *)Address;
    if (mCorebootTable->Signature == CB_TABLE_SIGNATURE) {
      return EFI_SUCCESS;
    }
  }
  
  // Search in 0x00000-0x1000
  for (Address = 0; Address < 0x1000; Address += 16) {
    mCorebootTable = (CB_TABLE_HEADER *)(UINTN)Address;
    if (mCorebootTable->Signature == CB_TABLE_SIGNATURE) {
      return EFI_SUCCESS;
    }
  }
  
  mCorebootTable = NULL;
  return EFI_NOT_FOUND;
}

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
  )
{
  EFI_STATUS  Status;
  UINT32      Crc;
  
  if (mCorebootTable != NULL) {
    return EFI_SUCCESS;
  }
  
  Status = FindCorebootTable ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Coreboot: Table not found\n"));
    return Status;
  }
  
  // Verify header checksum
  Crc = CalculateCrc32 (mCorebootTable, mCorebootTable->HeaderSize);
  if (Crc != 0) {
    DEBUG ((DEBUG_ERROR, "Coreboot: Invalid header checksum\n"));
    mCorebootTable = NULL;
    return EFI_CRC_ERROR;
  }
  
  // Verify table checksum
  Crc = CalculateCrc32 (
           (UINT8 *)mCorebootTable + mCorebootTable->HeaderSize,
           mCorebootTable->TableSize
           );
  
  if (Crc != 0) {
    DEBUG ((DEBUG_ERROR, "Coreboot: Invalid table checksum\n"));
    mCorebootTable = NULL;
    return EFI_CRC_ERROR;
  }
  
  DEBUG ((DEBUG_INFO, "Coreboot: Table found at 0x%p\n", mCorebootTable));
  return EFI_SUCCESS;
}

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
  )
{
  UINT8       *Ptr;
  UINT8       *End;
  UINT32      *TagPtr;
  
  if (mCorebootTable == NULL || Entry == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Ptr = (UINT8 *)mCorebootTable + mCorebootTable->HeaderSize;
  End = Ptr + mCorebootTable->TableSize;
  
  while (Ptr < End) {
    TagPtr = (UINT32 *)Ptr;
    if (*TagPtr == 0) {
      break;
    }
    
    if (*TagPtr == Tag) {
      if (EntrySize != NULL) {
        *EntrySize = *(UINT32 *)(Ptr + 4);
      }
      *Entry = Ptr;
      return EFI_SUCCESS;
    }
    
    // Move to next entry (4-byte aligned)
    Ptr += (*(UINT32 *)(Ptr + 4) + 3) & ~3;
  }
  
  return EFI_NOT_FOUND;
}

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
  )
{
  EFI_STATUS         Status;
  CB_TAG_FRAMEBUFFER *FbTag;
  
  if (FbInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = CorebootFindTable (CB_TAG_FRAMEBUFFER, (VOID **)&FbTag, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  *FbInfo = &FbTag->Framebuffer;
  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS  Status;
  VOID        *Table;
  UINT32      TableSize;
  
  if (MemoryMap == NULL || MemoryMapSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = CorebootFindTable (CB_TAG_MEMORY, &Table, &TableSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  *MemoryMap = (CB_MEMORY_RANGE *)((UINT8 *)Table + sizeof (UINT32) * 2);
  *MemoryMapSize = (TableSize - sizeof (UINT32) * 2) / sizeof (CB_MEMORY_RANGE);
  
  return EFI_SUCCESS;
}

/**
  Library constructor

  @param[in]  ImageHandle   The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The constructor always returns EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
CorebootPayloadLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  // Initialize the Coreboot table if not already done
  if (mCorebootTable == NULL) {
    CorebootPayloadInitialize ();
  }
  
  return EFI_SUCCESS;
}
