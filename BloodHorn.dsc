#
# BloodHorn Bootloader - Platform Description File
#
# Copyright (c) 2025, BloodyHell Industries INC. All rights reserved.<BR>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = BloodHorn
  PLATFORM_GUID                  = 6f4f3e3a-1c4c-4b7d-9a5e-9b3a5c4d1001
  PLATFORM_VERSION                = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/BloodHorn
  SUPPORTED_ARCHITECTURES        = IA32|X64|ARM|AARCH64|RISCV64|LOONGARCH64
  SUPPORTED_TOOL_CHAINS          = GCC:CLANG:MSVC:INTEL:RVCT
  BUILD_TARGETS                  = DEBUG|RELEASE

[BuildOptions]
  GCC:*_*_*_CC_FLAGS = -DUNICODE -std=c11 -Wall -Wextra
  GCC:*_*_*_PP_FLAGS = -DUNICODE
  GCC:*_*_*_ASM_FLAGS = -DUNICODE
  MSFT:*_*_*_CC_FLAGS = /D UNICODE /WX
  MSFT:*_*_*_PP_FLAGS = /D UNICODE
  INTEL:*_*_*_CC_FLAGS = /D UNICODE /WX
  RVCT:*_*_*_CC_FLAGS = -DUNICODE

[PcdsFeatureFlag]
  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x06
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLangCodes|en-US;fr-FR
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLang|en-US

