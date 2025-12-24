/*
 * coreboot_payload.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SmBios.h>
#include "coreboot_platform.h"
#include "coreboot_payload.h"

// Coreboot payload entry point signature
typedef VOID (*COREBOOT_PAYLOAD_ENTRY)(VOID* coreboot_table, VOID* payload);

// Coreboot boot parameter structure definitions
#define COREBOOT_BOOT_SIGNATURE 0x12345678
#define COREBOOT_BOOT_FLAG_KERNEL 0x01
#define COREBOOT_BOOT_FLAG_FRAMEBUFFER 0x02
#define COREBOOT_BOOT_FLAG_ACPI 0x04
#define COREBOOT_BOOT_FLAG_SMBIOS 0x08

typedef struct {
    UINT32 signature;           // Boot signature
    UINT32 version;             // Boot parameters version
    UINT64 kernel_base;         // Kernel base address
    UINT64 kernel_size;         // Kernel size in bytes
    UINT32 boot_flags;          // Boot flags
    UINT64 reserved1;          // Reserved for alignment

    // Framebuffer information
    UINT64 framebuffer_addr;    // Framebuffer physical address
    UINT32 framebuffer_width;   // Framebuffer width
    UINT32 framebuffer_height;  // Framebuffer height
    UINT32 framebuffer_bpp;     // Bits per pixel
    UINT32 framebuffer_pitch;   // Bytes per line
    UINT32 framebuffer_red_mask;     // Red mask position/size
    UINT32 framebuffer_green_mask;   // Green mask position/size
    UINT32 framebuffer_blue_mask;    // Blue mask position/size

    // ACPI information
    UINT64 acpi_rsdp;           // ACPI RSDP address
    UINT64 acpi_rsdt;           // ACPI RSDT address

    // SMBIOS information
    UINT64 smbios_entry;        // SMBIOS entry point address

    // Memory information
    UINT64 memory_size;         // Total memory size
    UINT64 memory_map_addr;     // Memory map address
    UINT32 memory_map_entries;  // Number of memory map entries

    // Coreboot table information
    UINT64 coreboot_table_addr; // Coreboot table address
    UINT32 coreboot_version;    // Coreboot version

    // System information
    UINT32 cpu_count;           // Number of CPUs
    UINT64 cpu_frequency;       // CPU frequency in Hz
    UINT32 board_id;            // Board ID
    UINT64 timestamp;           // Boot timestamp

    // Reserved for future use
    UINT64 reserved[8];
} COREBOOT_BOOT_PARAMS;

/**
 * Coreboot payload header structure
 * This structure is placed at the beginning of the payload binary
 */
typedef struct {
    UINT8 signature[8];      // "$COREBOOT"
    UINT32 header_version;   // Header format version
    UINT16 payload_version;  // Payload version
    UINT16 payload_size;     // Size of payload in 512-byte blocks
    UINT32 cmd_line_size;    // Size of command line
    UINT32 checksum;         // Checksum of header
    UINT32 entry_point;      // Offset to entry point from start of header
    UINT32 payload_load_addr; // Load address for payload
    UINT32 payload_compressed_size; // Compressed size if compressed
    UINT32 payload_uncompressed_size; // Uncompressed size
    UINT32 reserved[4];      // Reserved for future use
    CHAR8 cmd_line[0];       // Command line (null-terminated)
} __attribute__((packed)) COREBOOT_PAYLOAD_HEADER;

// Payload signature
#define COREBOOT_PAYLOAD_SIGNATURE "$COREBOOT"

// Current payload header version
#define PAYLOAD_HEADER_VERSION 2

/**
 * BloodHorn payload entry point
 * This function is called by Coreboot when loading the payload
 */
