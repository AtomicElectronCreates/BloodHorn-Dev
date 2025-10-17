## @file
# BloodHorn Bootloader PCD Definitions
#
# This file defines the PCDs used by BloodHorn bootloader
# for Coreboot platform integration and configuration.
#

[PcdsFixedAtBuild]
  # Coreboot platform configuration PCDs
  gBloodHornTokenSpaceGuid.PcdCorebootEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootTableAddress|0x500
  gBloodHornTokenSpaceGuid.PcdCorebootFramebufferEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootMemoryMapEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootAcpiEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdCorebootSmbiosEnabled|TRUE

  # Bootloader configuration PCDs
  gBloodHornTokenSpaceGuid.PcdDefaultBootEntry|"linux"
  gBloodHornTokenSpaceGuid.PcdMenuTimeout|10
  gBloodHornTokenSpaceGuid.PcdTpmEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdSecureBootEnabled|FALSE
  gBloodHornTokenSpaceGuid.PcdGuiEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdNetworkEnabled|FALSE
  gBloodHornTokenSpaceGuid.PcdLuaScriptingEnabled|TRUE
  gBloodHornTokenSpaceGuid.PcdPluginSystemEnabled|TRUE

  # Hardware support PCDs
  gBloodHornTokenSpaceGuid.PcdSupportX86_64|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportIa32|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportAarch64|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportRiscv64|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportLoongarch64|TRUE

  # Boot protocol support PCDs
  gBloodHornTokenSpaceGuid.PcdSupportLinuxBoot|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportMultiboot|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportMultiboot2|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportLimine|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportChainload|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportPxeBoot|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportBloodChain|TRUE

  # Filesystem support PCDs
  gBloodHornTokenSpaceGuid.PcdSupportFat32|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportExt2|TRUE
  gBloodHornTokenSpaceGuid.PcdSupportIso9660|FALSE

  # Security PCDs
  gBloodHornTokenSpaceGuid.PcdTpm20Support|TRUE
  gBloodHornTokenSpaceGuid.PcdMeasuredBoot|TRUE
  gBloodHornTokenSpaceGuid.PcdFileVerification|TRUE
  gBloodHornTokenSpaceGuid.PcdSecureBootSupport|TRUE

  # UI PCDs
  gBloodHornTokenSpaceGuid.PcdDefaultFont|"DejaVuSans.ttf"
  gBloodHornTokenSpaceGuid.PcdDefaultFontSize|12
  gBloodHornTokenSpaceGuid.PcdDefaultLanguage|"en"
  gBloodHornTokenSpaceGuid.PcdThemeSupport|TRUE

  # Debug PCDs
  gBloodHornTokenSpaceGuid.PcdDebugEnabled|FALSE
  gBloodHornTokenSpaceGuid.PcdVerboseBoot|FALSE
  gBloodHornTokenSpaceGuid.PcdSerialConsole|FALSE

[PcdsDynamic]
  # Dynamic PCDs that can be modified at runtime
  gBloodHornTokenSpaceGuid.PcdCurrentBootEntry|""
  gBloodHornTokenSpaceGuid.PcdBootDevicePath|""
  gBloodHornTokenSpaceGuid.PcdKernelCmdline|""
  gBloodHornTokenSpaceGuid.PcdInitrdPath|""

[Guids]
  # BloodHorn token space GUID
  gBloodHornTokenSpaceGuid = {0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB}}
