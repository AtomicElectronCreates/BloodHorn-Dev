/*
 * main.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

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
#include <Protocol/Tcg2Protocol.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Guid/FileInfo.h>

// Include BloodHorn boot modules
#include "boot/menu.h"
#include "boot/theme.h"
#include "boot/localization.h"
#include "boot/font.h"
#include "boot/mouse.h"
#include "boot/secure.h"
#include "fs/fat32.h"
#include "security/crypto.h"
#include "security/tpm2.h"
#include "scripting/lua.h"
#include "recovery/shell.h"
#include "plugins/plugin.h"
#include "net/pxe.h"
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
#include "security/sha512.h"

// Include Coreboot platform integration
#include "coreboot/coreboot_platform.h"

// File hash verification structure (typedef before usage)
typedef struct {
    char path[256];
    uint8_t expected_hash[64];
} FILE_HASH;

// Global TPM 2.0 context
static EFI_TCG2_PROTOCOL* gTcg2Protocol = NULL;

// Global BloodHorn context
static bh_context_t* gBhContext = NULL;

// Global image handle for ExitBootServices and child image loading
static EFI_HANDLE gImageHandle = NULL;

// Global flag to indicate if Coreboot is available
STATIC BOOLEAN gCorebootAvailable = FALSE;

// Global font handles
static bh_font_t gDefaultFont = {0};
static bh_font_t gHeaderFont = {0};

// Global known hashes for kernel verification (if security is enabled)
STATIC FILE_HASH g_known_hashes[1] = {0};

// Forward declaration for Coreboot main entry point
extern VOID EFIAPI CorebootMain(VOID* coreboot_table, VOID* payload);

// Global flag to indicate if we're running as Coreboot payload
STATIC BOOLEAN gRunningAsCorebootPayload = FALSE;

// Boot configuration structure
typedef struct {
    char default_entry[64];
    int menu_timeout;
    char kernel[128];
    char initrd[128];
    char cmdline[256];
    bool tpm_enabled;
    bool secure_boot;
    bool use_gui;
    char font_path[256];
    uint32_t font_size;
    uint32_t header_font_size;
    char language[8];
    bool enable_networking;
} BOOT_CONFIG;

// Coreboot boot parameter structure definitions
#define COREBOOT_BOOT_SIGNATURE 0x12345678
#define COREBOOT_BOOT_FLAG_KERNEL 0x01
#define COREBOOT_BOOT_FLAG_FRAMEBUFFER 0x02
#define COREBOOT_BOOT_FLAG_INITRD 0x04

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

    // Initrd information
    UINT64 initrd_addr;         // Initrd address
    UINT64 initrd_size;         // Initrd size

    // Command line (simplified)
    CHAR8 cmdline[256];         // Kernel command line
} COREBOOT_BOOT_PARAMS;

// Forward declarations for boot wrappers
EFI_STATUS EFIAPI BootBloodchainWrapper(VOID);
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID);
EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID);
EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID);
EFI_STATUS EFIAPI BootChainloadWrapper(VOID);
EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID);
EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID);
EFI_STATUS EFIAPI BootUefiShellWrapper(VOID);
EFI_STATUS EFIAPI ExitToFirmwareWrapper(VOID);
EFI_STATUS EFIAPI BootIa32Wrapper(VOID);
EFI_STATUS EFIAPI BootX86_64Wrapper(VOID);
EFI_STATUS EFIAPI BootAarch64Wrapper(VOID);
EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID);
EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID);

/**
 * Minimal helpers for configuration loading (INI -> JSON -> UEFI variables)
 */
STATIC BOOLEAN str_ieq(const CHAR8* a, const CHAR8* b) {
    while (*a && *b) {
        CHAR8 ca = (*a >= 'A' && *a <= 'Z') ? (*a - 'A' + 'a') : *a;
        CHAR8 cb = (*b >= 'A' && *b <= 'Z') ? (*b - 'A' + 'a') : *b;
        if (ca != cb) return FALSE;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

// Load a file from the filesystem into memory (no hash verification)
STATIC
EFI_STATUS
LoadFileRaw (
  IN CHAR16* Path,
  OUT VOID** Buffer,
  OUT UINTN* Size
  )
{
    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    EFI_FILE_HANDLE file;
    VOID* buffer = NULL;
    UINTN size = 0;

    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = root_dir->Open(root_dir, &file, Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    EFI_FILE_INFO* info = NULL;
    UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) { file->Close(file); return EFI_OUT_OF_RESOURCES; }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }
    if (EFI_ERROR(Status)) { if (info) FreePool(info); file->Close(file); return Status; }

    size = (UINTN)info->FileSize;
    buffer = AllocateZeroPool(size);
    if (!buffer) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }

    Status = file->Read(file, &size, buffer);
    file->Close(file);
    FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buffer); return Status; }

    *Buffer = buffer;
    *Size = size;
    return EFI_SUCCESS;
}

STATIC BOOLEAN parse_bool_ascii(const CHAR8* v, BOOLEAN defv) {
    if (!v) return defv;
    if (str_ieq(v, "true") || str_ieq(v, "1") || str_ieq(v, "yes")) return TRUE;
    if (str_ieq(v, "false") || str_ieq(v, "0") || str_ieq(v, "no")) return FALSE;
    return defv;
}

STATIC VOID trim_ascii(CHAR8* s) {
    if (!s) return;
    // trim leading
    CHAR8* p = s; while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) CopyMem(s, p, AsciiStrLen(p)+1);
    // trim trailing
    UINTN n = AsciiStrLen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t' || s[n-1] == '\r' || s[n-1] == '\n')) { s[n-1] = 0; n--; }
}