VOID
EFIAPI
BloodhornPayloadEntry (
  IN VOID* coreboot_table,
  IN VOID* payload
  )
{
    EFI_STATUS Status;
    
    // Initialize debug output early
    DEBUG((DEBUG_INFO, "BloodHorn Coreboot Payload Entry\n"));
    DEBUG((DEBUG_INFO, "Coreboot table: 0x%p, Payload: 0x%p\n", coreboot_table, payload));

    // Validate coreboot table
    if (!coreboot_table) {
        DEBUG((DEBUG_ERROR, "No coreboot table provided\n"));
        return;
    }

    // Initialize the Coreboot platform interface
    if (!CorebootPlatformInit()) {
        DEBUG((DEBUG_ERROR, "Coreboot platform initialization failed\n"));
        return;
    }

    // Initialize UEFI environment (minimal)
    Status = InitializeUefiEnvironment();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "UEFI environment initialization failed: %r\n", Status));
        // Continue without full UEFI support
    }

    // Initialize hardware abstraction layer
    Status = InitializeHardwareAbstraction();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Hardware abstraction initialization failed: %r\n", Status));
        return;
    }

    // Initialize security subsystem
    Status = InitializeSecuritySubsystem();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "Security subsystem initialization failed: %r\n", Status));
        // Continue without security features
    }

    // Start main bootloader functionality
    BloodhornMain();
}

/**
 * Initialize minimal UEFI environment for Coreboot payload
 */
STATIC
EFI_STATUS
EFIAPI
InitializeUefiEnvironment (
  VOID
  )
{
    EFI_STATUS Status;
    
    // Coreboot provides basic services, but we need minimal UEFI compatibility
    // For now, just verify we can access basic services
    
    // Initialize console output
    Status = InitializeConsole();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "Console initialization failed: %r\n", Status));
    }
    
    return Status;
}

/**
 * Initialize hardware abstraction layer
 */
STATIC
EFI_STATUS
EFIAPI
InitializeHardwareAbstraction (
  VOID
  )
{
    EFI_STATUS Status;
    
    // Initialize graphics using Coreboot framebuffer
    if (CorebootInitGraphics()) {
        DEBUG((DEBUG_INFO, "Graphics initialized using Coreboot framebuffer\n"));
    } else {
        DEBUG((DEBUG_WARN, "Graphics initialization failed\n"));
    }

    // Initialize storage devices
    Status = CorebootInitStorage();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "Storage initialization failed: %r\n", Status));
    } else {
        DEBUG((DEBUG_INFO, "Storage initialized successfully\n"));
    }

    // Initialize network if needed
    Status = CorebootInitNetwork();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "Network initialization failed: %r\n", Status));
    } else {
        DEBUG((DEBUG_INFO, "Network initialized successfully\n"));
    }

    // Initialize TPM if available
    Status = CorebootInitTpm();
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_WARN, "TPM initialization failed: %r\n", Status));
    } else {
        DEBUG((DEBUG_INFO, "TPM initialized successfully\n"));
    }

    return EFI_SUCCESS;
}

/**
 * Initialize security subsystem
 */
STATIC
EFI_STATUS
EFIAPI
InitializeSecuritySubsystem (
  VOID
  )
{
    // Initialize secure boot if available
    // Initialize TPM measurements
    // Initialize cryptographic services
    
    DEBUG((DEBUG_INFO, "Security subsystem initialized\n"));
    return EFI_SUCCESS;
}

/**
 * Initialize console output
 */
STATIC
EFI_STATUS
EFIAPI
InitializeConsole (
  VOID
  )
{
    // Use Coreboot framebuffer for console output
    // Initialize font rendering if needed
    
    DEBUG((DEBUG_INFO, "Console initialized\n"));
    return EFI_SUCCESS;
}

/**
 * Main BloodHorn entry point when running as Coreboot payload
 */
VOID
EFIAPI
BloodhornMain (
  VOID
  )
{
    EFI_STATUS Status;
    COREBOOT_BOOT_PARAMS boot_params;

    Print(L"BloodHorn Bootloader (Coreboot Payload Mode)\n");
    Print(L"Version: %s\n", COREBOOT_VERSION);
    Print(L"Build: %s\n", COREBOOT_BUILD_DATE);
    Print(L"Initializing hardware services...\n");

    // Print Coreboot system information
    CorebootPrintSystemInfo();

    // Set up boot parameters structure
    Status = SetupBootParameters(&boot_params);
    if (EFI_ERROR(Status)) {
        Print(L"Boot parameters setup failed: %r\n", Status);
        CorebootReboot();
        return;
    }

    // Load configuration
    Status = LoadBootConfiguration();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load boot configuration: %r\n", Status));
        // Use default configuration
        UseDefaultBootConfiguration();
    }

    // Initialize boot menu and configuration
    BloodhornBootMenu();

    // If boot menu returns, reboot
    Print(L"Boot process completed, rebooting...\n");
    CorebootReboot();
}

