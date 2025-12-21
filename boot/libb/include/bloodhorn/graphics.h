/*
 * graphics.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_GRAPHICS_H
#define BLOODHORN_GRAPHICS_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pixel format definitions
typedef enum {
    BH_PIXEL_FORMAT_UNKNOWN = 0,
    BH_PIXEL_FORMAT_RGBA8888,   // 32-bit RGBA (8:8:8:8)
    BH_PIXEL_FORMAT_BGRA8888,   // 32-bit BGRA (8:8:8:8)
    BH_PIXEL_FORMAT_RGB888,     // 24-bit RGB (8:8:8)
    BH_PIXEL_FORMAT_BGR888,     // 24-bit BGR (8:8:8)
    BH_PIXEL_FORMAT_RGB565,     // 16-bit RGB (5:6:5)
    BH_PIXEL_FORMAT_BGR565,     // 16-bit BGR (5:6:5)
    BH_PIXEL_FORMAT_GRAY8,      // 8-bit grayscale
    BH_PIXEL_FORMAT_ALPHA8,     // 8-bit alpha
    BH_PIXEL_FORMAT_RGBA4444,   // 16-bit RGBA (4:4:4:4)
    BH_PIXEL_FORMAT_RGBA5551,   // 16-bit RGBA (5:5:5:1)
    BH_PIXEL_FORMAT_MAX
} bh_pixel_format_t;

// Color structure
typedef struct {
    union {
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };
        uint32_t value;
    };
} bh_color_t;

// Rectangle structure
typedef struct {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} bh_rect_t;

// Point structure
typedef struct {
    int32_t x;
    int32_t y;
} bh_point_t;

// Display mode information
typedef struct {
    uint32_t width;             // Horizontal resolution in pixels
    uint32_t height;            // Vertical resolution in pixels
    uint32_t bpp;               // Bits per pixel
    uint32_t pitch;             // Bytes per scanline
    uint32_t refresh_rate;      // Refresh rate in Hz
    bh_pixel_format_t format;   // Pixel format
} bh_display_mode_t;

// Framebuffer information
typedef struct {
    void* address;              // Framebuffer base address
    bh_size_t size;             // Total size in bytes
    uint32_t width;             // Width in pixels
    uint32_t height;            // Height in pixels
    uint32_t pitch;             // Bytes per scanline
    uint8_t bpp;                // Bits per pixel
    uint8_t red_mask_size;      // Red channel mask size
    uint8_t red_field_position; // Red field position
    uint8_t green_mask_size;    // Green channel mask size
    uint8_t green_field_position; // Green field position
    uint8_t blue_mask_size;     // Blue channel mask size
    uint8_t blue_field_position; // Blue field position
    uint8_t reserved_mask_size;  // Reserved mask size
    uint8_t reserved_field_position; // Reserved field position
    bh_pixel_format_t format;   // Pixel format
} bh_framebuffer_t;

// Graphics context (opaque type)
typedef struct bh_graphics_context bh_graphics_context_t;

// Font information
typedef struct {
    const char* name;           // Font name
    uint32_t size;              // Font size in points
    uint32_t weight;            // Font weight (100-900)
    bool italic;                // Italic style
    bool bold;                  // Bold style
    bool underline;             // Underline style
    bool strikeout;             // Strikeout style
} bh_font_t;

// Image information
typedef struct {
    uint32_t width;             // Image width in pixels
    uint32_t height;            // Image height in pixels
    uint32_t bpp;               // Bits per pixel
    uint32_t pitch;             // Bytes per scanline
    bh_pixel_format_t format;   // Pixel format
    const void* data;           // Image data
    bh_size_t data_size;        // Size of image data in bytes
} bh_image_t;

// Blend modes
typedef enum {
    BH_BLEND_MODE_NONE,         // No blending
    BH_BLEND_MODE_ALPHA,        // Alpha blending
    BH_BLEND_MODE_ADD,          // Additive blending
    BH_BLEND_MODE_MULTIPLY,     // Multiply blending
    BH_BLEND_MODE_SCREEN,       // Screen blending
    BH_BLEND_MODE_OVERLAY,      // Overlay blending
    BH_BLEND_MODE_DARKEN,       // Darken blending
    BH_BLEND_MODE_LIGHTEN,      // Lighten blending
    BH_BLEND_MODE_MAX
} bh_blend_mode_t;

// Graphics initialization flags
typedef enum {
    BH_GRAPHICS_FLAG_NONE = 0,
    BH_GRAPHICS_FLAG_DOUBLE_BUFFER = 1 << 0,    // Enable double buffering
    BH_GRAPHICS_FLAG_VSYNC = 1 << 1,           // Enable vertical sync
    BH_GRAPHICS_FLAG_HARDWARE_ACCEL = 1 << 2,   // Enable hardware acceleration
    BH_GRAPHICS_FLAG_FULLSCREEN = 1 << 3,       // Start in fullscreen mode
    BH_GRAPHICS_FLAG_DEBUG = 1 << 4             // Enable debug output
} bh_graphics_flags_t;

// Graphics API types
typedef enum {
    BH_GRAPHICS_API_AUTO,       // Auto-detect best API
    BH_GRAPHICS_API_VULKAN,     // Vulkan
    BH_GRAPHICS_API_OPENGL,     // OpenGL
    BH_GRAPHICS_API_OPENGL_ES,  // OpenGL ES
    BH_GRAPHICS_API_DIRECT3D,   // Direct3D
    BH_GRAPHICS_API_METAL,      // Metal
    BH_GRAPHICS_API_SOFTWARE    // Software rendering
} bh_graphics_api_t;

// Graphics initialization parameters
typedef struct {
    const char* application_name;    // Application name
    uint32_t application_version;    // Application version
    const char* engine_name;         // Engine name
    uint32_t engine_version;         // Engine version
    uint32_t width;                  // Initial window width
    uint32_t height;                 // Initial window height
    uint32_t refresh_rate;           // Refresh rate (0 = default)
    bh_graphics_flags_t flags;       // Graphics flags
    bh_graphics_api_t api;           // Graphics API to use
    bh_pixel_format_t format;        // Preferred pixel format
    uint32_t max_frames_in_flight;   // Maximum frames in flight (0 = default)
    uint32_t msaa_samples;           // MSAA samples (0 = disabled)
} bh_graphics_init_params_t;

// Graphics capabilities
typedef struct {
    const char* api_name;            // Graphics API name
    uint32_t api_version;            // Graphics API version
    const char* gpu_name;            // GPU name
    const char* gpu_vendor;          // GPU vendor
    uint32_t max_texture_size;       // Maximum texture size
    uint32_t max_texture_units;      // Maximum texture units
    uint32_t max_render_targets;     // Maximum render targets
    uint32_t max_vertex_attributes;  // Maximum vertex attributes
    uint32_t max_uniform_blocks;     // Maximum uniform blocks
} bh_graphics_capabilities_t;

// Function declarations

/**
 * @brief Initialize the graphics system
 * 
 * @param params Initialization parameters
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_initialize(const bh_graphics_init_params_t* params);

/**
 * @brief Shutdown the graphics system
 */
