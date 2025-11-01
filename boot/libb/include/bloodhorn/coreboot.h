/**
 * @file coreboot.h
 * @brief Coreboot integration for BloodHorn (Header-only implementation)
 */

#ifndef BLOODHORN_COREBOOT_H
#define BLOODHORN_COREBOOT_H

#include <bloodhorn/types.h>
#include <bloodhorn/memory.h>
#include <bloodhorn/string.h>
#include <bloodhorn/printf.h>
#include <bloodhorn/arch/io.h>

// Coreboot table signatures
#define CB_HEADER_SIGNATURE  0x4F49424C  // 'LBIO'
#define CB_TABLE_SIGNATURE   0x4F49424C  // 'LBIO'

// Coreboot memory types
typedef enum {
    CB_MEM_RAM = 1,
    CB_MEM_RESERVED = 2,
    CB_MEM_ACPI = 3,
    CB_MEM_NVS = 4,
    CB_MEM_UNUSABLE = 5,
    CB_MEM_VENDOR_RSVD = 6,
    CB_MEM_TABLE = 16,
} bh_coreboot_memory_type_t;

// Coreboot table entry
typedef struct {
    bh_uint32_t tag;
    bh_uint32_t size;
} bh_coreboot_header_t;

// Coreboot memory range
typedef struct {
    bh_uint64_t start;
    bh_uint64_t size;
    bh_uint32_t type;
} bh_coreboot_memory_range_t;

// Coreboot memory map entry
typedef struct {
    bh_coreboot_header_t header;
    bh_uint64_t memory_map[];
} bh_coreboot_memory_map_t;

// Coreboot framebuffer information
typedef struct {
    bh_uint64_t physical_address;
    bh_uint32_t x_resolution;
    bh_uint32_t y_resolution;
    bh_uint32_t bytes_per_line;
    bh_uint8_t bits_per_pixel;
    bh_uint8_t red_mask_pos;
    bh_uint8_t red_mask_size;
    bh_uint8_t green_mask_pos;
    bh_uint8_t green_mask_size;
    bh_uint8_t blue_mask_pos;
    bh_uint8_t blue_mask_size;
    bh_uint8_t reserved_mask_pos;
    bh_uint8_t reserved_mask_size;
} bh_coreboot_framebuffer_t;

// Coreboot framebuffer entry
typedef struct {
    bh_coreboot_header_t header;
    bh_coreboot_framebuffer_t framebuffer;
} bh_coreboot_framebuffer_entry_t;

// Coreboot RSDP (Root System Description Pointer)
typedef struct {
    char signature[8];
    bh_uint8_t checksum;
    char oem_id[6];
    bh_uint8_t revision;
    bh_uint32_t rsdt_address;
    bh_uint32_t length;
    bh_uint64_t xsdt_address;
    bh_uint8_t extended_checksum;
    bh_uint8_t reserved[3];
} __attribute__((packed)) bh_coreboot_rsdp_t;

// Coreboot RSDP entry
typedef struct {
    bh_coreboot_header_t header;
    bh_coreboot_rsdp_t rsdp;
} bh_coreboot_rsdp_entry_t;

// Coreboot table
typedef struct {
    bh_uint32_t signature;
    bh_uint32_t header_bytes;
    bh_uint32_t header_checksum;
    bh_uint32_t table_bytes;
    bh_uint32_t table_checksum;
    bh_uint32_t table_entries;
} bh_coreboot_table_t;

// Coreboot tags
typedef enum {
    CB_TAG_NONE = 0,
    CB_TAG_MEMORY = 1,
    CB_TAG_HWRPB = 2,
    CB_TAG_MAINBOARD = 3,
    CB_TAG_VERSION = 4,
    CB_TAG_EXTRA_VERSION = 5,
    CB_TAG_BUILD = 6,
    CB_TAG_COMPILE_TIME = 7,
    CB_TAG_COMPILE_BY = 8,
    CB_TAG_COMPILE_HOST = 9,
    CB_TAG_COMPILE_DOMAIN = 10,
    CB_TAG_COMPILER = 11,
    CB_TAG_LINKER = 12,
    CB_TAG_ASSEMBLER = 13,
    CB_TAG_LOCATION = 14,
    CB_TAG_SERIAL = 17,
    CB_TAG_CONSOLE = 17,
    CB_TAG_FORWARD = 18,
    CB_TAG_FRAMEBUFFER = 19,
    CB_TAG_GPIO = 20,
    CB_TAG_TIMESTAMPS = 21,
    CB_TAG_CBMEM_CONSOLE = 22,
    CB_TAG_MRC_CACHE = 23,
    CB_TAG_ACPI_GNVS = 24,
    CB_TAG_BOARD_ID = 25,
    CB_TAG_BOOT_MEDIA_PARAMS = 26,
    CB_TAG_CB_MEMORY = 27,
    CB_TAG_VBNV = 28,
    CB_TAG_CMOS_OPTION_TABLE = 29,
    CB_TAG_OPTION = 30,
    CB_TAG_OPTION_ENUM = 31,
    CB_TAG_OPTION_DEFAULTS = 32,
    CB_TAG_OPTION_CHECKSUM = 33,
    CB_TAG_ACPI_RSDP = 0x21,
} bh_coreboot_tag_t;