STATIC EFI_STATUS ReadWholeFileAscii(EFI_FILE_HANDLE root, CONST CHAR16* path, CHAR8** outBuf, UINTN* outLen) {
    EFI_STATUS Status;
    EFI_FILE_HANDLE file = NULL;
    *outBuf = NULL; *outLen = 0;
    Status = root->Open(root, &file, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;
    EFI_FILE_INFO* info = NULL; UINTN info_size = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_size);
        if (!info) { file->Close(file); return EFI_OUT_OF_RESOURCES; }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    }
    if (EFI_ERROR(Status)) { if (info) FreePool(info); file->Close(file); return Status; }
    UINTN size = (UINTN)info->FileSize;
    CHAR8* buf = AllocateZeroPool(size + 1);
    if (!buf) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }
    Status = file->Read(file, &size, buf);
    file->Close(file);
    FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buf); return Status; }
    buf[size] = 0;
    *outBuf = buf; *outLen = size;
    return EFI_SUCCESS;
}

STATIC VOID ApplyIniToConfig(const CHAR8* ini, BOOT_CONFIG* config) {
    CHAR8 section[64] = {0};
    const CHAR8* cur = ini;
    while (*cur) {
        // read line
        const CHAR8* lineStart = cur;
        while (*cur && *cur != '\n') cur++;
        UINTN len = (UINTN)(cur - lineStart);
        CHAR8* line = AllocateZeroPool(len + 1);
        if (!line) return;
        CopyMem(line, lineStart, len); line[len] = 0;
        if (*cur == '\n') cur++;

        trim_ascii(line);
        if (line[0] == '#' || line[0] == ';' || line[0] == 0) { FreePool(line); continue; }
        if (line[0] == '[') {
            CHAR8* rb = AsciiStrStr(line, "]");
            if (rb) {
                *rb = 0;
                AsciiStrCpyS(section, sizeof(section), line + 1);
            }
            FreePool(line); continue;
        }
        CHAR8* eq = AsciiStrStr(line, "=");
        if (!eq) { FreePool(line); continue; }
        *eq = 0; CHAR8* k = line; CHAR8* v = eq + 1; trim_ascii(k); trim_ascii(v);

        if (str_ieq(section, "boot")) {
            if (str_ieq(k, "default")) {
                AsciiStrCpyS(config->default_entry, sizeof(config->default_entry), v);
            } else if (str_ieq(k, "menu_timeout")) {
                config->menu_timeout = (int)AsciiStrDecimalToUintn(v);
            } else if (str_ieq(k, "language")) {
                AsciiStrCpyS(config->language, sizeof(config->language), v);
            } else if (str_ieq(k, "secure_boot")) {
                config->secure_boot = parse_bool_ascii(v, config->secure_boot);
            } else if (str_ieq(k, "tpm_enabled")) {
                config->tpm_enabled = parse_bool_ascii(v, config->tpm_enabled);
            } else if (str_ieq(k, "use_gui")) {
                config->use_gui = parse_bool_ascii(v, config->use_gui);
            }
        } else if (str_ieq(section, "linux")) {
            if (str_ieq(k, "kernel")) {
                AsciiStrCpyS(config->kernel, sizeof(config->kernel), v);
            } else if (str_ieq(k, "initrd")) {
                AsciiStrCpyS(config->initrd, sizeof(config->initrd), v);
            } else if (str_ieq(k, "cmdline")) {
                AsciiStrCpyS(config->cmdline, sizeof(config->cmdline), v);
            }
        }
        FreePool(line);
    }
}

STATIC VOID ApplyJsonToConfig(const CHAR8* js, BOOT_CONFIG* config) {
    // Minimal substring extraction; this is not a full JSON parser but adequate for expected fields.
    #define FIND_STR(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t' || *p=='\n') p++; if (*p=='\"') { p++; const CHAR8* q=p; while (*q && *q!='\"') q++; UINTN L=(UINTN)(q-p); CHAR8 tmp[512]={0}; if (L>=sizeof(tmp)) L=sizeof(tmp)-1; CopyMem(tmp,p,L); tmp[L]=0; AsciiStrCpyS(dst, sizeof(dst), tmp); } } } } while(0)
    #define FIND_INT(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t'||*p=='\n') p++; INTN val=0; while (*p>='0'&&*p<='9'){ val = val*10 + (*p-'0'); p++; } dst = (int)val; } } } while(0)
    #define FIND_BOOL(key, dst) do { const CHAR8* p = AsciiStrStr((CHAR8*)js, key); if (p) { p = AsciiStrStr((CHAR8*)p, ":"); if (p) { p++; while (*p==' '||*p=='\t'||*p=='\n') p++; if (AsciiStrnCmp(p, "true", 4)==0) dst=TRUE; else if (AsciiStrnCmp(p,"false",5)==0) dst=FALSE; } } } while(0)

    FIND_STR("\"default\"", config->default_entry);
    FIND_INT("\"menu_timeout\"", config->menu_timeout);
    FIND_STR("\"language\"", config->language);

    // entries.linux
    FIND_STR("\"kernel\"", config->kernel);
    FIND_STR("\"initrd\"", config->initrd);
    FIND_STR("\"cmdline\"", config->cmdline);
}

