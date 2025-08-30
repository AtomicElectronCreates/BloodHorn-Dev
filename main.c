#include <Uefi.h>
#include "compat.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Tcg2Protocol.h>  // For TPM 2.0 support
#include "boot/menu.h"
#include "boot/theme.h"
#include "boot/localization.h"
#include "boot/mouse.h"
#include "boot/secure.h"
#include "fs/fat32.h"
#include "security/crypto.h"
#include "security/tpm2.h"       // TPM 2.0 support
#include "scripting/lua.h"
#include "recovery/shell.h"
#include "plugins/plugin.h"
#include "net/pxe.h"
#include "net/network.hpp"
#include "boot/Arch32/linux.h"
#include "boot/Arch32/limine.h"
#include "boot/Arch32/multiboot1.h"
#include "boot/Arch32/multiboot2.h"
#include "boot/Arch32/chainload.h"
#include "boot/Arch32/ia32.h"
#include "boot/Arch32/x86_64.h"
#include "boot/Arch32/aarch64.h"
#include "boot/Arch32/riscv64.h"
#include "boot/Arch32/loongarch64.h"
#include "boot/Arch32/BloodChain/bloodchain.h"
#include "config/config_ini.h"
#include "config/config_json.h"
#include "config/config_env.h"
#include "boot/libb/include/bloodhorn/bloodhorn.h"
#include "boot/libb/include/bloodhorn/graphics.h"
#include "boot/libb/include/bloodhorn/input.h"
#include "boot/libb/include/bloodhorn/filesystem.h"
#include "security/sha512.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global TPM 2.0 context
static EFI_TCG2_PROTOCOL* gTcg2Protocol = NULL;

// Global BloodHorn context
static bh_context_t* gBhContext = NULL;

// Global font handles
static bh_font_t gDefaultFont = {0};
static bh_font_t gHeaderFont = {0};

// Global settings
struct boot_config {
    char default_entry[64];
    int menu_timeout;
    char kernel[128];
    char initrd[128];
    char cmdline[256];
    bool tpm_enabled;             // TPM 2.0 support
    bool secure_boot;             // Secure boot status
    bool use_gui;                 // Use graphical interface
    char font_path[256];          // Path to custom font
    uint32_t font_size;           // Base font size
    uint32_t header_font_size;    // Header font size
    char language[8];             // UI language
    bool enable_networking;       // Enable networking
};

// TPM 2.0 measurement structure
typedef struct {
    uint8_t pcr_index;
    uint32_t event_type;
    uint8_t digest[32];
    uint32_t digest_size;
    uint8_t* event_data;
    uint32_t event_size;
} tpm_measurement_t;

// Structure to store file hashes
typedef struct {
    char path[256];
    uint8_t expected_hash[64]; // SHA-512 produces 64-byte hashes
} file_hash_t;

// Known good hashes for critical boot files
static file_hash_t g_known_hashes[] = {
    {"/boot/kernel.efi", {0}},  // Will be populated at runtime
    {"/boot/initrd.img", {0}},  // Will be populated from config
    // Add more files as needed
};

/**
 * @brief Verify file hash against expected value
 * @param file_path Path to the file to verify
 * @param expected_hash Expected SHA-512 hash (64 bytes)
 * @return EFI_SUCCESS if hashes match, error code otherwise
 */
