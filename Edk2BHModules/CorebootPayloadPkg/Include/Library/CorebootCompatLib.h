/** @file
  Coreboot Compatibility Library for BloodHorn

  This library provides a compatibility layer between EDK2 CorebootPayloadPkg
  and the BloodHorn coreboot.h interface.

  Copyright (c) 2025, BloodHorn Developers. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef COREBOOT_COMPAT_LIB_H_
#define COREBOOT_COMPAT_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/CorebootPayloadLib.h>

// Forward declarations from bloodhorn/coreboot.h
typedef UINT32 bh_coreboot_tag_t;
typedef UINT32 bh_status_t;

typedef struct {
  UINT32  tag;
  UINT32  size;
} bh_coreboot_header_t;

typedef enum {
  BH_STATUS_SUCCESS = 0,
  BH_STATUS_INVALID_PARAMETER,
  BH_STATUS_NOT_FOUND,
  BH_STATUS_UNSUPPORTED
} bh_status_enum_t;

/**
  Initialize Coreboot compatibility layer

  @retval EFI_SUCCESS     The compatibility layer was initialized successfully
  @retval Others          An error occurred during initialization
**/
EFI_STATUS
EFIAPI
BhCorebootCompatInitialize (
  VOID
  );

/**
  Find a Coreboot table by tag

  @param[in]   Tag      The tag to find
  @param[out]  Data     Pointer to store the found data
  @param[out]  Size     Size of the found data

  @retval BH_STATUS_SUCCESS      The tag was found successfully
  @retval BH_STATUS_NOT_FOUND    The tag was not found
  @retval BH_STATUS_INVALID_PARAMETER  Invalid parameters
**/
bh_status_t
EFIAPI
BhCorebootFindTag (
  IN  bh_coreboot_tag_t  Tag,
  OUT VOID               **Data,
  OUT UINTN              *Size
  );

/**
  Get the Coreboot memory map

  @param[out]  Ranges   Pointer to store the memory ranges
  @param[out]  Count    Number of memory ranges

  @retval BH_STATUS_SUCCESS      Memory map retrieved successfully
  @retval BH_STATUS_NOT_FOUND    Memory map not available
  @retval BH_STATUS_INVALID_PARAMETER  Invalid parameters
**/
bh_status_t
EFIAPI
BhCorebootGetMemoryMap (
  OUT VOID    **Ranges,
  OUT UINTN   *Count
  );

#endif // COREBOOT_COMPAT_LIB_H_