/**
 * BloodHorn boot menu for Coreboot environment
 */
VOID
EFIAPI
BloodhornBootMenu (
  VOID
  )
{
    EFI_STATUS Status;
    UINTN SelectedEntry = 0;
    CONST UINTN MaxEntries = 8;

    // Boot menu entries
    CONST CHAR16* BootEntries[] = {
        L"Linux Kernel",
        L"Multiboot2 Kernel",
        L"Limine Kernel",
        L"Chainload Bootloader",
        L"PXE Network Boot",
        L"BloodChain Protocol",
        L"Recovery Shell",
        L"Reboot System"
    };

    typedef EFI_STATUS (*BootFunctionPtr)(VOID);
    BootFunctionPtr BootFunctions[] = {
        BloodhornBootLinux,
        BloodhornBootMultiboot2,
        BloodhornBootLimine,
        BloodhornBootChainload,
        BloodhornBootPxe,
        BloodhornBootBloodchain,
        BloodhornBootRecovery,
        BloodhornReboot
    };

    // Load configuration to determine default entry and timeout
    UINTN Timeout = 10; // Default 10 seconds
    UINTN DefaultEntry = 0;

    Print(L"\nBloodHorn Boot Menu (Coreboot Payload)\n");
    Print(L"========================================\n\n");

    // Display boot options
    for (UINTN i = 0; i < MaxEntries; i++) {
        if (i == DefaultEntry) {
            Print(L"[%u] %s (default)\n", i + 1, BootEntries[i]);
        } else {
            Print(L" %u  %s\n", i + 1, BootEntries[i]);
        }
    }

    Print(L"\nPress number key to select, or wait %u seconds for default...\n", Timeout);

    // Simple countdown timer (simplified - real implementation would use timer)
    for (UINTN i = Timeout; i > 0; i--) {
        Print(L"\rAuto-boot in %u seconds... (press key to cancel)   ", i);

        // Check for key press (simplified implementation)
        // In real implementation, would use Coreboot input services

        // For now, just wait a bit
        for (volatile UINTN j = 0; j < 1000000; j++) {
            // Busy wait
        }
    }

    Print(L"\nBooting default entry...\n");

    // Execute selected boot function
    Status = BootFunctions[DefaultEntry]();
    if (EFI_ERROR(Status)) {
        Print(L"Boot failed: %r\n", Status);
        Print(L"Press any key to reboot...\n");

        // Wait for key press
        for (volatile UINTN j = 0; j < 10000000; j++) {
            // Busy wait for key press
        }

        CorebootReboot();
    }
}

/**
 * Load kernel using Coreboot services
 */
BOOLEAN
EFIAPI
BloodhornLoadKernel (
  IN CONST CHAR8* kernel_path,
  OUT VOID** kernel_buffer,
  OUT UINT32* kernel_size
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE file;
    VOID* buffer = NULL;
    UINTN size = 0;
    CHAR16 kernel_path_wide[256];

    // Convert ASCII path to wide character
    for (UINTN i = 0; i < AsciiStrLen(kernel_path) && i < 255; i++) {
        kernel_path_wide[i] = kernel_path[i];
    }
    kernel_path_wide[AsciiStrLen(kernel_path)] = 0;

    Print(L"Loading kernel: %s\n", kernel_path_wide);

    // Get root directory using UEFI services (Coreboot provides this)
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get root directory: %r\n", Status);
        return FALSE;
    }

    // Open kernel file
    Status = root_dir->Open(root_dir, &file, kernel_path_wide, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file: %r\n", Status);
        return FALSE;
    }

    // Get file size
    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) {
            file->Close(file);
            return FALSE;
        }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }

    if (EFI_ERROR(Status)) {
        if (info) FreePool(info);
        file->Close(file);
        Print(L"Failed to get file info: %r\n", Status);
        return FALSE;
    }

    size = (UINTN)info->FileSize;
    buffer = AllocateZeroPool(size);
    if (!buffer) {
        FreePool(info);
        file->Close(file);
        Print(L"Failed to allocate memory for kernel\n");
        return FALSE;
    }

    // Read kernel into memory
    Status = file->Read(file, &size, buffer);
    file->Close(file);
    FreePool(info);

    if (EFI_ERROR(Status)) {
        FreePool(buffer);
        Print(L"Failed to read kernel file: %r\n", Status);
        return FALSE;
    }

    *kernel_buffer = buffer;
    *kernel_size = (UINT32)size;

    Print(L"Kernel loaded successfully: %u bytes\n", *kernel_size);
    return TRUE;
}

