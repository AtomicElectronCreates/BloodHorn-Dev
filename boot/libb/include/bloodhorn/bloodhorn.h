/*
 * bloodhorn.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_H
#define BLOODHORN_H

// Core includes
#include <bloodhorn/types.h>
#include <bloodhorn/status.h>
#include <bloodhorn/memory.h>
#include <bloodhorn/graphics.h>
#include <bloodhorn/input.h>
#include <bloodhorn/fs.h>
#include <bloodhorn/time.h>
#include <bloodhorn/debug.h>

// System table and initialization
#include <bloodhorn/system.h>

// Boot information
#include <bloodhorn/bootinfo.h>

// Version information
#define BLOODHORN_VERSION_MAJOR 1
#define BLOODHORN_VERSION_MINOR 0
#define BLOODHORN_VERSION_PATCH 0

// Initialization function (called by bootloader)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the BloodHorn library
 * @param system_table Pointer to the system table
 * @return Status code indicating success or failure
 */
bh_status_t bh_initialize(bh_system_table_t* system_table);

/**
 * @brief Get library version information
 * @param[out] major Major version number
 * @param[out] minor Minor version number
 * @param[out] patch Patch version number
 */
void bh_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch);

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_H

// Basic type definitions
typedef uint32_t bh_size_t;
typedef uint64_t bh_addr_t;
typedef uint32_t bh_status_t;

// Status codes
#define BH_SUCCESS                0
#define BH_ERROR                 -1
#define BH_INVALID_PARAMETER     -2
#define BH_NOT_SUPPORTED         -3
#define BH_OUT_OF_RESOURCES      -4
#define BH_DEVICE_ERROR          -5

// Memory attributes
#define BH_MEMORY_UC             (1 << 0)  // Uncacheable
#define BH_MEMORY_WC             (1 << 1)  // Write-combining
#define BH_MEMORY_WT             (1 << 2)  // Write-through
#define BH_MEMORY_WB             (1 << 3)  // Write-back
#define BH_MEMORY_UCE            (1 << 4)  // Uncacheable with eviction
#define BH_MEMORY_WP             (1 << 5)  // Write-protected
#define BH_MEMORY_RP             (1 << 6)  // Read-protected
#define BH_MEMORY_XP             (1 << 7)  // Execute-protected

// Memory map entry
typedef struct {
    bh_addr_t base_address;
    bh_size_t length;
    uint32_t type;
    uint32_t attributes;
} bh_memory_descriptor_t;

// Graphics information
typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
    uint32_t bpp;  // Bits per pixel
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
} bh_graphics_info_t;

// Boot information passed to the kernel
typedef struct {
    // Memory information
    bh_memory_descriptor_t* memory_map;
    bh_size_t memory_map_size;
    bh_size_t memory_map_descriptor_size;
    
    // Graphics information
    bh_graphics_info_t graphics_info;
    
    // Boot device information
    void* boot_device_handle;
    
    // Command line
    const char* command_line;
    
    // RSDP (for ACPI)
    void* rsdp;
} bh_boot_info_t;

// Function pointer types
typedef void (*bh_putc_t)(char c);
typedef void (*bh_puts_t)(const char* str);
typedef void (*bh_printf_t)(const char* format, ...);

typedef struct {
    // Memory management
    void* (*alloc)(bh_size_t size);
    void (*free)(void* ptr);
    
    // Console output
    bh_putc_t putc;
    bh_puts_t puts;
    bh_printf_t printf;
    
    // Memory map
    bh_status_t (*get_memory_map)(bh_memory_descriptor_t** map, 
                                 bh_size_t* map_size,
                                 bh_size_t* descriptor_size);
    
    // Graphics
    bh_status_t (*get_graphics_info)(bh_graphics_info_t* info);
    
    // ACPI
    void* (*get_rsdp)(void);
    
    // Boot device
    void* (*get_boot_device)(void);
    
    // Power management
    void (*reboot)(void);
    void (*shutdown)(void);
    
    // Debugging
    void (*debug_break)(void);
    
} bh_system_table_t;

// Global system table (set by the bootloader)
extern bh_system_table_t* bh_system_table;

// Helper macros
#define BH_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            bh_system_table->puts("Assertion failed: " #expr "\n"); \
            bh_system_table->debug_break(); \
        } \
    } while (0)

// Kernel entry point (implemented by the kernel)
void kmain(bh_boot_info_t* boot_info);

#endif // BLOODHORN_H