STATIC VOID ApplyUefiEnvOverrides(BOOT_CONFIG* config) {
    struct { CONST CHAR16* name; enum { T_STR, T_INT, T_BOOL } typ; VOID* target; UINTN tsize; } vars[] = {
        { L"BLOODHORN_DEFAULT", T_STR,  config->default_entry, sizeof(config->default_entry) },
        { L"BLOODHORN_MENU_TIMEOUT", T_INT, &config->menu_timeout, sizeof(config->menu_timeout) },
        { L"BLOODHORN_LANGUAGE", T_STR,  config->language, sizeof(config->language) },
        { L"BLOODHORN_LINUX_KERNEL", T_STR,  config->kernel, sizeof(config->kernel) },
        { L"BLOODHORN_LINUX_INITRD", T_STR,  config->initrd, sizeof(config->initrd) },
        { L"BLOODHORN_LINUX_CMDLINE", T_STR,  config->cmdline, sizeof(config->cmdline) },
        { L"BLOODHORN_SECURE_BOOT", T_BOOL, &config->secure_boot, sizeof(config->secure_boot) },
        { L"BLOODHORN_TPM_ENABLED", T_BOOL, &config->tpm_enabled, sizeof(config->tpm_enabled) },
    };

    for (UINTN i = 0; i < ARRAY_SIZE(vars); ++i) {
        UINTN sz = 0; EFI_STATUS st;
        st = gRT->GetVariable((CHAR16*)vars[i].name, &gEfiGlobalVariableGuid, NULL, &sz, NULL);
        if (st != EFI_BUFFER_TOO_SMALL) continue;
        VOID* buf = AllocateZeroPool(sz);
        if (!buf) continue;
        st = gRT->GetVariable((CHAR16*)vars[i].name, &gEfiGlobalVariableGuid, NULL, &sz, buf);
        if (EFI_ERROR(st)) { FreePool(buf); continue; }
        if (vars[i].typ == T_STR) {
            // treat as UCS-2 string -> convert to ASCII
            CHAR16* w = (CHAR16*)buf; CHAR8 a[256] = {0};
            UnicodeStrToAsciiStrS(w, a, sizeof(a));
            AsciiStrCpyS((CHAR8*)vars[i].target, vars[i].tsize, a);
        } else if (vars[i].typ == T_INT) {
            if (sz >= sizeof(UINT32)) *(int*)vars[i].target = (int)(*(UINT32*)buf);
        } else if (vars[i].typ == T_BOOL) {
            if (sz >= sizeof(UINT8)) *(BOOLEAN*)vars[i].target = (*(UINT8*)buf) ? TRUE : FALSE;
        }
        FreePool(buf);
    }
}

/**
 * Load boot configuration from files
 */
STATIC
EFI_STATUS
LoadBootConfig (
  OUT BOOT_CONFIG* config
  )
{
    // Defaults
    AsciiStrCpyS(config->default_entry, sizeof(config->default_entry), "linux");
    config->menu_timeout = 10;
    config->tpm_enabled = TRUE;
    config->secure_boot = FALSE;
    config->use_gui = TRUE;
    AsciiStrCpyS(config->font_path, sizeof(config->font_path), "DejaVuSans.ttf");
    config->font_size = 12;
    config->header_font_size = 16;
    AsciiStrCpyS(config->language, sizeof(config->language), "en");
    config->enable_networking = FALSE;
    config->kernel[0] = 0; config->initrd[0] = 0; config->cmdline[0] = 0;

    EFI_STATUS Status;
    EFI_FILE_HANDLE root_dir;
    Status = get_root_dir(&root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open filesystem for config: %r\n", Status);
        return EFI_SUCCESS; // keep defaults, do not fail boot
    }

    // 1) INI: bloodhorn.ini
    CHAR8* buf = NULL; UINTN blen = 0;
    Status = ReadWholeFileAscii(root_dir, L"bloodhorn.ini", &buf, &blen);
    if (!EFI_ERROR(Status) && buf && blen > 0) {
        ApplyIniToConfig(buf, config);
        FreePool(buf); buf = NULL; blen = 0;
    }

    // 2) JSON: bloodhorn.json
    Status = ReadWholeFileAscii(root_dir, L"bloodhorn.json", &buf, &blen);
    if (!EFI_ERROR(Status) && buf && blen > 0) {
        ApplyJsonToConfig(buf, config);
        FreePool(buf); buf = NULL; blen = 0;
    }

    // 3) Environment variables (UEFI vars)
    ApplyUefiEnvOverrides(config);

    // Clamp values according to docs
    if (config->menu_timeout < 0) config->menu_timeout = 0;
    if (config->menu_timeout > 300) config->menu_timeout = 300;

    return EFI_SUCCESS;
}

/**
 * Initialize BloodHorn bootloader under Coreboot + UEFI hybrid mode
 */
STATIC
EFI_STATUS
InitializeBloodHorn (
  VOID
  )
{
    EFI_STATUS Status;

    // Check for Coreboot firmware and initialize if present
    gCorebootAvailable = CorebootPlatformInit();

    if (gCorebootAvailable) {
        Print(L"BloodHorn Bootloader (Coreboot + UEFI Hybrid Mode)\n");
        Print(L"Coreboot firmware detected - using hybrid initialization\n");

        // Use Coreboot for hardware initialization when available
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

        // Use UEFI for higher-level services
        Print(L"Using UEFI services for boot menu and file operations\n");
    } else {
        Print(L"BloodHorn Bootloader (UEFI Mode)\n");
        Print(L"Coreboot firmware not detected - using UEFI initialization\n");
    }

    return EFI_SUCCESS;
}