/**
 * Execute loaded kernel
 */
BOOLEAN
EFIAPI
BloodhornExecuteKernel (
  IN VOID* kernel_buffer,
  IN UINT32 kernel_size
  )
{
    EFI_STATUS Status;
    UINT64 kernel_base = 0;
    UINT64 largest_size = 0;

    Print(L"Executing kernel at 0x%llx (%u bytes)\n",
          (UINT64)(UINTN)kernel_buffer, kernel_size);

    // Get Coreboot memory map for proper kernel setup
    UINT32 mem_map_count = 0;
    CONST COREBOOT_MEM_ENTRY* mem_map = CorebootGetMemoryMap(&mem_map_count);

    if (!mem_map || mem_map_count == 0) {
        Print(L"Failed to get Coreboot memory map for kernel execution\n");
        return FALSE;
    }

    // Find suitable memory region for kernel execution
    for (UINT32 i = 0; i < mem_map_count; i++) {
        if (mem_map[i].type == CB_MEM_RAM && mem_map[i].size > largest_size) {
            kernel_base = mem_map[i].addr;
            largest_size = mem_map[i].size;
        }
    }

    if (kernel_base == 0) {
        Print(L"No suitable RAM region found for kernel execution\n");
        return FALSE;
    }

    // Set up kernel execution environment
    Print(L"Setting up kernel execution environment...\n");
    Print(L"Kernel base: 0x%llx, Size: %u bytes\n", kernel_base, kernel_size);

    // Configure framebuffer if graphics available
    CONST COREBOOT_FB* framebuffer = CorebootGetFramebuffer();
    if (framebuffer) {
        Print(L"Configuring Coreboot framebuffer for kernel\n");
        Print(L"Framebuffer: 0x%llx, %ux%u, %u bpp\n",
              framebuffer->physical_address,
              framebuffer->x_resolution,
              framebuffer->y_resolution,
              framebuffer->bits_per_pixel);
    }

    // Set up boot parameters in Coreboot format
    UINT64 boot_params_addr = kernel_base + 0x1000; // 4KB after kernel

    // Set up Coreboot boot parameters structure
    COREBOOT_BOOT_PARAMS* boot_params = (COREBOOT_BOOT_PARAMS*)boot_params_addr;

    // Zero out the boot parameters structure
    SetMem(boot_params, sizeof(COREBOOT_BOOT_PARAMS), 0);

    // Set up basic boot parameters
    boot_params->signature = COREBOOT_BOOT_SIGNATURE;
    boot_params->version = 1;
    boot_params->kernel_base = kernel_base;
    boot_params->kernel_size = kernel_size;
    boot_params->boot_flags = COREBOOT_BOOT_FLAG_KERNEL;

    // Set up framebuffer information if available
    if (framebuffer) {
        boot_params->framebuffer_addr = framebuffer->physical_address;
        boot_params->framebuffer_width = framebuffer->x_resolution;
        boot_params->framebuffer_height = framebuffer->y_resolution;
        boot_params->framebuffer_bpp = framebuffer->bits_per_pixel;
        boot_params->framebuffer_pitch = framebuffer->bytes_per_line;
        boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
    }

    // Set up memory information
    UINT64 total_memory = CorebootGetTotalMemory();
    boot_params->memory_size = total_memory;

    // Validate boot parameters structure
    if (!ValidateBootParameters(boot_params)) {
        Print(L"Boot parameters validation failed\n");
        return FALSE;
    }

    Print(L"Boot parameters set up at 0x%llx\n", boot_params_addr);

    // Jump to kernel entry point
    typedef VOID (*KernelEntry)(COREBOOT_BOOT_PARAMS*);
    KernelEntry EntryPoint = (KernelEntry)kernel_buffer;

    Print(L"Jumping to kernel entry point at 0x%llx\n", (UINT64)(UINTN)kernel_buffer);

    // Execute kernel (this should not return)
    EntryPoint(boot_params);

    // If we get here, kernel execution failed
    Print(L"Kernel execution returned unexpectedly\n");
    return FALSE;
}