static EFI_STATUS verify_file_hash(const char* file_path, const uint8_t* expected_hash) {
    EFI_STATUS Status;
    EFI_FILE* file = NULL;
    EFI_FILE_HANDLE root_dir;
    UINT8 file_buffer[4096];
    UINTN read_size;
    crypto_sha512_ctx_t ctx;
    uint8_t actual_hash[64];
    
    if (!file_path || !expected_hash) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Open the file
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Failed to get root directory\n"));
        return Status;
    }
    
    Status = root_dir->Open(root_dir, &file, (CHAR16*)file_path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Failed to open file %a: %r\n", file_path, Status));
        return Status;
    }
    
    // Initialize hash
    crypto_sha512_init(&ctx);
    
    // Read file and update hash
    while (1) {
        read_size = sizeof(file_buffer);
        Status = file->Read(file, &read_size, file_buffer);
        if (EFI_ERROR(Status) || read_size == 0) {
            break;
        }
        crypto_sha512_update(&ctx, file_buffer, (uint32_t)read_size);
    }
    
    // Finalize hash
    crypto_sha512_final(&ctx, actual_hash);
    
    // Clean up
    file->Close(file);
    
    // Compare hashes
    if (CompareMem(actual_hash, expected_hash, 64) != 0) {
        DEBUG((DEBUG_ERROR, "Hash verification failed for %a\n", file_path));
        DEBUG((DEBUG_ERROR, "Expected: "));
        for (int i = 0; i < 64; i++) {
            DEBUG((DEBUG_ERROR, "%02x", expected_hash[i]));
        }
        DEBUG((DEBUG_ERROR, "\nActual:   "));
        for (int i = 0; i < 64; i++) {
            DEBUG((DEBUG_ERROR, "%02x", actual_hash[i]));
        }
        DEBUG((DEBUG_ERROR, "\n"));
        return EFI_SECURITY_VIOLATION;
    }
    
    return EFI_SUCCESS;
}

/**
 * @brief Verify all known file hashes
 * @return EFI_SUCCESS if all hashes match, error code otherwise
 */
static EFI_STATUS verify_boot_files(void) {
    EFI_STATUS Status;
    
    for (UINTN i = 0; i < ARRAY_SIZE(g_known_hashes); i++) {
        // Skip entries with zero hashes (not configured)
        if (IsZeroBuffer(g_known_hashes[i].expected_hash, 64)) {
            continue;
        }
        
        Status = verify_file_hash(g_known_hashes[i].path, g_known_hashes[i].expected_hash);
        if (EFI_ERROR(Status)) {
            DEBUG((DEBUG_ERROR, "Verification failed for %a: %r\n", g_known_hashes[i].path, Status));
            return Status;
        }
    }
    
    return EFI_SUCCESS;
}

/**
 * @brief Load expected hashes from configuration
 * @param config Boot configuration
 */
static void load_expected_hashes(const struct boot_config* config) {
    // Load hashes from config or trusted storage
    // This is a simplified example - in a real implementation, these would come from
    // a secure storage or be compiled into the bootloader
    
    // Example: Load kernel hash from config
    if (config->kernel[0] != '\0') {
        // In a real implementation, this would come from a secure source
        // For now, we'll just zero it out
        SetMem(g_known_hashes[0].expected_hash, 64, 0);
    }
    
    // Example: Load initrd hash from config
    if (config->initrd[0] != '\0') {
        // In a real implementation, this would come from a secure source
        // For now, we'll just zero it out
        SetMem(g_known_hashes[1].expected_hash, 64, 0);
    }
}

// Function prototypes
static EFI_STATUS InitializeTpm2(void);
static EFI_STATUS MeasureBootComponents(void);
static EFI_STATUS LoadFonts(void);
static EFI_STATUS InitializeBloodHorn(void);
static void CleanupBloodHorn(void);
static EFI_STATUS VerifySecureBootState(void);

