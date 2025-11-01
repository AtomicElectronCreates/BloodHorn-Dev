## @file
#  Coreboot Payload Package DSC
#
#  This package provides support for running EDK2 as a Coreboot payload.
#
#  Copyright (c) 2025, BloodHorn Developers. All rights reserved.<BR>
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = CorebootPayloadPkg
  PLATFORM_GUID                  = 1E8C4E2A-3F3A-4D7A-8C5E-9A1B7D8F2C6D
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/CorebootPayloadPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  # Coreboot Payload Library
  CorebootPayloadLib|CorebootPayloadPkg/Library/CorebootPayloadLib/CorebootPayloadLib.inf
  
  # Standard Libraries
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[Components]
  # Coreboot Payload Library
  CorebootPayloadPkg/Library/CorebootPayloadLib/CorebootPayloadLib.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

[PcdsFixedAtBuild]
  # Coreboot Payload specific PCDs
  gCorebootPayloadPkgTokenSpaceGuid.PcdCorebootTableAddress|0x0
  gCorebootPayloadPkgTokenSpaceGuid.PcdCorebootDebug|FALSE