void bh_graphics_shutdown(void);

/**
 * @brief Get the graphics context
 * 
 * @return bh_graphics_context_t* Graphics context, or NULL if not initialized
 */
bh_graphics_context_t* bh_graphics_get_context(void);

/**
 * @brief Get the framebuffer
 * 
 * @param fb [out] Framebuffer information
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_get_framebuffer(bh_framebuffer_t* fb);

/**
 * @brief Clear the framebuffer
 * 
 * @param color Clear color
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_clear(bh_color_t color);

/**
 * @brief Present the framebuffer
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_present(void);

/**
 * @brief Get the display mode
 * 
 * @param mode [out] Display mode information
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_get_display_mode(bh_display_mode_t* mode);

/**
 * @brief Set the display mode
 * 
 * @param mode Display mode to set
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_display_mode(const bh_display_mode_t* mode);

/**
 * @brief Get the graphics capabilities
 * 
 * @param caps [out] Capabilities structure to fill
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_get_capabilities(bh_graphics_capabilities_t* caps);

/**
 * @brief Create a graphics context
 * 
 * @param ctx [out] Pointer to store the new context
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_context_create(bh_graphics_context_t** ctx);

/**
 * @brief Destroy a graphics context
 * 
 * @param ctx Context to destroy
 */
void bh_graphics_context_destroy(bh_graphics_context_t* ctx);

