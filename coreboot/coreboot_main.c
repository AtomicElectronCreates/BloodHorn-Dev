/*
 * coreboot_main.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Guid/Acpi.h>
#include <stdarg.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

#include "coreboot_platform.h"
#include "../boot/menu.h"
#include "../boot/theme.h"
#include "../boot/localization.h"
#include "../boot/mouse.h"
#include "../security/crypto.h"
#include "../security/sha512.h"
#include "../boot/libb/include/bloodhorn/bloodhorn.h"

STATIC EFI_HANDLE gImageHandle = NULL;
STATIC EFI_SYSTEM_TABLE* gST = NULL;
STATIC EFI_BOOT_SERVICES* gBS = NULL;
STATIC EFI_RUNTIME_SERVICES* gRT = NULL;

// Forward declarations
STATIC EFI_STATUS LoadAndVerifyKernelCoreboot(IN CHAR16* KernelPath, OUT VOID** KernelBuffer, OUT UINTN* KernelSize);
STATIC VOID LoadThemeAndLanguageFromConfig(VOID);
STATIC EFI_STATUS GetRootDirectory(OUT EFI_FILE_HANDLE* RootDir);
STATIC EFI_STATUS ExecuteKernelImage(IN VOID* KernelBuffer, IN UINTN KernelSize, IN CHAR16* Options);

// Boot wrapper function declarations
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID);
EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID);
EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID);
EFI_STATUS EFIAPI BootChainloadWrapper(VOID);
EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID);
EFI_STATUS EFIAPI BootBloodchainWrapper(VOID);
EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID);
EFI_STATUS EFIAPI ExitToCorebootWrapper(VOID);

VOID
EFIAPI
CorebootMain (
  IN VOID* coreboot_table,
  IN VOID* payload
  )
{
    EFI_STATUS Status;

    // Initialize UEFI system table (required for EDK2 compatibility btw)
    Status = EfiGetSystemConfigurationTable(&gEfiSystemTableGuid, (VOID**)&gST);
    if (EFI_ERROR(Status)) {
        // Fallback: assume we're running in a minimal environment
        Print(L"Warning: Could not get system table, using minimal initialization\n");
    }

    if (gST) {
        gBS = gST->BootServices;
        gRT = gST->RuntimeServices;
    }

    // Initialize Coreboot platform interface
    if (!CorebootPlatformInit()) {
        Print(L"Error: Coreboot platform initialization failed\n");
        return;
    }

    Print(L"BloodHorn Bootloader (Coreboot Payload Mode)\n");
    Print(L"Coreboot firmware detected and initialized\n");

    // Initialize hardware using Coreboot services
    if (CorebootInitGraphics()) {
        Print(L"Graphics initialized using Coreboot framebuffer\n");
    }

    if (CorebootInitStorage()) {
        Print(L"Storage initialized by Coreboot\n");
    }

    if (CorebootInitNetwork()) {
        Print(L"Network initialized by Coreboot\n");
    }

    if (CorebootInitTpm()) {
        Print(L"TPM initialized by Coreboot\n");
    }

    // Start main BloodHorn bootloader logic
    Status = BloodhornMainCoreboot();

    if (EFI_ERROR(Status)) {
        Print(L"BloodHorn execution failed: %r\n", Status);
    }

    // Should not reach here in normal operation
    Print(L"BloodHorn Coreboot payload terminated\n");
}

EFI_STATUS
EFIAPI
BloodhornMainCoreboot (
  VOID
  )
{
    EFI_STATUS Status;

    // Initialize BloodHorn library if available
    if (gST && gBS) {
        bh_system_table_t bloodhorn_system_table = {
            .alloc = (void* (*)(bh_size_t))AllocatePool,
            .free = (void (*)(void*))FreePool,
            .putc = (void (*)(char))bh_putc,
            .puts = (void (*)(const char*))bh_puts,
            .printf = (void (*)(const char*, ...))bh_printf,
            .get_memory_map = (bh_status_t (*)(bh_memory_descriptor_t**, bh_size_t*, bh_size_t*))bh_get_memory_map,
            .get_graphics_info = (bh_status_t (*)(bh_graphics_info_t*))bh_get_graphics_info,
            .get_rsdp = (void* (*)(void))bh_get_rsdp,
            .get_boot_device = (void* (*)(void))bh_get_boot_device,
            .reboot = (void (*)(void))bh_reboot,
            .shutdown = (void (*)(void))bh_shutdown,
            .debug_break = (void (*)(void))bh_debug_break
        };

        bh_status_t bh_status = bh_initialize(&bloodhorn_system_table);
        if (bh_status == BH_SUCCESS) {
            Print(L"BloodHorn library initialized successfully\n");
        }
    }

    // Initialize boot menu and UI
    LoadThemeAndLanguageFromConfig();
    InitMouse();

    // Set up boot entries for Coreboot environment
    AddBootEntry(L"Linux Kernel (Coreboot)", BootLinuxKernelWrapper);
    AddBootEntry(L"Multiboot2 Kernel (Coreboot)", BootMultiboot2KernelWrapper);
    AddBootEntry(L"Limine Kernel (Coreboot)", BootLimineKernelWrapper);
    AddBootEntry(L"Chainload Bootloader (Coreboot)", BootChainloadWrapper);
    AddBootEntry(L"PXE Network Boot (Coreboot)", BootPxeNetworkWrapper);
    AddBootEntry(L"BloodChain Protocol (Coreboot)", BootBloodchainWrapper);
    AddBootEntry(L"Recovery Shell (Coreboot)", BootRecoveryShellWrapper);
    AddBootEntry(L"Exit to Coreboot Firmware", ExitToCorebootWrapper);

    // Show boot menu
    Status = ShowBootMenu();
    if (Status == EFI_SUCCESS) {
        VOID* KernelBuffer = NULL;
        UINTN KernelSize = 0;

        // Load and verify kernel using Coreboot services
        Status = LoadAndVerifyKernelCoreboot(L"kernel.efi", &KernelBuffer, &KernelSize);
        if (!EFI_ERROR(Status)) {
            Status = ExecuteKernelImage(KernelBuffer, KernelSize, NULL);
            if (!EFI_ERROR(Status)) {
                return EFI_SUCCESS;
            }
        }
    }

    Print(L"No bootable device found or kernel failed.\n");
    Print(L"Press any key to reboot...");
    EFI_INPUT_KEY Key;
    UINTN Index;

    if (gBS) {
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
        gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    }

    // Reboot using Coreboot services
    CorebootReboot();

    return EFI_DEVICE_ERROR;
}

STATIC
EFI_STATUS
LoadAndVerifyKernelCoreboot (
  IN CHAR16* KernelPath,
  OUT VOID** KernelBuffer,
  OUT UINTN* KernelSize
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE RootDir;
    EFI_FILE_HANDLE File;
    VOID* Buffer = NULL;
    UINTN Size = 0;
    EFI_FILE_INFO* FileInfo = NULL;
    UINTN FileInfoSize = 0;

    if (!KernelPath || !KernelBuffer || !KernelSize) {
        return EFI_INVALID_PARAMETER;
    }

    // Get root directory
    Status = GetRootDirectory(&RootDir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get root directory: %r\n", Status);
        return Status;
    }

    // Open kernel file
    Status = RootDir->Open(RootDir, &File, KernelPath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file %s: %r\n", KernelPath, Status);
        RootDir->Close(RootDir);
        return Status;
    }

    // Get file size
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        FileInfo = AllocateZeroPool(FileInfoSize);
        if (!FileInfo) {
            File->Close(File);
            RootDir->Close(RootDir);
            return EFI_OUT_OF_RESOURCES;
        }
        Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    }

    if (EFI_ERROR(Status)) {
        if (FileInfo) FreePool(FileInfo);
        File->Close(File);
        RootDir->Close(RootDir);
        return Status;
    }

    Size = (UINTN)FileInfo->FileSize;
    Buffer = AllocateZeroPool(Size);
    if (!Buffer) {
        FreePool(FileInfo);
        File->Close(File);
        RootDir->Close(RootDir);
        return EFI_OUT_OF_RESOURCES;
    }

    // Read kernel into memory
    Status = File->Read(File, &Size, Buffer);
    File->Close(File);
    RootDir->Close(RootDir);
    FreePool(FileInfo);

    if (EFI_ERROR(Status)) {
        FreePool(Buffer);
        return Status;
    }

    // Verify kernel hash using SHA-512
    crypto_sha512_ctx_t ctx;
    uint8_t actual_hash[64];

    crypto_sha512_init(&ctx);
    crypto_sha512_update(&ctx, Buffer, (uint32_t)Size);
    crypto_sha512_final(&ctx, actual_hash);

    // Note: In production, compare against known good hash
    // For now, just log that verification was performed
    Print(L"Kernel hash computed successfully\n");

    *KernelBuffer = Buffer;
    *KernelSize = Size;

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetRootDirectory (
  OUT EFI_FILE_HANDLE* RootDir
  )
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_HANDLE* Handles = NULL;
    UINTN HandleCount = 0;

    if (!RootDir) {
        return EFI_INVALID_PARAMETER;
    }

    // Locate all file system handles
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &HandleCount,
        &Handles
    );

    if (EFI_ERROR(Status) || HandleCount == 0) {
        return EFI_NOT_FOUND;
    }

    // Use the first file system found
    Status = gBS->HandleProtocol(
        Handles[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**)&FileSystem
    );

    if (Handles) {
        FreePool(Handles);
    }

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Open root directory
    Status = FileSystem->OpenVolume(FileSystem, RootDir);
    return Status;
}

STATIC
EFI_STATUS
ExecuteKernelImage (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN CHAR16* Options
  )
{
    EFI_STATUS Status;
    EFI_HANDLE ImageHandle;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

    if (!KernelBuffer || KernelSize == 0) {
        return EFI_INVALID_PARAMETER;
    }

    // Load the image from memory
    Status = gBS->LoadImage(
        FALSE,
        gImageHandle,
        NULL,
        KernelBuffer,
        KernelSize,
        &ImageHandle
    );

    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel image: %r\n", Status);
        return Status;
    }

    // Set load options if provided
    if (Options) {
        Status = gBS->HandleProtocol(
            ImageHandle,
            &gEfiLoadedImageProtocolGuid,
            (VOID**)&LoadedImage
        );

        if (!EFI_ERROR(Status)) {
            LoadedImage->LoadOptions = Options;
            LoadedImage->LoadOptionsSize = (UINT32)(StrLen(Options) * sizeof(CHAR16));
        }
    }

    // Start the image
    Status = gBS->StartImage(ImageHandle, NULL, NULL);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to start kernel image: %r\n", Status);
        gBS->UnloadImage(ImageHandle);
        return Status;
    }

    return EFI_SUCCESS;
}

STATIC
VOID
LoadThemeAndLanguageFromConfig (
  VOID
  )
{
    // Load default theme and language
    // In production, this would read from config files
    Print(L"Loading default theme and language settings\n");
}

// Boot wrapper implementations
EFI_STATUS
EFIAPI
BootLinuxKernelWrapper(VOID) {
    const char* kernel_path = config_get_string("linux", "kernel");
    const char* initrd_path = config_get_string("linux", "initrd");
    const char* cmdline = config_get_string("linux", "cmdline");

    if (!kernel_path) {
        Print(L"Linux boot failed: kernel path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return linux_load_kernel(kernel_path, initrd_path, cmdline);
}

EFI_STATUS
EFIAPI
BootMultiboot2KernelWrapper(VOID) {
    const char* kernel_path = config_get_string("multiboot2", "kernel");
    const char* cmdline = config_get_string("multiboot2", "cmdline");

    if (!kernel_path) {
        Print(L"Multiboot2 boot failed: kernel path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return multiboot2_load_kernel(kernel_path, cmdline);
}

EFI_STATUS
EFIAPI
BootLimineKernelWrapper(VOID) {
    const char* kernel_path = config_get_string("limine", "kernel");
    const char* cmdline = config_get_string("limine", "cmdline");

    if (!kernel_path) {
        Print(L"Limine boot failed: kernel path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return limine_load_kernel(kernel_path, cmdline);
}

EFI_STATUS
EFIAPI
BootChainloadWrapper(VOID) {
    const char* file_path = config_get_string("chainload", "file");

    if (!file_path) {
        Print(L"Chainload failed: file path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return chainload_file(file_path);
}

EFI_STATUS
EFIAPI
BootPxeNetworkWrapper(VOID) {
    const char* kernel_path = config_get_string("pxe", "kernel");
    const char* initrd_path = config_get_string("pxe", "initrd");
    const char* cmdline = config_get_string("pxe", "cmdline");

    if (!kernel_path) {
        Print(L"PXE boot failed: kernel path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return BootFromNetwork(kernel_path, initrd_path, cmdline);
}

EFI_STATUS
EFIAPI
BootBloodchainWrapper(VOID) {
    const char* kernel_path = config_get_string("bloodchain", "kernel");
    const char* initrd_path = config_get_string("bloodchain", "initrd");
    const char* cmdline = config_get_string("bloodchain", "cmdline");

    if (!kernel_path) {
        Print(L"BloodChain boot failed: kernel path not specified in config.\n");
        return EFI_NOT_FOUND;
    }

    return bloodchain_load_kernel(kernel_path, initrd_path, cmdline);
}

EFI_STATUS
EFIAPI
BootRecoveryShellWrapper(VOID) {
    return shell_start();
}

STATIC VOID bh_putc(CHAR8 c) {
    if (gST && gST->ConOut) {
        CHAR16 wbuf[2] = { (CHAR16)c, 0 };
        gST->ConOut->OutputString(gST->ConOut, wbuf);
    }
}

STATIC VOID bh_puts(const CHAR8* str) {
    if (gST && gST->ConOut && str) {
        CHAR16 wbuf[512];
        UINTN i = 0;
        while (*str && i < 511) {
            wbuf[i++] = (CHAR16)*str++;
        }
        wbuf[i] = 0;
        gST->ConOut->OutputString(gST->ConOut, wbuf);
        gST->ConOut->OutputString(gST->ConOut, L"\n");
    }
}

STATIC VOID bh_printf(const CHAR8* format, ...) {
    VA_LIST args;
    VA_START(args, format);
    if (gST && gST->ConOut) {
        CHAR16 wbuf[512];
        AsciiVSPrintUnicodeFormat(wbuf, sizeof(wbuf), (CHAR8*)format, args);
        gST->ConOut->OutputString(gST->ConOut, wbuf);
    }
    VA_END(args);
}

STATIC bh_status_t bh_get_memory_map(bh_memory_descriptor_t** map, bh_size_t* map_size, bh_size_t* descriptor_size) {
    if (!gBS) return BH_ERROR;

    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;

    EFI_STATUS Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Status = gBS->AllocatePool(EfiBootServicesData, MemoryMapSize, (VOID**)&MemoryMap);
        if (EFI_ERROR(Status)) return BH_OUT_OF_MEMORY;

        Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    }

    if (EFI_ERROR(Status)) return BH_ERROR;

    *map = (bh_memory_descriptor_t*)MemoryMap;
    *map_size = MemoryMapSize;
    *descriptor_size = DescriptorSize;
    return BH_SUCCESS;
}

STATIC bh_status_t bh_get_graphics_info(bh_graphics_info_t* info) {
    if (!info) return BH_INVALID_ARGUMENT;

    if (CorebootGetFramebuffer()) {
        CONST COREBOOT_FB* fb = CorebootGetFramebuffer();
        info->framebuffer = (void*)fb->physical_address;
        info->width = fb->x_resolution;
        info->height = fb->y_resolution;
        info->bpp = fb->bits_per_pixel;
        return BH_SUCCESS;
    }

    return BH_NOT_FOUND;
}

STATIC VOID* bh_get_rsdp(VOID) {
    EFI_CONFIGURATION_TABLE* ConfigTable = gST->ConfigurationTable;
    EFI_GUID AcpiTableGuid = ACPI_TABLE_GUID;

    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &AcpiTableGuid)) {
            return ConfigTable[i].VendorTable;
        }
    }
    return NULL;
}

STATIC VOID* bh_get_boot_device(VOID) {
    if (!gBS || !gImageHandle) return NULL;

    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_STATUS Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) return NULL;

    return LoadedImage->DeviceHandle;
}

STATIC VOID bh_reboot(VOID) {
    if (gRT && gRT->ResetSystem) {
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
}

STATIC VOID bh_shutdown(VOID) {
    if (gRT && gRT->ResetSystem) {
        gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
}

STATIC VOID bh_debug_break(VOID) {
    if (gBS) {
        gBS->Stall(1000000); // 1 second stall for debugging
    }
}
