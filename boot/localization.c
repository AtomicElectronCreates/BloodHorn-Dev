/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */
#include "localization.h"
#include "compat.h"
#include <string.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Guid/FileInfo.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

static const struct { const char* key; const wchar_t* en; const wchar_t* es; } table[] = {
    {"menu_title", L"BloodHorn Boot Menu", L"Menú de Arranque BloodHorn"},
    {"select", L"Select", L"Seleccionar"},
    {"boot", L"Boot", L"Arrancar"},
    {"exit", L"Exit", L"Salir"},
    {"instructions", L"↑/↓: Select  Enter: Boot  ESC: Exit", L"↑/↓: Seleccionar  Enter: Arrancar  ESC: Salir"},
    {NULL, NULL, NULL}
};

static const char* current_lang = "en";

typedef struct { const char* key; wchar_t* value; } loc_kv_t;
static loc_kv_t* g_loc = NULL;
static UINTN g_loc_count = 0;

static VOID free_locale_table(void) {
    if (!g_loc) return;
    for (UINTN i = 0; i < g_loc_count; ++i) {
        if (g_loc[i].value) FreePool(g_loc[i].value);
    }
    FreePool(g_loc);
    g_loc = NULL; g_loc_count = 0;
}

static EFI_STATUS get_root(EFI_FILE_HANDLE* root) {
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Fs = NULL;
    extern EFI_HANDLE gImageHandle;
    extern EFI_BOOT_SERVICES* gBS;
    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) return Status;
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if (EFI_ERROR(Status)) return Status;
    return Fs->OpenVolume(Fs, root);
}

static EFI_STATUS read_text_file(CONST CHAR16* path, CHAR8** out, UINTN* out_len) {
    *out = NULL; *out_len = 0;
    EFI_FILE_HANDLE root = NULL, file = NULL; EFI_STATUS Status;
    Status = get_root(&root); if (EFI_ERROR(Status)) return Status;
    Status = root->Open(root, &file, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) return Status;
    EFI_FILE_INFO* info = NULL; UINTN info_sz = 0;
    Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_sz, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        info = AllocateZeroPool(info_sz);
        if (!info) { file->Close(file); return EFI_OUT_OF_RESOURCES; }
        Status = file->GetInfo(file, &gEfiFileInfoGuid, &info_sz, info);
    }
    if (EFI_ERROR(Status)) { if (info) FreePool(info); file->Close(file); return Status; }
    UINTN size = (UINTN)info->FileSize; CHAR8* buf = AllocateZeroPool(size + 1);
    if (!buf) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }
    Status = file->Read(file, &size, buf);
    file->Close(file); FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buf); return Status; }
    buf[size] = 0; *out = buf; *out_len = size; return EFI_SUCCESS;
}

static VOID parse_locale_ini(CHAR8* text) {
    free_locale_table();
    // Count lines with '='
    UINTN cap = 0, count = 0; CHAR8* p = text;
    while (*p) { CHAR8* e = p; while (*e && *e!='\n') e++; if (AsciiStrStr(p, "=")) count++; p = (*e=='\n') ? e+1 : e; }
    cap = count ? count : 1; g_loc = AllocateZeroPool(sizeof(loc_kv_t)*cap); g_loc_count = 0;
    p = text;
    while (*p) {
        CHAR8* e = p; while (*e && *e!='\n') e++; UINTN len = (UINTN)(e-p);
        CHAR8* line = AllocateZeroPool(len+1); if (!line) return; CopyMem(line,p,len); line[len]=0;
        // trim
        // skip comments
        if (line[0] != '#' && line[0] != ';' && AsciiStrStr(line, "=")) {
            CHAR8* eq = AsciiStrStr(line, "="); *eq = 0; CHAR8* k=line; CHAR8* v=eq+1;
            // trim simple
            while (*k==' '||*k=='\t') k++; while (*v==' '||*v=='\t') v++;
            UINTN vl = AsciiStrLen(v); while (vl>0 && (v[vl-1]=='\r'||v[vl-1]==' '||v[vl-1]=='\t')) { v[vl-1]=0; vl--; }
            // convert to wide
            UINTN wlen = AsciiStrLen(v);
            wchar_t* w = AllocateZeroPool((wlen+1)*sizeof(wchar_t));
            if (!w) { FreePool(line); return; }
            for (UINTN i=0;i<wlen;i++) w[i] = (wchar_t)(UINT8)v[i];
            // duplicate key string
            UINTN kl = AsciiStrLen(k);
            CHAR8* kdup = AllocateZeroPool(kl + 1);
            if (!kdup) { FreePool(w); FreePool(line); return; }
            CopyMem(kdup, k, kl);
            g_loc[g_loc_count].key = (const char*)kdup;
            g_loc[g_loc_count].value = (wchar_t*)w;
            g_loc_count++;
        }
        FreePool(line);
        p = (*e=='\n') ? e+1 : e;
    }
}

void SetLanguage(const char* lang_code) {
    current_lang = (lang_code && lang_code[0]) ? lang_code : "en";
    // Try to load locales/<lang>.ini if present
    CHAR16 path[64];
    path[0]=L'\0';
    UnicodeSPrint(path, sizeof(path), L"\\locales\\%a.ini", current_lang);
    CHAR8* buf = NULL; UINTN blen = 0;
    if (!EFI_ERROR(read_text_file(path, &buf, &blen)) && buf) {
        parse_locale_ini(buf);
        FreePool(buf);
    } else {
        free_locale_table();
    }
}

const char* GetCurrentLanguage(void) {
    return current_lang;
}

const wchar_t* GetLocalizedString(const char* key) {
    // First look in loaded locale table
    if (g_loc && key) {
        for (UINTN i=0;i<g_loc_count;i++) {
            if (g_loc[i].key && strcmp(g_loc[i].key, key) == 0) {
                return g_loc[i].value ? g_loc[i].value : L"";
            }
        }
    }
    // Fallback to built-ins (en/es)
    for (int i = 0; table[i].key != NULL; ++i) {
        if (strcmp(table[i].key, key) == 0) {
            if (strcmp(current_lang, "es") == 0)
                return table[i].es;
            else
                return table[i].en;
        }
    }
    return L"";
}