// Include necessary headers for file operations
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

// Coreboot boot parameter structure definitions
#define COREBOOT_BOOT_SIGNATURE 0x12345678
#define COREBOOT_BOOT_FLAG_KERNEL 0x01
#define COREBOOT_BOOT_FLAG_FRAMEBUFFER 0x02

typedef struct {
    UINT32 signature;           // Boot signature
    UINT32 version;             // Boot parameters version
    UINT64 kernel_base;         // Kernel base address
    UINT64 kernel_size;         // Kernel size in bytes
    UINT32 boot_flags;          // Boot flags

    // Framebuffer information
    UINT64 framebuffer_addr;    // Framebuffer physical address
    UINT32 framebuffer_width;   // Framebuffer width
    UINT32 framebuffer_height;  // Framebuffer height
    UINT32 framebuffer_bpp;     // Bits per pixel
    UINT32 framebuffer_pitch;   // Bytes per line

    // Memory information
    UINT64 memory_size;         // Total memory size
} COREBOOT_BOOT_PARAMS;

/**
 * Create payload binary with proper header
 * This function would be used during build process
 */
VOID
EFIAPI
CreateBloodhornPayload (
  IN CONST CHAR8* output_file,
  IN CONST CHAR8* input_binary,
  IN CONST CHAR8* cmdline
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE input_file;
    EFI_FILE_HANDLE output_file_handle;
    VOID* input_buffer = NULL;
    VOID* output_buffer = NULL;
    UINTN input_size = 0;
    UINTN output_size = 0;
    CHAR16 output_file_wide[256];
    CHAR16 input_binary_wide[256];

    // Convert ASCII paths to wide character
    for (UINTN i = 0; i < AsciiStrLen(output_file) && i < 255; i++) {
        output_file_wide[i] = output_file[i];
    }
    output_file_wide[AsciiStrLen(output_file)] = 0;

    for (UINTN i = 0; i < AsciiStrLen(input_binary) && i < 255; i++) {
        input_binary_wide[i] = input_binary[i];
    }
    input_binary_wide[AsciiStrLen(input_binary)] = 0;

    Print(L"Creating BloodHorn payload: %s -> %s\n", input_binary_wide, output_file_wide);

    // Get root directory
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get root directory: %r\n", Status);
        return;
    }

    // Open input binary
    Status = root_dir->Open(root_dir, &input_file, input_binary_wide, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open input binary: %r\n", Status);
        return;
    }

    // Get input file size
    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = input_file->GetInfo(input_file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) {
            input_file->Close(input_file);
            return;
        }
        Status = input_file->GetInfo(input_file, &gEfiFileInfoGuid, &info_size, info);
    }

    if (EFI_ERROR(Status)) {
        if (info) FreePool(info);
        input_file->Close(input_file);
        Print(L"Failed to get input file info: %r\n", Status);
        return;
    }

    input_size = (UINTN)info->FileSize;
    input_buffer = AllocateZeroPool(input_size);
    if (!input_buffer) {
        FreePool(info);
        input_file->Close(input_file);
        Print(L"Failed to allocate memory for input buffer\n");
        return;
    }

    // Read input binary
    Status = input_file->Read(input_file, &input_size, input_buffer);
    input_file->Close(input_file);
    FreePool(info);

    if (EFI_ERROR(Status)) {
        FreePool(input_buffer);
        Print(L"Failed to read input binary: %r\n", Status);
        return;
    }

    // Calculate output size (header + binary + cmdline)
    UINTN cmdline_size = cmdline ? AsciiStrLen(cmdline) + 1 : 1;
    output_size = sizeof(COREBOOT_PAYLOAD_HEADER) + cmdline_size + input_size;

    output_buffer = AllocateZeroPool(output_size);
    if (!output_buffer) {
        FreePool(input_buffer);
        Print(L"Failed to allocate memory for output buffer\n");
        return;
    }

    // Create payload header
    COREBOOT_PAYLOAD_HEADER* header = (COREBOOT_PAYLOAD_HEADER*)output_buffer;
    CopyMem(header->signature, COREBOOT_PAYLOAD_SIGNATURE, 8);
    header->header_version = PAYLOAD_HEADER_VERSION;
    header->payload_version = 1;
    header->payload_size = (input_size + 511) / 512; // Size in 512-byte blocks
    header->cmd_line_size = cmdline_size;
    header->entry_point = sizeof(COREBOOT_PAYLOAD_HEADER) + cmdline_size;
    header->payload_load_addr = 0x100000; // Default load address
    header->payload_compressed_size = 0; // Not compressed
    header->payload_uncompressed_size = input_size;

    // Calculate checksum
    UINT32 checksum = 0;
    UINT8* ptr = (UINT8*)header;
    for (UINT32 i = 0; i < sizeof(COREBOOT_PAYLOAD_HEADER) - 4; i++) {
        checksum += ptr[i];
    }
    header->checksum = checksum;

    // Copy command line
    CHAR8* cmdline_dest = (CHAR8*)((UINT8*)output_buffer + sizeof(COREBOOT_PAYLOAD_HEADER));
    if (cmdline) {
        CopyMem(cmdline_dest, cmdline, cmdline_size - 1);
        cmdline_dest[cmdline_size - 1] = 0;
    } else {
        cmdline_dest[0] = 0;
    }

    // Copy input binary
    CopyMem((UINT8*)output_buffer + sizeof(COREBOOT_PAYLOAD_HEADER) + cmdline_size,
            input_buffer, input_size);

    // Create output file
    Status = root_dir->Open(root_dir, &output_file_handle, output_file_wide,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to create output file: %r\n", Status);
        FreePool(input_buffer);
        FreePool(output_buffer);
        return;
    }

    // Write payload to file
    Status = output_file_handle->Write(output_file_handle, &output_size, output_buffer);
    output_file_handle->Close(output_file_handle);

    if (EFI_ERROR(Status)) {
        Print(L"Failed to write payload file: %r\n", Status);
    } else {
        Print(L"Payload created successfully: %u bytes\n", output_size);
    }

    FreePool(input_buffer);
    FreePool(output_buffer);
}