// Static instance of coreboot table
static bh_coreboot_table_t* g_coreboot_table = NULL;

/**
 * @brief Calculate coreboot table checksum
 */
static inline bh_uint8_t bh_coreboot_checksum(const bh_uint8_t* data, bh_size_t len) {
    bh_uint8_t sum = 0;
    for (bh_size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * @brief Find coreboot table in memory
 */
static inline bh_status_t bh_coreboot_find_table(void) {
    // Coreboot typically places the table at 0x00000-0x1000 or in EBDA
    bh_uint16_t ebda_segment = *((bh_uint16_t*)0x40E) << 4;
    
    // Search in EBDA
    if (ebda_segment >= 0x8000 && ebda_segment < 0xA0000) {
        bh_uint8_t* ebda = (bh_uint8_t*)(bh_phys_addr_t)ebda_segment;
        for (bh_size_t i = 0; i < 0x400; i += 16) {
            if (*(bh_uint32_t*)(ebda + i) == CB_HEADER_SIGNATURE) {
                g_coreboot_table = (bh_coreboot_table_t*)(ebda + i);
                return BH_STATUS_SUCCESS;
            }
        }
    }
    
    // Search in 0x00000-0x1000
    for (bh_phys_addr_t addr = 0; addr < 0x1000; addr += 16) {
        if (*(bh_uint32_t*)addr == CB_HEADER_SIGNATURE) {
            g_coreboot_table = (bh_coreboot_table_t*)addr;
            return BH_STATUS_SUCCESS;
        }
    }
    
    return BH_STATUS_NOT_FOUND;
}

/**
 * @brief Find coreboot tag
 */
static inline void* bh_coreboot_find_tag_internal(bh_coreboot_tag_t tag) {
    if (!g_coreboot_table) {
        if (bh_coreboot_find_table() != BH_STATUS_SUCCESS) {
            return NULL;
        }
    }
    
    bh_uint8_t* ptr = (bh_uint8_t*)g_coreboot_table + g_coreboot_table->header_bytes;
    bh_uint8_t* end = (bh_uint8_t*)g_coreboot_table + g_coreboot_table->table_bytes;
    
    while (ptr < end) {
        bh_coreboot_header_t* header = (bh_coreboot_header_t*)ptr;
        
        if (header->tag == CB_TAG_NONE) {
            break;
        }
        
        if (header->tag == tag) {
            return ptr;
        }
        
        // Align to 4 bytes
        ptr += (header->size + 3) & ~3;
    }
    
    return NULL;
}

/**
 * @brief Get coreboot table
 */
static inline bh_status_t bh_coreboot_get_table(bh_coreboot_table_t** table) {
    if (!table) return BH_STATUS_INVALID_PARAMETER;
    
    if (!g_coreboot_table) {
        bh_status_t status = bh_coreboot_find_table();
        if (status != BH_STATUS_SUCCESS) {
            return status;
        }
        
        // Verify header checksum
        bh_uint8_t sum = bh_coreboot_checksum(
            (const bh_uint8_t*)g_coreboot_table,
            g_coreboot_table->header_bytes
        );
        
        if (sum != 0) {
            g_coreboot_table = NULL;
            return BH_STATUS_CHECKSUM_MISMATCH;
        }
    }
    
    *table = g_coreboot_table;
    return BH_STATUS_SUCCESS;
}

/**
 * @brief Get memory map from coreboot
 */
static inline bh_status_t bh_coreboot_get_memory_map(bh_coreboot_memory_range_t** ranges, bh_size_t* count) {
    if (!ranges || !count) return BH_STATUS_INVALID_PARAMETER;
    
    bh_coreboot_memory_map_t* mem_map = (bh_coreboot_memory_map_t*)bh_coreboot_find_tag_internal(CB_TAG_MEMORY);
    if (!mem_map) {
        return BH_STATUS_NOT_FOUND;
    }
    
    *ranges = (bh_coreboot_memory_range_t*)&mem_map->memory_map;
    *count = (mem_map->header.size - sizeof(bh_coreboot_header_t)) / sizeof(bh_coreboot_memory_range_t);
    
    return BH_STATUS_SUCCESS;
}

/**
 * @brief Find coreboot tag
 */
static inline bh_status_t bh_coreboot_find_tag(bh_coreboot_tag_t tag, void** data, bh_size_t* size) {
    if (!data) return BH_STATUS_INVALID_PARAMETER;
    
    bh_coreboot_header_t* header = (bh_coreboot_header_t*)bh_coreboot_find_tag_internal(tag);
    if (!header) {
        return BH_STATUS_NOT_FOUND;
    }
    
    *data = (void*)((bh_uint8_t*)header + sizeof(bh_coreboot_header_t));
    
    if (size) {
        *size = header->size - sizeof(bh_coreboot_header_t);
    }
    
    return BH_STATUS_SUCCESS;
}

/**
 * @brief Get framebuffer information
 */
static inline bh_status_t bh_coreboot_get_framebuffer(bh_coreboot_framebuffer_t** fb) {
    if (!fb) return BH_STATUS_INVALID_PARAMETER;
    
    bh_coreboot_framebuffer_entry_t* fb_entry = 
        (bh_coreboot_framebuffer_entry_t*)bh_coreboot_find_tag_internal(CB_TAG_FRAMEBUFFER);
    
    if (!fb_entry) {
        return BH_STATUS_NOT_FOUND;
    }
    
    *fb = &fb_entry->framebuffer;
    return BH_STATUS_SUCCESS;
}

/**
 * @brief Get ACPI RSDP
 */
static inline bh_status_t bh_coreboot_get_rsdp(bh_coreboot_rsdp_t** rsdp) {
    if (!rsdp) return BH_STATUS_INVALID_PARAMETER;
    
    bh_coreboot_rsdp_entry_t* rsdp_entry = 
        (bh_coreboot_rsdp_entry_t*)bh_coreboot_find_tag_internal(CB_TAG_ACPI_RSDP);
    
    if (!rsdp_entry) {
        // Try to find it in the 0xE0000-0xFFFFF area
        for (bh_phys_addr_t addr = 0xE0000; addr < 0x100000; addr += 16) {
            if (bh_memcmp((void*)addr, "RSD PTR ", 8) == 0) {
                // Verify checksum
                bh_uint8_t sum = 0;
                for (bh_size_t i = 0; i < 20; i++) {
                    sum += ((bh_uint8_t*)addr)[i];
                }
                
                if (sum == 0) {
                    *rsdp = (bh_coreboot_rsdp_t*)addr;
                    return BH_STATUS_SUCCESS;
                }
            }
        }
        return BH_STATUS_NOT_FOUND;
    }
    
    *rsdp = &rsdp_entry->rsdp;
    return BH_STATUS_SUCCESS;
}

/**
 * @brief Initialize coreboot integration
 */
static inline bh_status_t bh_coreboot_initialize(void) {
    // Find and verify coreboot table
    bh_coreboot_table_t* table;
    bh_status_t status = bh_coreboot_get_table(&table);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Get memory map
    bh_coreboot_memory_range_t* mem_ranges;
    bh_size_t mem_range_count;
    status = bh_coreboot_get_memory_map(&mem_ranges, &mem_range_count);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    // Initialize memory manager with coreboot memory map
    for (bh_size_t i = 0; i < mem_range_count; i++) {
        bh_memory_region_t region = {
            .base = mem_ranges[i].start,
            .size = mem_ranges[i].size,
            .type = (mem_ranges[i].type == CB_MEM_RAM) ? 
                BH_MEMORY_TYPE_USABLE : BH_MEMORY_TYPE_RESERVED
        };
        
        bh_memory_add_region(&region);
    }
    
    return BH_STATUS_SUCCESS;
}

#endif /* BLOODHORN_COREBOOT_H */