static int load_boot_config(struct boot_config* cfg) {
    struct boot_menu_entry entries[16];
    int n = parse_ini("bloodhorn.ini", entries, 16);
    
    // Set default values
    strncpy(cfg->default_entry, "linux", sizeof(cfg->default_entry) - 1);
    cfg->menu_timeout = 10;
    cfg->tpm_enabled = TRUE;
    cfg->secure_boot = FALSE;
    cfg->use_gui = TRUE;
    strncpy(cfg->font_path, "DejaVuSans.ttf", sizeof(cfg->font_path) - 1);
    cfg->font_size = 12;
    cfg->header_font_size = 16;
    strncpy(cfg->language, "en", sizeof(cfg->language) - 1);
    cfg->enable_networking = FALSE;
    
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            if (strcmp(entries[i].section, "boot") == 0) {
                if (strcmp(entries[i].name, "default") == 0) {
                    strncpy(cfg->default_entry, entries[i].path, sizeof(cfg->default_entry) - 1);
                } else if (strcmp(entries[i].name, "menu_timeout") == 0) {
                    cfg->menu_timeout = atoi(entries[i].path);
                } else if (strcmp(entries[i].name, "use_gui") == 0) {
                    cfg->use_gui = (strcasecmp(entries[i].path, "true") == 0 || 
                                  strcmp(entries[i].path, "1") == 0);
                } else if (strcmp(entries[i].name, "language") == 0) {
                    strncpy(cfg->language, entries[i].path, sizeof(cfg->language) - 1);
                }
            } else if (strcmp(entries[i].section, "security") == 0) {
                if (strcmp(entries[i].name, "tpm_enabled") == 0) {
                    cfg->tpm_enabled = (strcasecmp(entries[i].path, "true") == 0 || 
                                       strcmp(entries[i].path, "1") == 0);
                } else if (strcmp(entries[i].name, "secure_boot") == 0) {
                    cfg->secure_boot = (strcasecmp(entries[i].path, "true") == 0 || 
                                      strcmp(entries[i].path, "1") == 0);
                }
            } else if (strcmp(entries[i].section, "display") == 0) {
                if (strcmp(entries[i].name, "font") == 0) {
                    strncpy(cfg->font_path, entries[i].path, sizeof(cfg->font_path) - 1);
                } else if (strcmp(entries[i].name, "font_size") == 0) {
                    cfg->font_size = atoi(entries[i].path);
                } else if (strcmp(entries[i].name, "header_font_size") == 0) {
                    cfg->header_font_size = atoi(entries[i].path);
                }
            } else if (strcmp(entries[i].section, "linux") == 0) {
                if (strcmp(entries[i].name, "kernel") == 0) {
                    strncpy(cfg->kernel, entries[i].path, sizeof(cfg->kernel) - 1);
                } else if (strcmp(entries[i].name, "initrd") == 0) {
                    strncpy(cfg->initrd, entries[i].path, sizeof(cfg->initrd) - 1);
                } else if (strcmp(entries[i].name, "cmdline") == 0) {
                    strncpy(cfg->cmdline, entries[i].path, sizeof(cfg->cmdline) - 1);
                }
            } else if (strcmp(entries[i].section, "networking") == 0) {
                if (strcmp(entries[i].name, "enable") == 0) {
                    cfg->enable_networking = (strcasecmp(entries[i].path, "true") == 0 || 
                                             strcmp(entries[i].path, "1") == 0);
                }
            }
        }
        return 0;
    }

    // Try to load from JSON if INI not found
    struct config_json json_entries[64];
    FILE* f = fopen("bloodhorn.json", "r");
    if (f) {
        char buf[4096];
        size_t len = fread(buf, 1, sizeof(buf) - 1, f);
        buf[len] = '\0';
        fclose(f);
        
        int m = config_json_parse(buf, json_entries, 64);
        for (int i = 0; i < m; ++i) {
            if (strcmp(json_entries[i].key, "boot.default") == 0) {
                strncpy(cfg->default_entry, json_entries[i].value, sizeof(cfg->default_entry) - 1);
            } else if (strcmp(json_entries[i].key, "boot.menu_timeout") == 0) {
                cfg->menu_timeout = atoi(json_entries[i].value);
            } else if (strcmp(json_entries[i].key, "boot.use_gui") == 0) {
                cfg->use_gui = (strcasecmp(json_entries[i].value, "true") == 0 || 
                              strcmp(json_entries[i].value, "1") == 0);
            } else if (strcmp(json_entries[i].key, "security.tpm_enabled") == 0) {
                cfg->tpm_enabled = (strcasecmp(json_entries[i].value, "true") == 0 || 
                                  strcmp(json_entries[i].value, "1") == 0);
            } else if (strcmp(json_entries[i].key, "security.secure_boot") == 0) {
                cfg->secure_boot = (strcasecmp(json_entries[i].value, "true") == 0 || 
                                  strcmp(json_entries[i].value, "1") == 0);
            } else if (strcmp(json_entries[i].key, "display.font") == 0) {
                strncpy(cfg->font_path, json_entries[i].value, sizeof(cfg->font_path) - 1);
            } else if (strcmp(json_entries[i].key, "display.font_size") == 0) {
                cfg->font_size = atoi(json_entries[i].value);
            } else if (strcmp(json_entries[i].key, "display.header_font_size") == 0) {
                cfg->header_font_size = atoi(json_entries[i].value);
            } else if (strcmp(json_entries[i].key, "display.language") == 0) {
                strncpy(cfg->language, json_entries[i].value, sizeof(cfg->language) - 1);
            } else if (strcmp(json_entries[i].key, "linux.kernel") == 0) {
                strncpy(cfg->kernel, json_entries[i].value, sizeof(cfg->kernel) - 1);
            } else if (strcmp(json_entries[i].key, "linux.initrd") == 0) {
                strncpy(cfg->initrd, json_entries[i].value, sizeof(cfg->initrd) - 1);
            } else if (strcmp(json_entries[i].key, "linux.cmdline") == 0) {
                strncpy(cfg->cmdline, json_entries[i].value, sizeof(cfg->cmdline) - 1);
            } else if (strcmp(json_entries[i].key, "networking.enable") == 0) {
                cfg->enable_networking = (strcasecmp(json_entries[i].value, "true") == 0 || 
                                         strcmp(json_entries[i].value, "1") == 0);
            }
        }
        return 0;
    }

    // Fallback to environment variables
    char val[256];
    if (config_env_get("BLOODHORN_DEFAULT", val, sizeof(val)) == 0) {
        strncpy(cfg->default_entry, val, sizeof(cfg->default_entry) - 1);
    }
    if (config_env_get("BLOODHORN_MENU_TIMEOUT", val, sizeof(val)) == 0) {
        cfg->menu_timeout = atoi(val);
    }
    if (config_env_get("BLOODHORN_USE_GUI", val, sizeof(val)) == 0) {
        cfg->use_gui = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
    }
    if (config_env_get("BLOODHORN_TPM_ENABLED", val, sizeof(val)) == 0) {
        cfg->tpm_enabled = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
    }
    if (config_env_get("BLOODHORN_SECURE_BOOT", val, sizeof(val)) == 0) {
        cfg->secure_boot = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
    }
    if (config_env_get("BLOODHORN_FONT", val, sizeof(val)) == 0) {
        strncpy(cfg->font_path, val, sizeof(cfg->font_path) - 1);
    }
    if (config_env_get("BLOODHORN_FONT_SIZE", val, sizeof(val)) == 0) {
        cfg->font_size = atoi(val);
    }
    if (config_env_get("BLOODHORN_HEADER_FONT_SIZE", val, sizeof(val)) == 0) {
        cfg->header_font_size = atoi(val);
    }
    if (config_env_get("BLOODHORN_LANGUAGE", val, sizeof(val)) == 0) {
        strncpy(cfg->language, val, sizeof(cfg->language) - 1);
    }
    if (config_env_get("BLOODHORN_LINUX_KERNEL", val, sizeof(val)) == 0) {
        strncpy(cfg->kernel, val, sizeof(cfg->kernel) - 1);
    }
    if (config_env_get("BLOODHORN_LINUX_INITRD", val, sizeof(val)) == 0) {
        strncpy(cfg->initrd, val, sizeof(cfg->initrd) - 1);
    }
    if (config_env_get("BLOODHORN_LINUX_CMDLINE", val, sizeof(val)) == 0) {
        strncpy(cfg->cmdline, val, sizeof(cfg->cmdline) - 1);
    }
    if (config_env_get("BLOODHORN_NETWORKING_ENABLE", val, sizeof(val)) == 0) {
        cfg->enable_networking = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
    }

    return 0;
}

