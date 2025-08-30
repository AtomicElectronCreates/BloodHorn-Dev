/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#ifndef BLOODHORN_FONT_H
#define BLOODHORN_FONT_H

#include <stdint.h>
#include "compat.h"

// Font formats supported
typedef enum {
    FONT_FORMAT_BITMAP = 0,   // Simple bitmap font
    FONT_FORMAT_PSF,          // PC Screen Font
    FONT_FORMAT_TTF,          // TrueType Font (basic support)
    FONT_FORMAT_OTF           // OpenType Font (basic support)
} FontFormat;

// Font weight options
typedef enum {
    FONT_WEIGHT_REGULAR = 0,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_LIGHT
} FontWeight;

// Font style options
typedef enum {
    FONT_STYLE_NORMAL = 0,
    FONT_STYLE_ITALIC,
    FONT_STYLE_OBLIQUE
} FontStyle;

// Bitmap font glyph
typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t* bitmap;
} BitmapGlyph;

// Font metadata
typedef struct {
    const char* name;
    uint16_t size;
    FontWeight weight;
    FontStyle style;
    uint16_t line_height;
    uint16_t baseline;
    uint16_t max_width;
} FontMetadata;

// Generic font structure
typedef struct {
    FontFormat format;
    FontMetadata metadata;
    void* font_data;          // Format-specific font data
    uint32_t font_data_size;
    void* private_context;    // Font renderer context
} Font;

// Glyph rendering options
typedef struct {
    uint32_t color;
    uint32_t bg_color;
    uint8_t opacity;          // 0-255
    uint8_t use_bg;           // Boolean
    uint8_t antialias;        // Boolean
    uint8_t subpixel;         // Boolean
} GlyphRenderOptions;

// Text metrics
typedef struct {
    int32_t width;
    int32_t height;
    int32_t ascent;
    int32_t descent;
} TextMetrics;

// Font management functions
Font* LoadFontFromMemory(const void* data, uint32_t size, FontFormat format);
Font* LoadFontFile(const char* filename);
void UnloadFont(Font* font);

// Default built-in fonts
Font* GetDefaultFont(void);
Font* GetMonospaceFont(void);
Font* GetBoldFont(void);

// Text rendering functions
int32_t RenderGlyph(Font* font, uint32_t codepoint, int32_t x, int32_t y, GlyphRenderOptions* options);
int32_t RenderText(Font* font, const wchar_t* text, int32_t x, int32_t y, GlyphRenderOptions* options);
void MeasureText(Font* font, const wchar_t* text, TextMetrics* metrics);

// Font cache management
void InitFontSystem(void);
void ShutdownFontSystem(void);
void ClearFontCache(void);

#endif // BLOODHORN_FONT_H
