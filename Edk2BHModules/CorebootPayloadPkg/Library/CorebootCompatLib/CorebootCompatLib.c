/** @file
  Coreboot Compatibility Library Implementation

  Copyright (c) 2025, BloodHorn Developers. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CorebootCompatLib.h>

// Global state
STATIC BOOLEAN mCorebootInitialized = FALSE;

/**
  Initialize Coreboot compatibility layer
**/
EFI_STATUS
EFIAPI
BhCorebootCompatInitialize (
  VOID
  )
{
  EFI_STATUS  Status;

  if (mCorebootInitialized) {
    return EFI_SUCCESS;
  }

  // Initialize CorebootPayloadLib if needed
  Status = CorebootPayloadInitialize ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to initialize CorebootPayloadLib: %r\n", Status));
    return Status;
  }

  mCorebootInitialized = TRUE;
  return EFI_SUCCESS;
}

/**
  Find a Coreboot table by tag
**/
bh_status_t
EFIAPI
BhCorebootFindTag (
  IN  bh_coreboot_tag_t  Tag,
  OUT VOID               **Data,
  OUT UINTN              *Size
  )
{
  VOID   *Table;
  UINTN  TableSize;

  if (Data == NULL || Size == NULL) {
    return BH_STATUS_INVALID_PARAMETER;
  }

  if (!mCorebootInitialized) {
    return BH_STATUS_UNSUPPORTED;
  }

  Table = CorebootFindTable ((UINT32)Tag, (UINT32 *)&TableSize);
  if (Table == NULL) {
    return BH_STATUS_NOT_FOUND;
  }

  *Data = Table;
  *Size = TableSize;
  return BH_STATUS_SUCCESS;
}

/**
  Get the Coreboot memory map
**/
bh_status_t
EFIAPI
BhCorebootGetMemoryMap (
  OUT VOID    **Ranges,
  OUT UINTN   *Count
  )
{
  EFI_STATUS  Status;
  UINTN       MemoryMapSize;
  VOID        *MemoryMap;
  UINTN       EntryCount;

  if (Ranges == NULL || Count == NULL) {
    return BH_STATUS_INVALID_PARAMETER;
  }

  if (!mCorebootInitialized) {
    return BH_STATUS_UNSUPPORTED;
  }

  // Get the memory map from CorebootPayloadLib
  Status = CorebootGetMemoryMap (&MemoryMap, &MemoryMapSize, &EntryCount);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get Coreboot memory map: %r\n", Status));
    return BH_STATUS_NOT_FOUND;
  }

  *Ranges = MemoryMap;
  *Count = EntryCount;
  return BH_STATUS_SUCCESS;
}