// Forward declarations for boot wrappers
EFI_STATUS boot_bloodchain_wrapper(void);
EFI_STATUS boot_linux_kernel_wrapper(void);
EFI_STATUS boot_multiboot2_kernel_wrapper(void);
EFI_STATUS boot_limine_kernel_wrapper(void);
EFI_STATUS boot_chainload_wrapper(void);
EFI_STATUS boot_pxe_network_wrapper(void);
EFI_STATUS boot_recovery_shell_wrapper(void);
EFI_STATUS boot_uefi_shell_wrapper(EFI_HANDLE ImageHandle);  // added parameter here
EFI_STATUS exit_to_firmware_wrapper(void);
EFI_STATUS boot_ia32_wrapper(void);
EFI_STATUS boot_x86_64_wrapper(void);
EFI_STATUS boot_aarch64_wrapper(void);
EFI_STATUS boot_riscv64_wrapper(void);
EFI_STATUS boot_loongarch64_wrapper(void);

// Global network state
static BloodHorn::Net::PXEClient* gPXEClient = NULL;

// Function to initialize network
EFI_STATUS InitializeNetwork(void) {
    EFI_STATUS Status = EFI_SUCCESS;
    
    // Only initialize once
    if (gPXEClient != NULL) {
        return EFI_SUCCESS;
    }
    
    try {
        // Create and initialize network interface
        auto iface = BloodHorn::Net::NetworkInterface::create();
        gPXEClient = new BloodHorn::Net::PXEClient(std::move(iface));
        
        // Discover network configuration
        auto ec = gPXEClient->discoverNetwork();
        if (ec) {
            DEBUG((DEBUG_ERROR, "Failed to initialize network: %s\n", ec.message().c_str()));
            return EFI_DEVICE_ERROR;
        }
        
        DEBUG((DEBUG_INFO, "Network initialized successfully\n"));
        return EFI_SUCCESS;
    } catch (const std::exception& e) {
        DEBUG((DEBUG_ERROR, "Network initialization failed: %a\n", e.what()));
        return EFI_DEVICE_ERROR;
    }
}

