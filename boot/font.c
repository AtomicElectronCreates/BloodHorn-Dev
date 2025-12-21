/*
 * font.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include "font.h"
#include "compat.h"
#include "../uefi/graphics.h"
#include <string.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Guid/FileInfo.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include "freetype/ft2build.h"
#include FT_FREETYPE_H
#include "font_data.h"
#include "font_mono_8x8.h"
#include "font_bold_8x16.h"

// Built-in 8x16 bitmap font (ASCII 32-126)
static const uint8_t builtin_font_8x16[] = complete_font_8x16;

// Built-in monospace font (8x8)
static const uint8_t builtin_mono_font_8x8[] = complete_mono_font_8x8;

// Built-in bold font (8x16 bold)
static const uint8_t builtin_bold_font_8x16[] = complete_bold_font_8x16;

// Font cache
#define MAX_CACHED_FONTS 16
static Font* g_font_cache[MAX_CACHED_FONTS];
static int g_font_cache_count = 0;
static Font* g_default_font = NULL;
static Font* g_mono_font = NULL;
static Font* g_bold_font = NULL;

// Initialize default fonts
static void InitBuiltinFonts(void) {
    // Create default font
    g_default_font = (Font*)malloc(sizeof(Font));
    if (g_default_font) {
        g_default_font->format = FONT_FORMAT_BITMAP;
        g_default_font->metadata.name = "Built-in Default";
        g_default_font->metadata.size = 16;
        g_default_font->metadata.weight = FONT_WEIGHT_REGULAR;
        g_default_font->metadata.style = FONT_STYLE_NORMAL;
        g_default_font->metadata.line_height = 16;
        g_default_font->metadata.baseline = 12;
        g_default_font->metadata.max_width = 8;
        g_default_font->font_data = (void*)builtin_font_8x16;
        g_default_font->font_data_size = sizeof(builtin_font_8x16);
        g_default_font->private_context = NULL;
    }
    
    // Create monospace font
    g_mono_font = (Font*)malloc(sizeof(Font));
    if (g_mono_font) {
        g_mono_font->format = FONT_FORMAT_BITMAP;
        g_mono_font->metadata.name = "Built-in Monospace";
        g_mono_font->metadata.size = 8;
        g_mono_font->metadata.weight = FONT_WEIGHT_REGULAR;
        g_mono_font->metadata.style = FONT_STYLE_NORMAL;
        g_mono_font->metadata.line_height = 8;
        g_mono_font->metadata.baseline = 6;
        g_mono_font->metadata.max_width = 8;
        g_mono_font->font_data = (void*)builtin_mono_font_8x8;
        g_mono_font->font_data_size = sizeof(builtin_mono_font_8x8);
        g_mono_font->private_context = NULL;
    }
    
    // Create bold font
    g_bold_font = (Font*)malloc(sizeof(Font));
    if (g_bold_font) {
        g_bold_font->format = FONT_FORMAT_BITMAP;
        g_bold_font->metadata.name = "Built-in Bold";
        g_bold_font->metadata.size = 16;
        g_bold_font->metadata.weight = FONT_WEIGHT_BOLD;
        g_bold_font->metadata.style = FONT_STYLE_NORMAL;
        g_bold_font->metadata.line_height = 16;
        g_bold_font->metadata.baseline = 12;
        g_bold_font->metadata.max_width = 8;
        g_bold_font->font_data = (void*)builtin_bold_font_8x16;
        g_bold_font->font_data_size = sizeof(builtin_bold_font_8x16);
        g_bold_font->private_context = NULL;
    }
}

void InitFontSystem(void) {
    memset(g_font_cache, 0, sizeof(g_font_cache));
    g_font_cache_count = 0;
    InitBuiltinFonts();
}

void ShutdownFontSystem(void) {
    for (int i = 0; i < g_font_cache_count; i++) {
        if (g_font_cache[i]) {
            UnloadFont(g_font_cache[i]);
        }
    }
    
    if (g_default_font) {
        free(g_default_font);
        g_default_font = NULL;
    }
    if (g_mono_font) {
        free(g_mono_font);
        g_mono_font = NULL;
    }
    if (g_bold_font) {
        free(g_bold_font);
        g_bold_font = NULL;
    }
    
    g_font_cache_count = 0;
}

Font* GetDefaultFont(void) {
    return g_default_font;
}

Font* GetMonospaceFont(void) {
    return g_mono_font;
}

Font* GetBoldFont(void) {
    return g_bold_font;
}

void SetDefaultFont(Font* font) {
    if (font) {
        g_default_font = font;
    }
}

FT_Library ft_library = NULL;

static int32_t RenderPsfGlyph(Font* font, uint32_t codepoint, int32_t x, int32_t y, GlyphRenderOptions* options) {
    if (!font || !font->font_data || codepoint >= 256) {
        return 0;
    }
    
    const uint8_t* font_data = (const uint8_t*)font->font_data;
    uint8_t height = font->metadata.line_height;
    uint32_t bytes_per_glyph = height; // 1 byte per row for 8-pixel wide PSF
    const uint8_t* glyph_data = &font_data[codepoint * bytes_per_glyph];
    
    // Render the PSF bitmap
    for (int row = 0; row < height; row++) {
        uint8_t byte_data = glyph_data[row];
        for (int col = 0; col < 8; col++) {
            if (byte_data & (0x80 >> col)) {
                // Pixel is set, draw it
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                if (GraphicsOutput) {
                    uint32_t* framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
                    uint32_t pixels_per_scanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
                    
                    if (pixel_x < GraphicsOutput->Mode->Info->HorizontalResolution &&
                        pixel_y < GraphicsOutput->Mode->Info->VerticalResolution) {
                        framebuffer[pixel_y * pixels_per_scanline + pixel_x] = options->color;
                    }
                }
            } else if (options->use_bg) {
                // Draw background pixel
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                
                if (GraphicsOutput) {
                    uint32_t* framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
                    uint32_t pixels_per_scanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
                    
                    if (pixel_x < GraphicsOutput->Mode->Info->HorizontalResolution &&
                        pixel_y < GraphicsOutput->Mode->Info->VerticalResolution) {
                        framebuffer[pixel_y * pixels_per_scanline + pixel_x] = options->bg_color;
                    }
                }
            }
        }
    }
    
    return font->metadata.max_width; // PSF fonts are typically 8 pixels wide
}

static int32_t RenderBitmapGlyph(Font* font, uint32_t codepoint, int32_t x, int32_t y, GlyphRenderOptions* options) {
    if (!font || !font->font_data || codepoint < 32 || codepoint > 126) {
        return 0;
    }
    
    const uint8_t* font_data = (const uint8_t*)font->font_data;
    uint32_t glyph_index = codepoint - 32;
    uint32_t bytes_per_glyph = (font->metadata.line_height * font->metadata.max_width + 7) / 8;
    
    if (font->metadata.line_height == 16 && font->metadata.max_width == 8) {
        bytes_per_glyph = 16; // 8x16 = 16 bytes per character
    } else if (font->metadata.line_height == 8 && font->metadata.max_width == 8) {
        bytes_per_glyph = 8;  // 8x8 = 8 bytes per character
    }
    
    const uint8_t* glyph_data = &font_data[glyph_index * bytes_per_glyph];
    
    // Render the bitmap
    for (int row = 0; row < font->metadata.line_height; row++) {
        uint8_t byte_data = glyph_data[row];
        for (int col = 0; col < 8; col++) {
            if (byte_data & (0x80 >> col)) {
                // Pixel is set, draw it
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                // Pixel Drawing starts from here.
                if (GraphicsOutput) {
                    uint32_t* framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
                    uint32_t pixels_per_scanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
                    
                    if (pixel_x < GraphicsOutput->Mode->Info->HorizontalResolution &&
                        pixel_y < GraphicsOutput->Mode->Info->VerticalResolution) {
                        framebuffer[pixel_y * pixels_per_scanline + pixel_x] = options->color;
                    }
                }
            } else if (options->use_bg) {
                // Draw background pixel
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                
                if (GraphicsOutput) {
                    uint32_t* framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
                    uint32_t pixels_per_scanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
                    
                    if (pixel_x < GraphicsOutput->Mode->Info->HorizontalResolution &&
                        pixel_y < GraphicsOutput->Mode->Info->VerticalResolution) {
                        framebuffer[pixel_y * pixels_per_scanline + pixel_x] = options->bg_color;
                    }
                }
            }
        }
    }
    
    return font->metadata.max_width;
}

int32_t RenderGlyph(Font* font, uint32_t codepoint, int32_t x, int32_t y, GlyphRenderOptions* options) {
    if (!font || !options) return 0;
    
    switch (font->format) {
        case FONT_FORMAT_BITMAP:
            return RenderBitmapGlyph(font, codepoint, x, y, options);
        case FONT_FORMAT_PSF:
            return RenderPsfGlyph(font, codepoint, x, y, options);
        case FONT_FORMAT_TTF:
        case FONT_FORMAT_OTF: {
            if (!ft_library) {
                FT_Error error = FT_Init_FreeType(&ft_library);
                if (error) {
                    return 0;
                }
            }
            
            FT_Face face;
            FT_Error error = FT_New_Memory_Face(ft_library, font->font_data, font->font_data_size, 0, &face);
            if (error) {
                return 0;
            }
            
            error = FT_Set_Char_Size(face, 0, font->metadata.size * 64, 72, 72);
            if (error) {
                FT_Done_Face(face);
                return 0;
            }
            
            error = FT_Load_Char(face, codepoint, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
            if (error) {
                FT_Done_Face(face);
                return 0;
            }
            
            FT_Bitmap* bitmap = &face->glyph->bitmap;
            int32_t bitmap_left = face->glyph->bitmap_left;
            int32_t bitmap_top = face->glyph->bitmap_top;
            
            for (int row = 0; row < bitmap->rows; row++) {
                for (int col = 0; col < bitmap->width; col++) {
                    uint32_t pixel_x = x + col + bitmap_left;
                    uint32_t pixel_y = y + bitmap_top - row - 1;
                    
                    if (pixel_x < GraphicsOutput->Mode->Info->HorizontalResolution &&
                        pixel_y < GraphicsOutput->Mode->Info->VerticalResolution) {
                        uint32_t* framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
                        uint32_t pixels_per_scanline = GraphicsOutput->Mode->Info->PixelsPerScanLine;
                        
                        uint8_t alpha = bitmap->buffer[row * bitmap->pitch + col];
                        if (alpha > 0) {
                            uint32_t color = options->color;
                            if (options->use_bg && alpha < 255) {
                                uint32_t bg_color = options->bg_color;
                                // Alpha blending
                                uint32_t result_color = ((color & 0xFF) * alpha + (bg_color & 0xFF) * (255 - alpha)) / 255 |
                                                      (((color & 0xFF00) >> 8) * alpha + ((bg_color & 0xFF00) >> 8) * (255 - alpha)) / 255 << 8 |
                                                      (((color & 0xFF0000) >> 16) * alpha + ((bg_color & 0xFF0000) >> 16) * (255 - alpha)) / 255 << 16;
                                framebuffer[pixel_y * pixels_per_scanline + pixel_x] = result_color;
                            } else {
                                framebuffer[pixel_y * pixels_per_scanline + pixel_x] = color;
                            }
                        } else if (options->use_bg) {
                            framebuffer[pixel_y * pixels_per_scanline + pixel_x] = options->bg_color;
                        }
                    }
                }
            }
            
            int32_t advance = face->glyph->advance.x >> 6;
            FT_Done_Face(face);
            return advance > 0 ? advance : font->metadata.max_width;
        }
        default:
            return 0;
    }
}

int32_t RenderText(Font* font, const wchar_t* text, int32_t x, int32_t y, GlyphRenderOptions* options) {
    if (!font || !text || !options) return 0;
    
    int32_t current_x = x;
    int32_t text_width = 0;
    
    while (*text) {
        uint32_t codepoint = *text;
        int32_t glyph_width = RenderGlyph(font, codepoint, current_x, y, options);
        current_x += glyph_width;
        text_width += glyph_width;
        text++;
    }
    
    return text_width;
}

void MeasureText(Font* font, const wchar_t* text, TextMetrics* metrics) {
    if (!font || !text || !metrics) return;
    
    metrics->width = 0;
    metrics->height = font->metadata.line_height;
    metrics->ascent = font->metadata.baseline;
    metrics->descent = font->metadata.line_height - font->metadata.baseline;
    
    while (*text) {
        if (*text >= 32 && *text <= 126) {
            metrics->width += font->metadata.max_width;
        }
        text++;
    }
}

Font* LoadFontFromMemory(const void* data, uint32_t size, FontFormat format) {
    if (!data || size == 0) return NULL;
    
    Font* font = (Font*)malloc(sizeof(Font));
    if (!font) return NULL;
    
    font->format = format;
    font->font_data = malloc(size);
    if (!font->font_data) {
        free(font);
        return NULL;
    }
    
    memcpy(font->font_data, data, size);
    font->font_data_size = size;
    font->private_context = NULL;
    
    // Set default metadata (would be parsed from font file in real implementation)
    font->metadata.name = "Custom Font";
    font->metadata.size = 16;
    font->metadata.weight = FONT_WEIGHT_REGULAR;
    font->metadata.style = FONT_STYLE_NORMAL;
    font->metadata.line_height = 16;
    font->metadata.baseline = 12;
    font->metadata.max_width = 8;
    
    // Add to cache
    if (g_font_cache_count < MAX_CACHED_FONTS) {
        g_font_cache[g_font_cache_count++] = font;
    }
    
    return font;
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

static EFI_STATUS read_file(CONST CHAR16* path, VOID** out, UINTN* out_size) {
    *out = NULL; *out_size = 0;
    EFI_STATUS Status; EFI_FILE_HANDLE root=NULL, file=NULL;
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
    UINTN size = (UINTN)info->FileSize; VOID* buf = AllocateZeroPool(size);
    if (!buf) { FreePool(info); file->Close(file); return EFI_OUT_OF_RESOURCES; }
    Status = file->Read(file, &size, buf);
    file->Close(file); FreePool(info);
    if (EFI_ERROR(Status)) { FreePool(buf); return Status; }
    *out = buf; *out_size = size; return EFI_SUCCESS;
}

// Enhanced PSF1/PSF2 detection and extraction
static BOOLEAN parse_psf1(const UINT8* data, UINTN size, UINT8* out_height, const UINT8** glyphs, UINTN* glyph_count) {
    if (size < 4) return FALSE;
    // PSF1 magic 0x36 0x04
    if (!(data[0] == 0x36 && data[1] == 0x04)) return FALSE;
    UINT8 mode = data[2];
    UINT8 charsize = data[3];
    *out_height = charsize;
    *glyph_count = (mode & 0x01) ? 512 : 256;
    UINTN table_off = 4; // followed by glyphs
    if (size < table_off + (UINTN)charsize * (*glyph_count)) return FALSE;
    *glyphs = data + table_off;
    return TRUE;
}

static BOOLEAN parse_psf2(const UINT8* data, UINTN size, UINT8* out_height, UINT8* out_width, const UINT8** glyphs, UINTN* glyph_count) {
    if (size < 32) return FALSE;
    // PSF2 magic
    if (!(data[0] == 0x72 && data[1] == 0xb5 && data[2] == 0x4a && data[3] == 0x86)) return FALSE;
    
    UINT32 version = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    UINT32 headersize = data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24);
    UINT32 flags = data[12] | (data[13] << 8) | (data[14] << 16) | (data[15] << 24);
    UINT32 numglyphs = data[16] | (data[17] << 8) | (data[18] << 16) | (data[19] << 24);
    UINT32 bytesperglyph = data[20] | (data[21] << 8) | (data[22] << 16) | (data[23] << 24);
    UINT32 height = data[24] | (data[25] << 8) | (data[26] << 16) | (data[27] << 24);
    UINT32 width = data[28] | (data[29] << 8) | (data[30] << 16) | (data[31] << 24);
    
    *out_height = (UINT8)height;
    *out_width = (UINT8)width;
    *glyph_count = numglyphs;
    
    if (headersize > size || size < headersize + bytesperglyph * numglyphs) return FALSE;
    *glyphs = data + headersize;
    return TRUE;
}

static BOOLEAN is_ttf_font(const UINT8* data, UINTN size) {
    if (size < 4) return FALSE;
    // Check for TrueType/OpenType signatures
    return (data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0x00) || // TrueType
           (data[0] == 0x4F && data[1] == 0x54 && data[2] == 0x54 && data[3] == 0x4F) || // OpenType with CFF
           (data[0] == 0x74 && data[1] == 0x72 && data[2] == 0x75 && data[3] == 0x65);   // "true" at start
}

Font* LoadFontFile(const char* filename) {
    if (!filename || !filename[0]) return GetDefaultFont();
    // Convert ASCII path to UCS-2
    CHAR16 wpath[256];
    UnicodeSPrint(wpath, sizeof(wpath), L"%a", filename);
    VOID* filebuf = NULL; UINTN filesz = 0;
    if (EFI_ERROR(read_file(wpath, &filebuf, &filesz)) || !filebuf || filesz < 4) {
        if (filebuf) FreePool(filebuf);
        return GetDefaultFont();
    }
    const UINT8* bytes = (const UINT8*)filebuf;
    
    // Try PSF2 first (more capable)
    UINT8 height = 0, width = 0; const UINT8* glyphs = NULL; UINTN glyph_count = 0;
    if (parse_psf2(bytes, filesz, &height, &width, &glyphs, &glyph_count)) {
        UINTN bytes_per_row = (width + 7) / 8;
        UINTN table_sz = bytes_per_row * height * glyph_count;
        UINT8* table = AllocateZeroPool(table_sz);
        if (!table) { FreePool(filebuf); return GetDefaultFont(); }
        CopyMem(table, glyphs, table_sz);
        
        Font* font = (Font*)malloc(sizeof(Font));
        if (!font) { FreePool(table); FreePool(filebuf); return GetDefaultFont(); }
        font->format = FONT_FORMAT_PSF;
        font->metadata.name = "PSF2";
        font->metadata.size = height;
        font->metadata.weight = FONT_WEIGHT_REGULAR;
        font->metadata.style = FONT_STYLE_NORMAL;
        font->metadata.line_height = height;
        font->metadata.baseline = (height > 4) ? (height - 4) : height;
        font->metadata.max_width = width;
        font->font_data = table;
        font->font_data_size = (uint32_t)table_sz;
        font->private_context = filebuf;
        
        if (g_font_cache_count < MAX_CACHED_FONTS) g_font_cache[g_font_cache_count++] = font;
        return font;
    }
    
    // Try PSF1
    if (parse_psf1(bytes, filesz, &height, &glyphs, &glyph_count)) {
        UINTN table_sz = height * glyph_count; // 1 byte per row, width=8
        UINT8* table = AllocateZeroPool(table_sz);
        if (!table) { FreePool(filebuf); return GetDefaultFont(); }
        CopyMem(table, glyphs, table_sz);
        
        Font* font = (Font*)malloc(sizeof(Font));
        if (!font) { FreePool(table); FreePool(filebuf); return GetDefaultFont(); }
        font->format = FONT_FORMAT_PSF;
        font->metadata.name = "PSF1";
        font->metadata.size = height;
        font->metadata.weight = FONT_WEIGHT_REGULAR;
        font->metadata.style = FONT_STYLE_NORMAL;
        font->metadata.line_height = height;
        font->metadata.baseline = (height > 4) ? (height - 4) : height;
        font->metadata.max_width = 8;
        font->font_data = table;
        font->font_data_size = (uint32_t)table_sz;
        font->private_context = filebuf;
        
        if (g_font_cache_count < MAX_CACHED_FONTS) g_font_cache[g_font_cache_count++] = font;
        return font;
    }
    
    // Try TTF/OTF
    if (is_ttf_font(bytes, filesz)) {
        Font* font = (Font*)malloc(sizeof(Font));
        if (!font) { FreePool(filebuf); return GetDefaultFont(); }
        
        font->format = (bytes[0] == 0x4F && bytes[1] == 0x54 && bytes[2] == 0x54 && bytes[3] == 0x4F) ? FONT_FORMAT_OTF : FONT_FORMAT_TTF;
        font->font_data = malloc(filesz);
        if (!font->font_data) {
            free(font);
            FreePool(filebuf);
            return GetDefaultFont();
        }
        
        memcpy(font->font_data, bytes, filesz);
        font->font_data_size = (uint32_t)filesz;
        font->private_context = NULL;
        
        // Set default TTF metadata
        font->metadata.name = (font->format == FONT_FORMAT_OTF) ? "OpenType Font" : "TrueType Font";
        font->metadata.size = 16; // Default size
        font->metadata.weight = FONT_WEIGHT_REGULAR;
        font->metadata.style = FONT_STYLE_NORMAL;
        font->metadata.line_height = 16;
        font->metadata.baseline = 12;
        font->metadata.max_width = 8;
        
        if (g_font_cache_count < MAX_CACHED_FONTS) g_font_cache[g_font_cache_count++] = font;
        FreePool(filebuf);
        return font;
    }
    
    // Unsupported font type -> fallback
    FreePool(filebuf);
    return GetDefaultFont();
}

void UnloadFont(Font* font) {
    if (!font) return;
    
    // Remove from cache
    for (int i = 0; i < g_font_cache_count; i++) {
        if (g_font_cache[i] == font) {
            for (int j = i; j < g_font_cache_count - 1; j++) {
                g_font_cache[j] = g_font_cache[j + 1];
            }
            g_font_cache_count--;
            break;
        }
    }
    
    if (font->font_data) {
        free(font->font_data);
    }
    if (font->private_context) {
        FreePool(font->private_context);
    }
    free(font);
}

void ClearFontCache(void) {
    for (int i = 0; i < g_font_cache_count; i++) {
        if (g_font_cache[i] && 
            g_font_cache[i] != g_default_font && 
            g_font_cache[i] != g_mono_font && 
            g_font_cache[i] != g_bold_font) {
            UnloadFont(g_font_cache[i]);
        }
    }
    g_font_cache_count = 3; // Keep the three built-in fonts
}
