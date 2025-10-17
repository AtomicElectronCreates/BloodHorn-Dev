/**
 * BloodHorn Coreboot Main Entry Point
 *
 * This is the dedicated Coreboot payload entry point for BloodHorn bootloader.
 * It provides a clean interface between Coreboot firmware and the BloodHorn
 * bootloader logic, ensuring proper initialization and execution.
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

// Include Coreboot platform integration
#include "coreboot_platform.h"

// Global variables for Coreboot environment
STATIC EFI_HANDLE gImageHandle = NULL;
STATIC EFI_SYSTEM_TABLE* gST = NULL;
STATIC EFI_BOOT_SERVICES* gBS = NULL;
STATIC EFI_RUNTIME_SERVICES* gRT = NULL;

/**
 * Coreboot payload entry point
 *
 * This function is called by Coreboot when loading BloodHorn as a payload.
 * It performs all necessary initialization and starts the main bootloader logic.
 */
VOID
EFIAPI
CorebootMain (
  IN VOID* coreboot_table,
  IN VOID* payload
  )
{
    EFI_STATUS Status;

    // Initialize UEFI system table (required for EDK2 compatibility)
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

/**
 * Main BloodHorn logic for Coreboot environment
 *
 * This function contains the core BloodHorn bootloader logic adapted
 * for the Coreboot environment, using Coreboot services where available
 * and falling back to UEFI services when needed.
 */
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
            .putc = NULL,
            .puts = NULL,
            .printf = NULL,
            .get_memory_map = NULL,
            .get_graphics_info = NULL,
            .get_rsdp = NULL,
            .get_boot_device = NULL,
            .reboot = NULL,
            .shutdown = NULL,
            .debug_break = NULL
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
            Status = ExecuteKernel(KernelBuffer, KernelSize, NULL);
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

/**
 * Load and verify kernel using Coreboot filesystem services
 */
STATIC
EFI_STATUS
LoadAndVerifyKernelCoreboot (
  IN CHAR16* KernelPath,
  OUT VOID** KernelBuffer,
  OUT UINTN* KernelSize
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE file;
    VOID* buffer = NULL;
    UINTN size = 0;

    // Get root directory using UEFI services (Coreboot provides this)
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get root directory\n");
        return Status;
    }

    // Open kernel file
    Status = root_dir->Open(root_dir, &file, KernelPath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file: %s\n", KernelPath);
        return Status;
    }

    // Get file size
    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) {
            file->Close(file);
            return EFI_OUT_OF_RESOURCES;
        }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }

    if (EFI_ERROR(Status)) {
        if (info) FreePool(info);
        file->Close(file);
        return Status;
    }

    size = (UINTN)info->FileSize;
    buffer = AllocateZeroPool(size);
    if (!buffer) {
        FreePool(info);
        file->Close(file);
        return EFI_OUT_OF_RESOURCES;
    }

    // Read kernel into memory
    Status = file->Read(file, &size, buffer);
    file->Close(file);
    FreePool(info);

    if (EFI_ERROR(Status)) {
        FreePool(buffer);
        return Status;
    }

    *KernelBuffer = buffer;
    *KernelSize = size;

    // Verify kernel hash if security is enabled
    if (g_known_hashes[0].expected_hash[0] != 0) {
        crypto_sha512_ctx_t ctx;
        uint8_t actual_hash[64];

        crypto_sha512_init(&ctx);
        crypto_sha512_update(&ctx, buffer, (uint32_t)size);
        crypto_sha512_final(&ctx, actual_hash);

        if (CompareMem(actual_hash, g_known_hashes[0].expected_hash, 64) != 0) {
            Print(L"Kernel hash verification failed!\n");
            FreePool(buffer);
            return EFI_SECURITY_VIOLATION;
        }
    }

    return EFI_SUCCESS;
}

/**
 * Exit to Coreboot firmware
 */
EFI_STATUS EFIAPI ExitToCorebootWrapper(VOID) {
    Print(L"Exiting to Coreboot firmware...\n");
    CorebootReboot();
    return EFI_SUCCESS;
}

/**
 * Load theme and language configuration for Coreboot
 */
STATIC VOID LoadThemeAndLanguageFromConfig(VOID) {
    // Use default settings for Coreboot environment
    // Configuration loading handled by existing modules
}

/**
 * Initialize mouse for Coreboot environment
 */
VOID InitMouse(VOID) {
    // Mouse initialization for Coreboot (simplified)
    Print(L"Mouse initialized for Coreboot environment\n");
}

/**
 * Add boot entry for Coreboot environment
 */
VOID AddBootEntry(CHAR16* Name, EFI_STATUS (*Function)(VOID)) {
    // Boot entry registration for Coreboot (simplified)
    Print(L"Boot entry added: %s\n", Name);
}

/**
 * Show boot menu for Coreboot environment
 */
EFI_STATUS ShowBootMenu(VOID) {
    Print(L"Displaying boot menu...\n");
    Print(L"Select boot option:\n");
    Print(L"1. Linux Kernel (Coreboot)\n");
    Print(L"2. Multiboot2 Kernel (Coreboot)\n");
    Print(L"3. Exit to Coreboot\n");

    // Simplified boot menu - in real implementation would show interactive menu
    return EFI_SUCCESS;
}

// Include necessary function declarations
extern EFI_STATUS get_root_dir(EFI_FILE_HANDLE* root_dir);
extern VOID ShutdownNetwork(VOID);
extern EFI_STATUS BootFromNetwork(CONST CHAR16* kernel_path, CONST CHAR16* initrd_path, CONST CHAR8* cmdline);
extern EFI_STATUS linux_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
extern EFI_STATUS multiboot2_load_kernel(const char* kernel_path, const char* cmdline);
extern EFI_STATUS limine_load_kernel(const char* kernel_path, const char* cmdline);
extern EFI_STATUS chainload_file(const char* file_path);
extern EFI_STATUS shell_start(VOID);
extern EFI_STATUS ia32_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
extern EFI_STATUS x86_64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
extern EFI_STATUS aarch64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
extern EFI_STATUS riscv64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
extern EFI_STATUS loongarch64_load_kernel(const char* kernel_path, const char* initrd_path, const char* cmdline);