// Function to clean up network resources
VOID ShutdownNetwork(void) {
    if (gPXEClient) {
        delete gPXEClient;
        gPXEClient = NULL;
    }
}

// Function to boot from network
EFI_STATUS BootFromNetwork(const CHAR16* kernel_path, const CHAR16* initrd_path, const CHAR8* cmdline) {
    if (!gPXEClient) {
        return EFI_NOT_READY;
    }
    
    try {
        // Convert paths to UTF-8
        CHAR8 kernel_path_utf8[256] = {0};
        UnicodeStrToAsciiStrS(kernel_path, kernel_path_utf8, ARRAY_SIZE(kernel_path_utf8));
        
        CHAR8 initrd_path_utf8[256] = {0};
        if (initrd_path) {
            UnicodeStrToAsciiStrS(initrd_path, initrd_path_utf8, ARRAY_SIZE(initrd_path_utf8));
        }
        
        // Boot the kernel
        auto ec = gPXEClient->bootKernel(
            (const char*)kernel_path_utf8,
            initrd_path ? (const char*)initrd_path_utf8 : "",
            cmdline ? (const char*)cmdline : ""
        );
        
        if (ec) {
            DEBUG((DEBUG_ERROR, "Network boot failed: %s\n", ec.message().c_str()));
            return EFI_LOAD_ERROR;
        }
        
        return EFI_SUCCESS;
    } catch (const std::exception& e) {
        DEBUG((DEBUG_ERROR, "Network boot exception: %a\n", e.what()));
        return EFI_LOAD_ERROR;
    }
}