/**
 * @brief Set the current graphics context
 * 
 * @param ctx Context to make current
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_context_make_current(bh_graphics_context_t* ctx);

/**
 * @brief Get the current graphics context
 * 
 * @return bh_graphics_context_t* Current context, or NULL if none
 */
bh_graphics_context_t* bh_graphics_context_get_current(void);

/**
 * @brief Swap the front and back buffers
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_context_swap_buffers(void);

/**
 * @brief Set the viewport
 * 
 * @param x X position
 * @param y Y position
 * @param width Viewport width
 * @param height Viewport height
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Set the scissor rectangle
 * 
 * @param x X position
 * @param y Y position
 * @param width Scissor width
 * @param height Scissor height
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_scissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Set the blend mode
 * 
 * @param mode Blend mode
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_blend_mode(bh_blend_mode_t mode);

/**
 * @brief Draw a rectangle
 * 
 * @param rect Rectangle to draw
 * @param color Fill color
 * @param filled Whether to fill the rectangle
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_draw_rect(const bh_rect_t* rect, bh_color_t color, bool filled);

/**
 * @brief Draw a line
 * 
 * @param x1 Start X position
 * @param y1 Start Y position
 * @param x2 End X position
 * @param y2 End Y position
 * @param color Line color
 * @param thickness Line thickness
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, bh_color_t color, uint32_t thickness);

/**
 * @brief Draw text
 * 
 * @param text Text to draw
 * @param x X position
 * @param y Y position
 * @param color Text color
 * @param font Font to use (NULL for default)
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_draw_text(const char* text, int32_t x, int32_t y, bh_color_t color, const bh_font_t* font);

/**
 * @brief Draw an image
 * 
 * @param image Image to draw
 * @param x X position
 * @param y Y position
 * @param width Width to draw (0 for image width)
 * @param height Height to draw (0 for image height)
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_draw_image(const bh_image_t* image, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Load an image from memory
 * 
 * @param data Image data
 * @param size Size of image data
 * @param image [out] Loaded image
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_load_image(const void* data, bh_size_t size, bh_image_t* image);

/**
 * @brief Free image resources
 * 
 * @param image Image to free
 */
void bh_graphics_free_image(bh_image_t* image);

/**
 * @brief Load a font
 * 
 * @param name Font name
 * @param size Font size
 * @param font [out] Loaded font
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_load_font(const char* name, uint32_t size, bh_font_t* font);

/**
 * @brief Free font resources
 * 
 * @param font Font to free
 */
void bh_graphics_free_font(bh_font_t* font);

/**
 * @brief Get text dimensions
 * 
 * @param text Text to measure
 * @param font Font to use (NULL for default)
 * @param width [out] Text width
 * @param height [out] Text height
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_measure_text(const char* text, const bh_font_t* font, uint32_t* width, uint32_t* height);

/**
 * @brief Create a texture
 * 
 * @param width Texture width
 * @param height Texture height
 * @param format Pixel format
 * @param data Initial data (can be NULL)
 * @return void* Texture handle, or NULL on failure
 */
void* bh_graphics_create_texture(uint32_t width, uint32_t height, bh_pixel_format_t format, const void* data);

/**
 * @brief Update a texture
 * 
 * @param texture Texture to update
 * @param x X offset
 * @param y Y offset
 * @param width Width of region to update
 * @param height Height of region to update
 * @param format Pixel format of data
 * @param data New texture data
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_update_texture(void* texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bh_pixel_format_t format, const void* data);

/**
 * @brief Destroy a texture
 * 
 * @param texture Texture to destroy
 */
void bh_graphics_destroy_texture(void* texture);

/**
 * @brief Set the current texture
 * 
 * @param texture Texture to set (can be NULL)
 * @param unit Texture unit (0-31)
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_texture(void* texture, uint32_t unit);

/**
 * @brief Set the clear color
 * 
 * @param color Clear color
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_set_clear_color(bh_color_t color);

/**
 * @brief Begin a new frame
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_begin_frame(void);

/**
 * @brief End the current frame
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_graphics_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_GRAPHICS_H
