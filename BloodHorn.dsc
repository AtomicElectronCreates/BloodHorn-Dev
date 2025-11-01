[Defines]
  PLATFORM_NAME                  = BloodHorn
  PLATFORM_GUID                  = 12345678-1234-1234-1234-1234567890AB
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/BloodHorn
  SUPPORTED_ARCHITECTURES        = X64|AARCH64|RISCV64|LOONGARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = BloodHorn.fdf
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24D7EEDEED21

[LibraryClasses]
  # Basic UEFI Libraries
  UefiLib                 = Edk2BHModules/MdePkg/Library/UefiLib/UefiLib.inf
  UefiApplicationEntryPoint = Edk2BHModules/MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib = Edk2BHModules/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib = Edk2BHModules/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  
  # Core Libraries
  BaseLib                 = Edk2BHModules/MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib           = Edk2BHModules/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib                = Edk2BHModules/MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib  = Edk2BHModules/MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  
  # Memory Management
  MemoryAllocationLib     = Edk2BHModules/MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DxeServicesTableLib     = Edk2BHModules/MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  
  # PCD and Configuration
  PcdLib                  = Edk2BHModules/MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  
  # String and Print
  PrintLib                = Edk2BHModules/MdePkg/Library/BasePrintLib/BasePrintLib.inf
  
  # Platform Specific
  DebugAgentLib           = Edk2BHModules/MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  
  # For Coreboot integration
  CorebootLib             = BloodHorn/Coreboot/Library/CorebootLib/CorebootLib.inf
  
  # Coreboot Payload Support
  CorebootPayloadLib|Edk2BHModules/CorebootPayloadPkg/Library/CorebootPayloadLib/CorebootPayloadLib.inf
  
  # Security Libraries
  BaseCryptLib            = Edk2BHModules/CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  Tpm2DeviceLib           = Edk2BHModules/SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf

[Packages]
  # Core EDK2 Packages
  Edk2BHModules/MdePkg/MdePkg.dec
  Edk2BHModules/MdeModulePkg/MdeModulePkg.dec
  
  # Coreboot Integration
  Edk2BHModules/CorebootPayloadPkg/CorebootPayloadPkg.dec
  
  # Security Packages
  Edk2BHModules/CryptoPkg/CryptoPkg.dec
  Edk2BHModules/SecurityPkg/SecurityPkg.dec
  
  # Platform Specific
  BloodHorn/BloodHorn.dec

[Components]
  # Main Application
  BloodHorn/BloodHorn.inf
  
  # Coreboot Support
  BloodHorn/Coreboot/Library/CorebootLib/CorebootLib.inf
  
  # Coreboot Payload Support
  Edk2BHModules/CorebootPayloadPkg/Library/CorebootPayloadLib/CorebootPayloadLib.inf {
    <PcdsFixedAtBuild>
      gCorebootPayloadPkgTokenSpaceGuid.PcdCorebootTableAddress|0x0
  }
  
  # Required UEFI Drivers
  Edk2BHModules/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  Edk2BHModules/MdeModulePkg/Core/Dxe/DxeMain.inf
  Edk2BHModules/MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  
  # Console Support
  Edk2BHModules/MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  
  # File System Support
  Edk2BHModules/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  Edk2BHModules/MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  
  # FAT File System
  Edk2BHModules/FatPkg/EnhancedFatDxe/Fat.inf
  
  # BDS (Boot Device Selection)
  Edk2BHModules/MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

[PcdsFixedAtBuild]
  # Coreboot platform configuration
  gBloodHornTokenSpaceGuid.PcdCorebootEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootTableAddress|0x500
  gBloodHornTokenSpaceGuid.PcdCorebootFramebufferEnabled|TRUE

  # Bootloader configuration
  gBloodHornTokenSpaceGuid.PcdDefaultBootEntry|"linux"
  gBloodHornTokenSpaceGuid.PcdMenuTimeout|10
  gBloodHornTokenSpaceGuid.PcdTpmEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdSecureBootEnabled|FALSE
  gBloodHornTokenSpaceGuid.PcdGuiEnabled|TRUE

[BuildOptions]
  # Global build options
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
  
  # Coreboot-specific build options
  GCC:*_*_*_CC_FLAGS = -DCOREBOOT_PLATFORM -I$(WORKSPACE)/BloodHorn/coreboot -I$(WORKSPACE)/Edk2BHModules
  
  # Debug options
  *_DEBUG_*_CC_FLAGS = -DDEBUG_ENABLE=1
  *_RELEASE_*_CC_FLAGS = -DNDEBUG
  
  # Security flags
  *_*_*_CC_FLAGS = -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks
  
  # Warning flags
  GCC:*_*_*_CC_FLAGS = -Wall -Werror -Wno-array-bounds -Wno-unused-parameter
  
  # Set the EDK2 build tools path
  *_*_*_PATH = $(WORKSPACE)/Edk2BHModules/BaseTools/BinWrappers/WindowsLike

[UserExtensions.TianoCore."ExtraFiles"]
  # Coreboot platform files
  BloodHorn/coreboot/coreboot_platform.h
  BloodHorn/coreboot/coreboot_payload.h