// Helper to load theme and language from config files
static void LoadThemeAndLanguageFromConfig(void) {
    struct BootMenuTheme theme = {0};
    char lang[8] = "en";

    FILE* f = fopen("bloodhorn.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "theme_background_color")) sscanf(line, "%*[^=]=%x", &theme.background_color);
            if (strstr(line, "theme_header_color")) sscanf(line, "%*[^=]=%x", &theme.header_color);
            if (strstr(line, "theme_highlight_color")) sscanf(line, "%*[^=]=%x", &theme.highlight_color);
            if (strstr(line, "theme_text_color")) sscanf(line, "%*[^=]=%x", &theme.text_color);
            if (strstr(line, "theme_selected_text_color")) sscanf(line, "%*[^=]=%x", &theme.selected_text_color);
            if (strstr(line, "theme_footer_color")) sscanf(line, "%*[^=]=%x", &theme.footer_color);
            if (strstr(line, "theme_background_image")) {
                char imgfile[128];
                sscanf(line, "%*[^=]=%127s", imgfile);
                theme.background_image = LoadImageFile(imgfile); // Assumes LoadImageFile defined elsewhere
            }
            if (strstr(line, "language")) sscanf(line, "%*[^=]=%7s", lang);
        }
        fclose(f);
    } else {
        f = fopen("bloodhorn.json", "r");
        if (f) {
            char json[4096];
            size_t len = fread(json, 1, sizeof(json) - 1, f);
            json[len] = '\0';
            fclose(f);

            struct config_json entries[64];
            int count = config_json_parse(json, entries, 64);
            for (int i = 0; i < count; ++i) {
                if (strcmp(entries[i].key, "theme.background_color") == 0) theme.background_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.header_color") == 0) theme.header_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.highlight_color") == 0) theme.highlight_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.text_color") == 0) theme.text_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.selected_text_color") == 0) theme.selected_text_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.footer_color") == 0) theme.footer_color = (uint32_t)strtoul(entries[i].value, NULL, 16);
                if (strcmp(entries[i].key, "theme.background_image") == 0) theme.background_image = LoadImageFile(entries[i].value);
                if (strcmp(entries[i].key, "language") == 0) strncpy(lang, entries[i].value, sizeof(lang) - 1);
            }
        }
    }

    SetBootMenuTheme(&theme); // Assumes defined elsewhere
    SetLanguage(lang);        // Assumes defined elsewhere
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRT = SystemTable->RuntimeServices;

    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);

    gST->ConOut->Reset(gST->ConOut, FALSE);
    gST->ConOut->SetMode(gST->ConOut, 0);
    gST->ConOut->ClearScreen(gST->ConOut);

    // Initialize BloodHorn library with UEFI system table integration
    bh_system_table_t bloodhorn_system_table = {
        // Memory management (use UEFI services)
        .alloc = (void* (*)(bh_size_t))AllocatePool,
        .free = (void (*)(void*))FreePool,
        
        // Console output (redirect to UEFI console)
        .putc = NULL,  // Will use default implementation
        .puts = NULL,  // Will use default implementation  
        .printf = NULL, // Will use default implementation
        
        // Memory map (use UEFI memory services)
        .get_memory_map = NULL, // TODO: Implement UEFI memory map wrapper
        
        // Graphics (use UEFI Graphics Output Protocol)
        .get_graphics_info = NULL, // TODO: Implement GOP wrapper
        
        // ACPI and firmware tables
        .get_rsdp = NULL, // TODO: Implement UEFI table lookup
        .get_boot_device = NULL, // TODO: Implement device path wrapper
        
        // Power management (use UEFI runtime services)
        .reboot = NULL,   // TODO: Implement UEFI reset wrapper
        .shutdown = NULL, // TODO: Implement UEFI shutdown wrapper
        
        // Debugging
        .debug_break = NULL // Will use default implementation
    };
    
    // Initialize the BloodHorn library
    bh_status_t bh_status = bh_initialize(&bloodhorn_system_table);
    if (bh_status != BH_SUCCESS) {
        Print(L"Warning: BloodHorn library initialization failed: %a\n", bh_status_to_string(bh_status));
        // Continue anyway - the bootloader can work without the library
    } else {
        Print(L"BloodHorn library initialized successfully\n");
        
        // Print version information
        uint32_t major, minor, patch;
        bh_get_version(&major, &minor, &patch);
        Print(L"BloodHorn Library v%d.%d.%d\n", major, minor, patch);
    }

    LoadThemeAndLanguageFromConfig();
    InitMouse();

    AddBootEntry(L"BloodChain Boot Protocol", boot_bloodchain_wrapper);
    AddBootEntry(L"Linux Kernel", boot_linux_kernel_wrapper);
    AddBootEntry(L"Multiboot2 Kernel", boot_multiboot2_kernel_wrapper);
    AddBootEntry(L"Limine Kernel", boot_limine_kernel_wrapper);
    AddBootEntry(L"Chainload Bootloader", boot_chainload_wrapper);
    AddBootEntry(L"PXE Network Boot", boot_pxe_network_wrapper);
    AddBootEntry(L"IA-32 (32-bit x86)", boot_ia32_wrapper);
    AddBootEntry(L"x86-64 (64-bit x86)", boot_x86_64_wrapper);
    AddBootEntry(L"ARM64 (aarch64)", boot_aarch64_wrapper);
    AddBootEntry(L"RISC-V 64", boot_riscv64_wrapper);
    AddBootEntry(L"LoongArch 64", boot_loongarch64_wrapper);
    AddBootEntry(L"Recovery Shell", boot_recovery_shell_wrapper);
    AddBootEntry(L"UEFI Shell", (EFI_STATUS (*)(void))boot_uefi_shell_wrapper);
    AddBootEntry(L"Exit to UEFI Firmware", exit_to_firmware_wrapper);

    Status = ShowBootMenu();
    if (Status == EFI_SUCCESS) {
        VOID* KernelBuffer = NULL;
        UINTN KernelSize = 0;
        Status = LoadAndVerifyKernel(L"kernel.efi", &KernelBuffer, &KernelSize);
        if (!EFI_ERROR(Status)) {
            Status = ExecuteKernel(KernelBuffer, KernelSize, NULL);
            if (!EFI_ERROR(Status)) return EFI_SUCCESS;
        }
    }

    Print(L"\r\n  No bootable device found or kernel failed.\r\n");
    Print(L"  Press any key to reboot...");
    EFI_INPUT_KEY Key;
    UINTN Index;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_DEVICE_ERROR;
}