/**
 * Main entry point for BloodHorn bootloader under Coreboot + UEFI hybrid
 */
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;

    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRT = SystemTable->RuntimeServices;
    gImageHandle = ImageHandle;

    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);

    gST->ConOut->Reset(gST->ConOut, FALSE);
    gST->ConOut->SetMode(gST->ConOut, 0);
    gST->ConOut->ClearScreen(gST->ConOut);

    // Initialize BloodHorn in hybrid mode
    Status = InitializeBloodHorn();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize BloodHorn: %r\n", Status);
        return Status;
    }

    // Initialize BloodHorn library with UEFI system table integration
    bh_system_table_t bloodhorn_system_table = {
        // Memory management (use UEFI services)
        .alloc = (void* (*)(bh_size_t))AllocatePool,
        .free = (void (*)(void*))FreePool,

        // Console output (redirect to UEFI console)
        .putc = bh_uefi_putc,
        .puts = bh_uefi_puts,
        .printf = (void (*)(const char*, ...))Print,

        // Memory map (use UEFI memory services or Coreboot if available)
        .get_memory_map = (bh_status_t (*)(bh_memory_map_t*))gBS->GetMemoryMap,

        // Graphics (use UEFI Graphics Output Protocol or Coreboot framebuffer)
        .get_graphics_info = NULL, // Graphics are handled outside libb

        // ACPI and firmware tables (use UEFI or Coreboot tables)
        .get_rsdp = (void* (*)(void))GetRsdp,
        .get_boot_device = NULL, // Platform specific

        // Power management (use UEFI runtime services)
        .reboot = bh_uefi_reboot,
        .shutdown = bh_uefi_shutdown,

        // Debugging
        .debug_break = bh_uefi_debug_break
    };

    // Initialize the BloodHorn library
    bh_status_t bh_status = bh_initialize(&bloodhorn_system_table);
    if (bh_status != BH_SUCCESS) {
        Print(L"Warning: BloodHorn library initialization failed: %a\n", bh_status_to_string(bh_status));
    } else {
        Print(L"BloodHorn library initialized successfully\n");

        uint32_t major, minor, patch;
        bh_get_version(&major, &minor, &patch);
        Print(L"BloodHorn Library v%d.%d.%d\n", major, minor, patch);
    }

    // Load configuration (INI -> JSON -> UEFI vars)
    BOOT_CONFIG config;
    LoadBootConfig(&config);

    // Apply language and font from configuration before any UI
    SetLanguage(config.language);
    if (config.font_path[0] != '\0') {
        Font* user_font = LoadFontFile(config.font_path);
        if (user_font) {
            SetDefaultFont(user_font);
        }
    }

    // Theme loading (language already applied from config)
    LoadThemeAndLanguageFromConfig();
    InitMouse();

    // Pre-menu autoboot based on configuration
    BOOLEAN showMenu = TRUE;
    if (config.menu_timeout > 0) {
        // Countdown; any key press cancels and shows menu
        INTN secs = config.menu_timeout;
        while (secs > 0) {
            Print(L"Auto-boot '%a' in %d seconds... Press any key to open menu.\r\n", config.default_entry, secs);
            // Wait up to 1 second, polling key every 100ms
            BOOLEAN keyPressed = FALSE;
            for (int i = 0; i < 10; i++) {
                EFI_INPUT_KEY Key;
                if (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &Key))) { keyPressed = TRUE; break; }
                gBS->Stall(100000); // 100ms
            }
            if (keyPressed) { showMenu = TRUE; break; }
            secs--;
            if (secs == 0) {
                showMenu = FALSE;
            }
        }
    }

    // If no key pressed within timeout, autoboot supported default
    if (!showMenu) {
        if (AsciiStrCmp(config.default_entry, "linux") == 0 && config.kernel[0] != 0) {
            const char* k = config.kernel;
            const char* i = (config.initrd[0] != 0) ? config.initrd : NULL;
            const char* c = (config.cmdline[0] != 0) ? config.cmdline : "";
            EFI_STATUS ls = linux_load_kernel(k, i, c);
            if (!EFI_ERROR(ls)) {
                return EFI_SUCCESS;
            } else {
                Print(L"Autoboot linux failed: %r\n", ls);
                showMenu = TRUE;
            }
        } else {
            // Unsupported default entry fallback to menu
            showMenu = TRUE;
        }
    }

    AddBootEntry(L"BloodChain Boot Protocol", BootBloodchainWrapper);
    AddBootEntry(L"Linux Kernel", BootLinuxKernelWrapper);
    AddBootEntry(L"Multiboot2 Kernel", BootMultiboot2KernelWrapper);
    AddBootEntry(L"Limine Kernel", BootLimineKernelWrapper);
    AddBootEntry(L"Chainload Bootloader", BootChainloadWrapper);
    AddBootEntry(L"PXE Network Boot", BootPxeNetworkWrapper);
    AddBootEntry(L"IA-32 (32-bit x86)", BootIa32Wrapper);
    AddBootEntry(L"x86-64 (64-bit x86)", BootX86_64Wrapper);
    AddBootEntry(L"ARM64 (aarch64)", BootAarch64Wrapper);
    AddBootEntry(L"RISC-V 64", BootRiscv64Wrapper);
    AddBootEntry(L"LoongArch 64", BootLoongarch64Wrapper);
    AddBootEntry(L"Recovery Shell", BootRecoveryShellWrapper);
    AddBootEntry(L"UEFI Shell", (EFI_STATUS (*)(void))BootUefiShellWrapper);
    AddBootEntry(L"Exit to UEFI Firmware", ExitToFirmwareWrapper);

    Status = showMenu ? ShowBootMenu() : EFI_NOT_READY;
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

    // Use Coreboot for reboot if available, otherwise use UEFI
    if (gCorebootAvailable) {
        CorebootReboot();
    } else {
        gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    return EFI_DEVICE_ERROR;
}

// Helper function to convert UEFI putc to bh_putc
static void bh_uefi_putc(char c) {
    CHAR16 C[2] = {c, 0};
    gST->ConOut->OutputString(gST->ConOut, C);
}

// Helper function to convert UEFI puts to bh_puts
static void bh_uefi_puts(const char* s) {
    while (*s) {
        bh_uefi_putc(*s++);
    }
}