[Components]
  #
  # BloodHorn Boot Manager Application
  #
  BloodHorn/BloodHorn.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
      GCC:*_*_*_CC_FLAGS = -std=c11 -I$(WORKSPACE)/BloodHorn/boot/freetype/include -DFT2_BUILD_LIBRARY
      MSFT:*_*_*_CC_FLAGS = /std=c11 /I"$(WORKSPACE)/BloodHorn/boot/freetype/include" /DFT2_BUILD_LIBRARY
    </BuildOptions>
    
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
    </PcdsFixedAtBuild>
    
    <LibraryClasses>
      NULL|MdePkg/Library/BaseLib/BaseLib.inf
      NULL|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
      NULL|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
      NULL|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
      NULL|MdePkg/Library/UefiLib/UefiLib.inf
      NULL|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      NULL|MdePkg/Library/DebugPrintErrorLevelLib/DebugPrintErrorLevelLib.inf
      NULL|MdePkg/Library/PcdLib/PcdLib.inf
      NULL|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
      NULL|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
      NULL|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
      NULL|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
      NULL|MdeModulePkg/Library/DevicePathLib/DevicePathLib.inf
      NULL|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
      NULL|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
      NULL|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
      NULL|MdePkg/Library/BaseCryptLib/BaseCryptLib.inf
      NULL|MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
      NULL|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
      NULL|MdeModulePkg/Library/ConsoleLoggerLib/ConsoleLoggerLib.inf
      NULL|MdePkg/Library/CapsuleLib/CapsuleLib.inf
      NULL|MdePkg/Library/PerformanceLib/PerformanceLib.inf
      NULL|MdePkg/Library/TimerLib/TimerLib.inf
      NULL|MdePkg/Library/SynchronizationLib/SynchronizationLib.inf
      NULL|MdePkg/Library/ReportStatusCodeLib/ReportStatusCodeLib.inf
      NULL|MdePkg/Library/PeCoffExtraActionLib/PeCoffExtraActionLib.inf
      NULL|MdePkg/Library/CacheMaintenanceLib/CacheMaintenanceLib.inf
      NULL|MdePkg/Library/UefiDecompressLib/UefiDecompressLib.inf
      NULL|MdePkg/Library/OrderedCollectionLib/OrderedCollectionLib.inf
      NULL|MdePkg/Library/SafeIntLib/SafeIntLib.inf
      NULL|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
      NULL|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
      NULL|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
      NULL|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
      NULL|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
      NULL|SecurityPkg/Library/TpmMeasurementLib/TpmMeasurementLib.inf
      NULL|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLib/Tpm2DeviceLib.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouter.inf
      NULL|NetworkPkg/Library/DpcLib/DpcLib.inf
      NULL|NetworkPkg/Library/NetLib/NetLib.inf
      NULL|NetworkPkg/Library/IpIoLib/IpIoLib.inf
      NULL|NetworkPkg/Library/UdpIoLib/UdpIoLib.inf
      NULL|NetworkPkg/Library/Dhcp4Lib/Dhcp4Lib.inf
      NULL|NetworkPkg/Library/Mtftp4Lib/Mtftp4Lib.inf
      NULL|NetworkPkg/Library/ArpLib/ArpLib.inf
      NULL|NetworkPkg/Library/BaseNetworkLib/BaseNetworkLib.inf
      NULL|StdLib/StandardLib/StandardLib.inf
    </LibraryClasses>
    
    <Protocols>
      gEfiSimpleFileSystemProtocolGuid
      gEfiLoadedImageProtocolGuid
      gEfiDevicePathProtocolGuid
      gEfiUnicodeCollation2ProtocolGuid
      gEfiSimpleTextInProtocolGuid
      gEfiSimpleTextOutProtocolGuid
      gEfiGraphicsOutputProtocolGuid
      gEfiHiiProtocolGuid
      gEfiHiiDatabaseProtocolGuid
      gEfiHiiStringProtocolGuid
      gEfiHiiConfigRoutingProtocolGuid
      gEfiHiiConfigAccessProtocolGuid
      gEfiBootManagerPolicyProtocolGuid
      gEfiDevicePathToTextProtocolGuid
      gEfiDevicePathFromTextProtocolGuid
      gEfiSimpleNetworkProtocolGuid
      gEfiSimpleTextInputProtocolGuid
      gEfiSimpleTextInputExProtocolGuid
      gEfiPxeBaseCodeProtocolGuid
      gEfiBlockIoProtocolGuid
      gEfiBlockIo2ProtocolGuid
      gEfiDiskIoProtocolGuid
      gEfiScsiIoProtocolGuid
      gEfiScsiPassThruProtocolGuid
      gEfiUsbIoProtocolGuid
      gEfiUsb2HcProtocolGuid
      gEfiTcg2ProtocolGuid
      gEfiTcgProtocolGuid
    </Protocols>
    
    <Guids>
      gEfiGlobalVariableGuid
      gEfiEventVirtualAddressChangeGuid
      gEfiEventExitBootServicesGuid
      gEfiEventSetVirtualAddressMapGuid
      gEfiEventReadyToBootGuid
      gEfiEventMemoryMapChangeGuid
      gEfiEventVendorGuid
    </Guids>
    
    <PcdsFeatureFlag>
      gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
    </PcdsFeatureFlag>
    
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
      gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x06
      gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
      gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
      gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
      gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLangCodes|en-US;fr-FR
      gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLang|en-US
    </PcdsFixedAtBuild>
  }

[BuildOptions.Common.EDKII]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

[BuildOptions.Common.GCC]
  *_*_*_CC_FLAGS = -std=c11 -DFT2_BUILD_LIBRARY -I$(WORKSPACE)/BloodHorn/boot/freetype/include

[BuildOptions.Common.MSFT]
  *_*_*_CC_FLAGS = /std=c11 /DFT2_BUILD_LIBRARY /I"$(WORKSPACE)/BloodHorn/boot/freetype/include"

[BuildOptions.Common.INTEL]
  *_*_*_CC_FLAGS = /std=c11 /DFT2_BUILD_LIBRARY /I"$(WORKSPACE)/BloodHorn/boot/freetype/include"

[BuildOptions.Common.CLANG]
  *_*_*_CC_FLAGS = -std=c11 -DFT2_BUILD_LIBRARY -I$(WORKSPACE)/BloodHorn/boot/freetype/include