// Boot wrapper implementations
EFI_STATUS boot_linux_kernel_wrapper(void) {
    return linux_load_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_multiboot2_kernel_wrapper(void) {
    return multiboot2_load_kernel("/boot/vmlinuz-mb2", "root=/dev/sda1 ro");
}

EFI_STATUS boot_limine_kernel_wrapper(void) {
    return limine_load_kernel("/boot/vmlinuz-limine", "root=/dev/sda1 ro");
}

EFI_STATUS boot_chainload_wrapper(void) {
    return chainload_file("/boot/grub2.bin");
}

EFI_STATUS boot_pxe_network_wrapper(void) {
    EFI_STATUS Status;
    
    // Initialize network if not already done
    Status = InitializeNetwork();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize network\r\n");
        return Status;
    }
    
    // Boot the default kernel from network
    Status = BootFromNetwork(
        L"/boot/kernel.efi",  // Default kernel path
        L"/boot/initrd.img",  // Default initrd path
        "console=ttyS0"       // Default command line
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Network boot failed: %r\r\n", Status);
    }
    
    return Status;
}

EFI_STATUS boot_recovery_shell_wrapper(void) {
    return shell_start();
}

EFI_STATUS boot_uefi_shell_wrapper(EFI_HANDLE ImageHandle) {
    return gBS->StartImage(ImageHandle, NULL, NULL);
}

EFI_STATUS exit_to_firmware_wrapper(void) {
    gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}