// Helper function for UEFI reboot
static void bh_uefi_reboot(void) {
    gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
}

// Helper function for UEFI shutdown
static void bh_uefi_shutdown(void) {
    gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}

// Helper function for debug break
static void bh_uefi_debug_break(void) {
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("int $3");
#else
    /* On non-x86 platforms, there's no INT3; use a portable hang to aid debugging */
    for (;;) { __asm__ volatile ("nop"); }
#endif
}

// Boot wrapper implementations
EFI_STATUS EFIAPI BootLinuxKernelWrapper(VOID) {
    return linux_load_kernel("/boot/vmlinuz", "/boot/initrd.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootMultiboot2KernelWrapper(VOID) {
    return multiboot2_load_kernel("/boot/vmlinuz-mb2", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootLimineKernelWrapper(VOID) {
    return limine_load_kernel("/boot/vmlinuz-limine", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootChainloadWrapper(VOID) {
    return chainload_file("/boot/grub2.bin");
}

EFI_STATUS EFIAPI BootPxeNetworkWrapper(VOID) {
    EFI_STATUS Status;

    Status = InitializeNetwork();
    if (EFI_ERROR(Status)) {
        Print(L"Failed to initialize network\n");
        return Status;
    }

    Status = BootFromNetwork(
        L"/boot/kernel.efi",
        L"/boot/initrd.img",
        "console=ttyS0"
    );

    if (EFI_ERROR(Status)) {
        Print(L"Network boot failed: %r\n", Status);
    }

    return Status;
}

EFI_STATUS EFIAPI BootRecoveryShellWrapper(VOID) {
    return shell_start();
}

EFI_STATUS EFIAPI BootUefiShellWrapper(VOID) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;

    CONST CHAR16* Candidates[] = {
        L"\\EFI\\BOOT\\Shell.efi",
        L"\\EFI\\tools\\Shell.efi",
        L"\\Shell.efi"
    };

    for (UINTN i = 0; i < ARRAY_SIZE(Candidates); ++i) {
        Status = LoadAndStartImageFromPath(gImageHandle, LoadedImage->DeviceHandle, Candidates[i]);
        if (!EFI_ERROR(Status)) return EFI_SUCCESS;
    }

    Print(L"UEFI Shell not found on this system.\n");
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI ExitToFirmwareWrapper(VOID) {
    if (gCorebootAvailable) {
        CorebootReboot();
    } else {
        gST->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
    return EFI_SUCCESS;
}

// BloodChain Boot Protocol implementation
EFI_STATUS EFIAPI BootBloodchainWrapper(VOID) {
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // 1MB mark
    EFI_PHYSICAL_ADDRESS BcbpBase;

    // Allocate memory for BCBP header (4KB aligned)
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData,
                              EFI_SIZE_TO_PAGES(64 * 1024), &BcbpBase);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for BCBP header\n");
        return Status;
    }

    // Initialize BCBP header
    struct bcbp_header* hdr = (struct bcbp_header*)(UINTN)BcbpBase;
    bcbp_init(hdr, KernelBase, 0);  // 0 for boot device (set by bootloader)

    // Load kernel
    const char* kernel_path = "kernel.elf";
    const char* initrd_path = "initrd.img";
    const char* cmdline = "root=/dev/sda1 ro";

    // Load kernel into memory
    EFI_PHYSICAL_ADDRESS KernelLoadAddr = KernelBase;
    UINTN KernelSize = 0;

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
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Rsdp = NULL;
    EFI_CONFIGURATION_TABLE* ConfigTable = gST->ConfigurationTable;

    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&ConfigTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
            Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)ConfigTable[i].VendorTable;
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
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Gop);
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
    typedef void (*KernelEntry)(struct bcbp_header*);
    KernelEntry EntryPoint = (KernelEntry)(UINTN)KernelLoadAddr;

    // Properly exit boot services (robustly handle map changes/races)
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    EFI_STATUS EStatus = EFI_SUCCESS;

    const int max_retries = 8;
    int attempt = 0;
    while (attempt++ < max_retries) {
        MapSize = 0;
        EStatus = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        if (EStatus == EFI_BUFFER_TOO_SMALL) {
            if (MemMap) { FreePool(MemMap); MemMap = NULL; }
            MemMap = AllocatePool(MapSize);
            if (!MemMap) { EStatus = EFI_OUT_OF_RESOURCES; break; }
            EStatus = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        }

        if (EFI_ERROR(EStatus)) {
            /* Unexpected error; retry a couple times */
            CpuPause();
            continue;
        }

        /* Try to exit boot services; if it fails with EFI_INVALID_PARAMETER the map changed -> retry */
        EStatus = gBS->ExitBootServices(gImageHandle, MapKey);
        if (EStatus == EFI_INVALID_PARAMETER) {
            /* Map changed between GetMemoryMap and ExitBootServices; retry */
            CpuPause();
            continue;
        }

        /* On success or other error, break and handle below */
        break;
    }

    if (MemMap) { FreePool(MemMap); MemMap = NULL; }
    if (EFI_ERROR(EStatus)) {
        Print(L"Failed to exit boot services (status=%r)\n", EStatus);
        return EFI_LOAD_ERROR;
    }

    EntryPoint(hdr);

    return EFI_LOAD_ERROR;
}

// Architecture-specific boot wrappers
EFI_STATUS EFIAPI BootIa32Wrapper(VOID) {
    return ia32_load_kernel("/boot/vmlinuz-ia32", "/boot/initrd-ia32.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootX86_64Wrapper(VOID) {
    return x86_64_load_kernel("/boot/vmlinuz-x86_64", "/boot/initrd-x86_64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootAarch64Wrapper(VOID) {
    return aarch64_load_kernel("/boot/Image-aarch64", "/boot/initrd-aarch64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootRiscv64Wrapper(VOID) {
    return riscv64_load_kernel("/boot/Image-riscv64", "/boot/initrd-riscv64.img", "root=/dev/sda1 ro");
}

EFI_STATUS EFIAPI BootLoongarch64Wrapper(VOID) {
    return loongarch64_load_kernel("/boot/Image-loongarch64", "/boot/initrd-loongarch64.img", "root=/dev/sda1 ro");
}

// Network and utility functions
EFI_STATUS InitializeNetwork(VOID) {
    int rc = pxe_network_init();
    return (rc == 0) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

VOID ShutdownNetwork(VOID) {
    pxe_cleanup_network();
}

EFI_STATUS BootFromNetwork(CONST CHAR16* kernel_path, CONST CHAR16* initrd_path, CONST CHAR8* cmdline) {
    CHAR8 kernel_ascii[256] = {0};
    CHAR8 initrd_ascii[256] = {0};
    if (kernel_path) UnicodeStrToAsciiStrS(kernel_path, kernel_ascii, sizeof(kernel_ascii));
    if (initrd_path) UnicodeStrToAsciiStrS(initrd_path, initrd_ascii, sizeof(initrd_ascii));

    const char* initrd = initrd_path ? (const char*)initrd_ascii : NULL;
    const char* cmd = cmdline ? (const char*)cmdline : "";

    int rc = pxe_boot_kernel((const char*)kernel_ascii, initrd, cmd);
    return (rc == 0) ? EFI_SUCCESS : EFI_LOAD_ERROR;
}

STATIC EFI_STATUS LoadAndStartImageFromPath(EFI_HANDLE ParentImage, EFI_HANDLE DeviceHandle, CONST CHAR16* Path) {
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    EFI_FILE_PROTOCOL* Root = NULL;
    EFI_FILE_PROTOCOL* File = NULL;
    EFI_DEVICE_PATH_PROTOCOL* FilePath = NULL;
    VOID* Buffer = NULL;
    UINTN Size = 0;

    Status = gBS->HandleProtocol(DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) return Status;
    Status = Fs->OpenVolume(Fs, &Root);
    if (EFI_ERROR(Status)) return Status;

    Status = Root->Open(Root, &File, (CHAR16*)Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) { Root->Close(Root); return Status; }

    EFI_FILE_INFO* Info = NULL;
    UINTN InfoSize = 0;
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Info = AllocateZeroPool(InfoSize);
        if (!Info) { File->Close(File); Root->Close(Root); return EFI_OUT_OF_RESOURCES; }
        Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
    }
    if (EFI_ERROR(Status)) { if (Info) FreePool(Info); File->Close(File); Root->Close(Root); return Status; }
    Size = (UINTN)Info->FileSize;
    Buffer = AllocateZeroPool(Size);
    if (!Buffer) { FreePool(Info); File->Close(File); Root->Close(Root); return EFI_OUT_OF_RESOURCES; }
    Status = File->Read(File, &Size, Buffer);
    File->Close(File);
    Root->Close(Root);
    FreePool(Info);
    if (EFI_ERROR(Status)) { FreePool(Buffer); return Status; }

    FilePath = FileDevicePath(DeviceHandle, Path);
    if (!FilePath) { FreePool(Buffer); return EFI_OUT_OF_RESOURCES; }
    EFI_HANDLE Child = NULL;
    Status = gBS->LoadImage(FALSE, ParentImage, FilePath, Buffer, Size, &Child);
    FreePool(FilePath);
    FreePool(Buffer);
    if (EFI_ERROR(Status)) return Status;
    return gBS->StartImage(Child, NULL, NULL);
}

/**
 * Load and verify kernel from filesystem
 */
STATIC
EFI_STATUS
LoadAndVerifyKernel (
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

    // Get root directory
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
 * Execute loaded kernel in hybrid EDK2 + Coreboot environment
 */
STATIC
EFI_STATUS
ExecuteKernel (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    Print(L"Executing kernel at 0x%llx (%u bytes)\n",
          (UINT64)(UINTN)KernelBuffer, KernelSize);

    // Set up execution environment based on available firmware
    if (gCorebootAvailable) {
        // Use Coreboot memory management for kernel execution
        Status = ExecuteKernelWithCoreboot(KernelBuffer, KernelSize, InitrdBuffer);
    } else {
        // Use UEFI services for kernel execution
        Status = ExecuteKernelWithUefi(KernelBuffer, KernelSize, InitrdBuffer);
    }

    return Status;
}

/**
 * Execute kernel using Coreboot services
 */
STATIC
EFI_STATUS
ExecuteKernelWithCoreboot (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    // Get Coreboot memory map for proper kernel setup
    UINT32 mem_map_count = 0;
    CONST COREBOOT_MEM_ENTRY* mem_map = CorebootGetMemoryMap(&mem_map_count);

    if (!mem_map || mem_map_count == 0) {
        Print(L"Failed to get Coreboot memory map for kernel execution\n");
        return EFI_DEVICE_ERROR;
    }

    // Find suitable memory region for kernel execution
    UINT64 kernel_base = 0;
    UINT64 largest_size = 0;

    for (UINT32 i = 0; i < mem_map_count; i++) {
        if (mem_map[i].type == CB_MEM_RAM && mem_map[i].size > largest_size) {
            kernel_base = mem_map[i].addr;
            largest_size = mem_map[i].size;
        }
    }

    if (kernel_base == 0) {
        Print(L"No suitable RAM region found for kernel execution\n");
        return EFI_DEVICE_ERROR;
    }

    // Set up kernel execution environment
    Print(L"Setting up kernel execution environment...\n");
    Print(L"Kernel base: 0x%llx, Size: %u bytes\n", kernel_base, KernelSize);

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
    // Set up proper boot parameter structure that kernel can access
    UINT64 boot_params_addr = kernel_base + 0x1000; // 4KB after kernel

    // Set up Coreboot boot parameters structure
    COREBOOT_BOOT_PARAMS* boot_params = (COREBOOT_BOOT_PARAMS*)boot_params_addr;

    // Zero out the boot parameters structure
    SetMem(boot_params, sizeof(COREBOOT_BOOT_PARAMS), 0);

    // Set up basic boot parameters
    boot_params->signature = COREBOOT_BOOT_SIGNATURE;
    boot_params->version = 1;
    boot_params->kernel_base = kernel_base;
    boot_params->kernel_size = KernelSize;
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

    // Set up initrd if provided
    if (InitrdBuffer) {
        boot_params->initrd_addr = (UINT64)(UINTN)InitrdBuffer;
        boot_params->boot_flags |= COREBOOT_BOOT_FLAG_INITRD;
    }

    /* Ensure kernel is placed at chosen Coreboot region. The Coreboot memory map
       provides physical addresses; copy the kernel buffer into the selected
       kernel_base region so boot params and entry point are consistent. */
    if (KernelSize > largest_size) {
        Print(L"Kernel size %u exceeds selected Coreboot region of %u bytes\n", KernelSize, largest_size);
        return EFI_BAD_BUFFER_SIZE;
    }

    VOID* kernel_target = (VOID*)(UINTN)kernel_base;
    CopyMem(kernel_target, KernelBuffer, (UINTN)KernelSize);
    /* Use the kernel target as the canonical kernel buffer/entry point from now on */
    KernelBuffer = kernel_target;

    // Validate boot parameters structure
    if (!ValidateBootParameters(boot_params)) {
        Print(L"Boot parameters validation failed\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"Boot parameters set up at 0x%llx\n", boot_params_addr);
    Print(L"Framebuffer: %ux%u @ 0x%llx\n",
          boot_params->framebuffer_width,
          boot_params->framebuffer_height,
          boot_params->framebuffer_addr);

    // Exit boot services and jump to kernel
    Status = ExitBootServicesAndExecuteKernel(KernelBuffer, KernelSize, boot_params_addr);

    return Status;
}

/**
 * Execute kernel using UEFI services
 */
STATIC
EFI_STATUS
ExecuteKernelWithUefi (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN VOID* InitrdBuffer OPTIONAL
  )
{
    EFI_STATUS Status;

    // Get UEFI memory map for kernel setup
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;

    Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        MemMap = AllocatePool(MapSize);
        if (!MemMap) {
            return EFI_OUT_OF_RESOURCES;
        }
        Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
    }

    if (EFI_ERROR(Status)) {
        Print(L"Failed to get UEFI memory map\n");
        return Status;
    }

    // Find suitable memory region for kernel
    EFI_PHYSICAL_ADDRESS KernelBase = 0x100000; // Default 1MB

    // Look for conventional memory above 1MB
    for (UINTN i = 0; i < MapSize / DescSize; i++) {
        EFI_MEMORY_DESCRIPTOR* Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + i * DescSize);
        if (Desc->Type == EfiConventionalMemory &&
            Desc->PhysicalStart >= 0x100000 &&
            Desc->NumberOfPages * EFI_PAGE_SIZE >= KernelSize) {
            KernelBase = Desc->PhysicalStart;
            break;
        }
    }

    // Configure GOP framebuffer if available
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if (!EFI_ERROR(Status) && Gop && Gop->Mode) {
        Print(L"Configuring UEFI GOP framebuffer for kernel\n");
        Print(L"GOP Framebuffer: 0x%llx, %ux%u, %u bpp\n",
              Gop->Mode->FrameBufferBase,
              Gop->Mode->Info->HorizontalResolution,
              Gop->Mode->Info->VerticalResolution,
              Gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ? 32 : 24);
    }

    // Set up boot parameters
    EFI_PHYSICAL_ADDRESS BootParamsAddr = KernelBase + 0x1000;

    // Exit boot services and execute kernel
    Status = ExitBootServicesAndExecuteKernel(KernelBuffer, KernelSize, BootParamsAddr);

    if (MemMap) {
        FreePool(MemMap);
    }

    return Status;
}

/**
 * Exit boot services and execute kernel
 */
STATIC
EFI_STATUS
ExitBootServicesAndExecuteKernel (
  IN VOID* KernelBuffer,
  IN UINTN KernelSize,
  IN EFI_PHYSICAL_ADDRESS BootParamsAddr
  )
{
    EFI_STATUS Status;
    UINTN MapSize = 0, MapKey = 0, DescSize = 0;
    UINT32 DescVer = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;

    // Robustly get the memory map and exit boot services (handle concurrent map updates)
    Status = EFI_SUCCESS;
    const int max_retries = 8;
    int attempt = 0;
    while (attempt++ < max_retries) {
        MapSize = 0;
        Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        if (Status == EFI_BUFFER_TOO_SMALL) {
            if (MemMap) { FreePool(MemMap); MemMap = NULL; }
            MemMap = AllocatePool(MapSize);
            if (!MemMap) { Status = EFI_OUT_OF_RESOURCES; break; }
            Status = gBS->GetMemoryMap(&MapSize, MemMap, &MapKey, &DescSize, &DescVer);
        }

        if (EFI_ERROR(Status)) {
            CpuPause();
            continue;
        }

        Status = gBS->ExitBootServices(gImageHandle, MapKey);
        if (Status == EFI_INVALID_PARAMETER) {
            /* Map changed between GetMemoryMap and ExitBootServices; retry */
            CpuPause();
            continue;
        }

        /* Either succeeded or failed with a different error */
        break;
    }

    if (EFI_ERROR(Status)) {
        Print(L"Failed to exit boot services: %r\n", Status);
        if (MemMap) FreePool(MemMap);
        return Status;
    }

    // Set up boot parameters at specified address
    // Set up proper boot parameter structure that kernel can access
    COREBOOT_BOOT_PARAMS* boot_params = (COREBOOT_BOOT_PARAMS*)BootParamsAddr;

    // Zero out the boot parameters structure
    SetMem(boot_params, sizeof(COREBOOT_BOOT_PARAMS), 0);

    // Set up basic boot parameters
    boot_params->signature = COREBOOT_BOOT_SIGNATURE;
    boot_params->version = 1;
    boot_params->kernel_base = (UINT64)(UINTN)KernelBuffer;
    boot_params->kernel_size = KernelSize;
    boot_params->boot_flags = COREBOOT_BOOT_FLAG_KERNEL;

    // Set up framebuffer information if available (Coreboot or UEFI)
    if (gCorebootAvailable) {
        CONST COREBOOT_FB* framebuffer = CorebootGetFramebuffer();
        if (framebuffer) {
            boot_params->framebuffer_addr = framebuffer->physical_address;
            boot_params->framebuffer_width = framebuffer->x_resolution;
            boot_params->framebuffer_height = framebuffer->y_resolution;
            boot_params->framebuffer_bpp = framebuffer->bits_per_pixel;
            boot_params->framebuffer_pitch = framebuffer->bytes_per_line;
            boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
        }
    } else {
        EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
        Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
        if (!EFI_ERROR(Status) && Gop && Gop->Mode) {
            boot_params->framebuffer_addr = Gop->Mode->FrameBufferBase;
            boot_params->framebuffer_width = Gop->Mode->Info->HorizontalResolution;
            boot_params->framebuffer_height = Gop->Mode->Info->VerticalResolution;
            boot_params->framebuffer_bpp = Gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ? 32 : 24;
            boot_params->framebuffer_pitch = Gop->Mode->Info->PixelsPerScanLine * (boot_params->framebuffer_bpp / 8);
            boot_params->boot_flags |= COREBOOT_BOOT_FLAG_FRAMEBUFFER;
        }
    }

    // Set up memory information
    if (gCorebootAvailable) {
        UINT64 total_memory = CorebootGetTotalMemory();
        boot_params->memory_size = total_memory;
    } else {
        // Calculate total memory from UEFI memory map
        UINT64 total_memory = 0;
        for (UINTN i = 0; i < MapSize / DescSize; i++) {
            EFI_MEMORY_DESCRIPTOR* Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + i * DescSize);
            if (Desc->Type == EfiConventionalMemory || Desc->Type == EfiLoaderCode ||
                Desc->Type == EfiLoaderData || Desc->Type == EfiBootServicesCode ||
                Desc->Type == EfiBootServicesData) {
                total_memory += Desc->NumberOfPages * EFI_PAGE_SIZE;
            }
        }
        boot_params->memory_size = total_memory;
    }

    // Set up initrd if provided
    // Note: InitrdBuffer parameter is not used in this implementation
    // but would be set up here if needed

    // Validate boot parameters
    if (!ValidateBootParameters(boot_params)) {
        Print(L"Boot parameters validation failed\n");
        if (MemMap) FreePool(MemMap);
        return EFI_INVALID_PARAMETER;
    }

    Print(L"Boot parameters validated successfully\n");
    Print(L"Framebuffer: %ux%u @ 0x%llx\n",
          boot_params->framebuffer_width,
          boot_params->framebuffer_height,
          boot_params->framebuffer_addr);

    // Jump to kernel entry point
    typedef VOID (*KernelEntry)(COREBOOT_BOOT_PARAMS*);
    KernelEntry EntryPoint = (KernelEntry)KernelBuffer;

    Print(L"Jumping to kernel entry point at 0x%llx\n", (UINT64)(UINTN)KernelBuffer);

    // Execute kernel (this should not return)
    EntryPoint(boot_params);

    // If we get here, kernel execution failed
    if (MemMap) FreePool(MemMap);
    return EFI_LOAD_ERROR;
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

    // Validate initrd information if present
    if (boot_params->boot_flags & COREBOOT_BOOT_FLAG_INITRD) {
        if (boot_params->initrd_addr == 0) {
            Print(L"Invalid initrd address\n");
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Check if running as Coreboot payload and handle entry point
 */
STATIC
EFI_STATUS
HandleCorebootPayloadEntry (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
    // Check if we have Coreboot table available (indicates Coreboot payload)
    if (CorebootPlatformInit()) {
        Print(L"Running as Coreboot payload - using dedicated entry point\n");

        // Set flag for hybrid mode detection
        gRunningAsCorebootPayload = TRUE;
        gCorebootAvailable = TRUE;

        // Call Coreboot main entry point
        CorebootMain(NULL, NULL);

        // Should not return, but handle gracefully if it does
        return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND; // Not running as Coreboot payload
}

/**
 * Get root directory handle for filesystem operations
 */
EFI_STATUS
EFIAPI
get_root_dir (
  OUT EFI_FILE_HANDLE* root_dir
  )
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;

    // Get the loaded image protocol to find our device
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get loaded image protocol: %r\n", Status);
        return Status;
    }

    // Get the file system protocol for the device
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get file system protocol: %r\n", Status);
        return Status;
    }

    // Open the root directory
    Status = Fs->OpenVolume(Fs, root_dir);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open root volume: %r\n", Status);
        return Status;
    }

    return EFI_SUCCESS;
}