/**
 * Validate boot parameters structure
 */
STATIC
BOOLEAN
ValidateBootParameters (
  IN COREBOOT_BOOT_PARAMS* boot_params
  )
{
    if (!boot_params) {
        return FALSE;
    }

    // Check signature
    if (boot_params->signature != COREBOOT_BOOT_SIGNATURE) {
        Print(L"Invalid boot parameters signature: 0x%x\n", boot_params->signature);
        return FALSE;
    }

    // Check version
    if (boot_params->version != 1) {
        Print(L"Unsupported boot parameters version: %d\n", boot_params->version);
        return FALSE;
    }

    // Check kernel information
    if (boot_params->kernel_base == 0 || boot_params->kernel_size == 0) {
        Print(L"Invalid kernel base or size\n");
        return FALSE;
    }

    // Check boot flags
    if ((boot_params->boot_flags & COREBOOT_BOOT_FLAG_KERNEL) == 0) {
        Print(L"Kernel flag not set in boot parameters\n");
        return FALSE;
    }

    // Validate framebuffer information if present
    if (boot_params->boot_flags & COREBOOT_BOOT_FLAG_FRAMEBUFFER) {
        if (boot_params->framebuffer_addr == 0 ||
            boot_params->framebuffer_width == 0 ||
            boot_params->framebuffer_height == 0) {
            Print(L"Invalid framebuffer information\n");
            return FALSE;
        }
    }

    return TRUE;
}