EFI_STATUS boot_ia32_wrapper(void) {
    return ia32_load_kernel("/boot/vmlinuz-ia32", "/boot/initrd-ia32.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_x86_64_wrapper(void) {
    return x86_64_load_kernel("/boot/vmlinuz-x86_64", "/boot/initrd-x86_64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_aarch64_wrapper(void) {
    return aarch64_load_kernel("/boot/Image-aarch64", "/boot/initrd-aarch64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_riscv64_wrapper(void) {
    return riscv64_load_kernel("/boot/Image-riscv64", "/boot/initrd-riscv64.img", "root=/dev/sda1 ro");
}

EFI_STATUS boot_loongarch64_wrapper(void) {
    return loongarch64_load_kernel("/boot/Image-loongarch64", "/boot/initrd-loongarch64.img", "root=/dev/sda1 ro");
}

// BloodChain Boot Protocol implementation
EFI_STATUS boot_bloodchain_wrapper(void) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // 1MB mark
    EFI_PHYSICAL_ADDRESS BcbpBase;
    struct bcbp_header *hdr;
    
    // Allocate memory for BCBP header (4KB aligned)
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, 
                              EFI_SIZE_TO_PAGES(64 * 1024), &BcbpBase);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for BCBP header\n");
        return Status;
    }
    
    // Initialize BCBP header
    hdr = (struct bcbp_header *)(UINTN)BcbpBase;
    bcbp_init(hdr, KernelBase, 0);  // 0 for boot device (set by bootloader)
    
    // Load kernel
    const char *kernel_path = "kernel.elf";  // Default kernel path
    const char *initrd_path = "initrd.img";  // Default initrd path
    const char *cmdline = "root=/dev/sda1 ro"; // Default command line
    
    // Load kernel into memory
    EFI_PHYSICAL_ADDRESS KernelLoadAddr = KernelBase;
    UINTN KernelSize = 0;
    
    // Load kernel file
    Status = LoadFileToMemory(kernel_path, &KernelLoadAddr, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel: %r\n", Status);
        return Status;
    }
    
    // Add kernel module
    bcbp_add_module(hdr, KernelLoadAddr, KernelSize, "kernel", 
                   BCBP_MODTYPE_KERNEL, cmdline);
    
    // Load initrd if it exists
    EFI_PHYSICAL_ADDRESS InitrdLoadAddr = KernelLoadAddr + ALIGN_UP(KernelSize, 0x1000);
    UINTN InitrdSize = 0;
    
    if (FileExists(initrd_path)) {
        Status = LoadFileToMemory(initrd_path, &InitrdLoadAddr, &InitrdSize);
        if (!EFI_ERROR(Status) && InitrdSize > 0) {
            bcbp_add_module(hdr, InitrdLoadAddr, InitrdSize, "initrd", 
                          BCBP_MODTYPE_INITRD, NULL);
        }
    }
    
    // Set up ACPI and SMBIOS if available
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp = NULL;
    EFI_CONFIGURATION_TABLE *ConfigTable = gST->ConfigurationTable;
    
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
            Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)ConfigTable[i].VendorTable;
            break;
        }
    }
    
    if (Rsdp) {
        bcbp_set_acpi_rsdp(hdr, (UINT64)(UINTN)Rsdp);
    }
    
    // Find SMBIOS table
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiSmbiosTableGuid)) {
            bcbp_set_smbios(hdr, (UINT64)(UINTN)ConfigTable[i].VendorTable);
            break;
        }
    }
    
    // Set framebuffer information if available
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void **)&Gop);
    if (!EFI_ERROR(Status) && Gop) {
        bcbp_set_framebuffer(hdr, Gop->Mode->FrameBufferBase);
    }
    
    // Set secure boot status
    UINT8 SecureBoot = 0;
    UINTN DataSize = sizeof(SecureBoot);
    if (EFI_ERROR(gRT->GetVariable(L"SecureBoot", &gEfiGlobalVariableGuid, 
                                 NULL, &DataSize, &SecureBoot)) == EFI_SUCCESS) {
        hdr->secure_boot = SecureBoot;
    }
    
    // Set UEFI 64-bit flag
    hdr->uefi_64bit = (sizeof(UINTN) == 8) ? 1 : 0;
    
    // Validate BCBP structure
    if (bcbp_validate(hdr) != 0) {
        Print(L"Invalid BCBP structure\n");
        return EFI_LOAD_ERROR;
    }
    
    // Print boot information
    Print(L"Booting with BloodChain Boot Protocol\n");
    Print(L"  Kernel: 0x%llx (%u bytes)\n", KernelLoadAddr, KernelSize);
    if (InitrdSize > 0) {
        Print(L"  Initrd: 0x%llx (%u bytes)\n", InitrdLoadAddr, InitrdSize);
    }
    Print(L"  Command line: %a\n", cmdline);
    
    // Jump to kernel
    typedef void (*KernelEntry)(struct bcbp_header *);
    KernelEntry EntryPoint = (KernelEntry)(UINTN)KernelLoadAddr;
    
    // Disable interrupts and jump to kernel
    gBS->ExitBootServices(gImageHandle, 0);
    EntryPoint(hdr);
    
    // We should never get here
    return EFI_LOAD_ERROR;
}