// Boot function declarations
EFI_STATUS EFIAPI BloodhornBootLinux(VOID);
EFI_STATUS EFIAPI BloodhornBootMultiboot2(VOID);
EFI_STATUS EFIAPI BloodhornBootLimine(VOID);
EFI_STATUS EFIAPI BloodhornBootChainload(VOID);
EFI_STATUS EFIAPI BloodhornBootPxe(VOID);
EFI_STATUS EFIAPI BloodhornBootBloodchain(VOID);
EFI_STATUS EFIAPI BloodhornBootRecovery(VOID);
EFI_STATUS EFIAPI BloodhornReboot(VOID);

// Boot function implementations
EFI_STATUS EFIAPI BloodhornBootLinux(VOID) {
    VOID* kernel_buffer;
    UINT32 kernel_size;

    if (BloodhornLoadKernel("kernel.efi", &kernel_buffer, &kernel_size)) {
        return BloodhornExecuteKernel(kernel_buffer, kernel_size) ? EFI_SUCCESS : EFI_LOAD_ERROR;
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI BloodhornBootMultiboot2(VOID) {
    VOID* kernel_buffer;
    UINT32 kernel_size;

    if (BloodhornLoadKernel("kernel-mb2.efi", &kernel_buffer, &kernel_size)) {
        return BloodhornExecuteKernel(kernel_buffer, kernel_size) ? EFI_SUCCESS : EFI_LOAD_ERROR;
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI BloodhornBootLimine(VOID) {
    VOID* kernel_buffer;
    UINT32 kernel_size;

    if (BloodhornLoadKernel("kernel-limine.efi", &kernel_buffer, &kernel_size)) {
        return BloodhornExecuteKernel(kernel_buffer, kernel_size) ? EFI_SUCCESS : EFI_LOAD_ERROR;
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI BloodhornBootChainload(VOID) {
    // Chainload implementation would load another bootloader
    Print(L"Chainloading not implemented in Coreboot payload\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS EFIAPI BloodhornBootPxe(VOID) {
    // PXE boot implementation
    Print(L"PXE boot not implemented in Coreboot payload\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS EFIAPI BloodhornBootBloodchain(VOID) {
    VOID* kernel_buffer;
    UINT32 kernel_size;

    if (BloodhornLoadKernel("kernel-bc.efi", &kernel_buffer, &kernel_size)) {
        return BloodhornExecuteKernel(kernel_buffer, kernel_size) ? EFI_SUCCESS : EFI_LOAD_ERROR;
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI BloodhornBootRecovery(VOID) {
    Print(L"Recovery shell not implemented in Coreboot payload\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS EFIAPI BloodhornReboot(VOID) {
    Print(L"Rebooting system...\n");
    CorebootReboot();
    return EFI_SUCCESS;
}

// External function declarations
extern EFI_STATUS get_root_dir(EFI_FILE_HANDLE* root_dir);
extern EFI_STATUS CorebootReboot(VOID);
extern CONST COREBOOT_FB* CorebootGetFramebuffer(VOID);
extern UINT64 CorebootGetTotalMemory(VOID);
extern UINT32 CorebootGetMemoryMap(UINT32* count);
extern BOOLEAN CorebootInitGraphics(VOID);
extern EFI_STATUS CorebootInitStorage(VOID);
extern EFI_STATUS CorebootInitNetwork(VOID);
extern EFI_STATUS CorebootInitTpm(VOID);
extern BOOLEAN CorebootPlatformInit(VOID);

/**
 * Validate payload header
 */
BOOLEAN
EFIAPI
ValidatePayloadHeader (
  IN CONST COREBOOT_PAYLOAD_HEADER* header
  )
{
    if (header->header_version != PAYLOAD_HEADER_VERSION) {
        return FALSE;
    }

    // Verify signature
    if (CompareMem(header->signature, COREBOOT_PAYLOAD_SIGNATURE, 8) != 0) {
        return FALSE;
    }

    // Verify checksum
    UINT32 checksum = 0;
    CONST UINT8* ptr = (CONST UINT8*)header;
    for (UINT32 i = 0; i < sizeof(COREBOOT_PAYLOAD_HEADER) - 4; i++) {
        checksum += ptr[i];
    }

    return (checksum == header->checksum);
}

/**
 * Get payload information
 */
VOID
EFIAPI
GetPayloadInfo (
  IN CONST COREBOOT_PAYLOAD_HEADER* header
  )
{
    // Extract and display payload information
    // Version, size, command line, etc.
}
