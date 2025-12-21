/*
 * bloodchain.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

void bcbp_init(struct bcbp_header *hdr, uint64_t entry_point, uint64_t boot_device);

/**
 * Add a module to the BCBP structure
 * 
 * @param hdr      Pointer to the BCBP header
 * @param start    Physical start address of the module
 * @param size     Size of the module in bytes
 * @param name     Name of the module (will be copied)
 * @param type     Module type (BCBP_MODTYPE_*)
 * @param cmdline  Command line string for the module (optional, can be NULL)
 */
void bcbp_add_module(struct bcbp_header *hdr, uint64_t start, uint64_t size,
                    const char *name, uint8_t type, const char *cmdline);

/**
 * Find a module by name
 * 
 * @param hdr   Pointer to the BCBP header
 * @param name  Name of the module to find
 * @return      Pointer to the module, or NULL if not found
 */
struct bcbp_module *bcbp_find_module(struct bcbp_header *hdr, const char *name);

/**
 * Validate the BCBP structure
 * 
 * @param hdr  Pointer to the BCBP header
 * @return     0 if valid, negative error code otherwise
 */
int bcbp_validate(const struct bcbp_header *hdr);

/**
 * Set up ACPI RSDP pointer
 * 
 * @param hdr    Pointer to the BCBP header
 * @param rsdp   Physical address of the ACPI RSDP
 */
void bcbp_set_acpi_rsdp(struct bcbp_header *hdr, uint64_t rsdp);

/**
 * Set up SMBIOS entry point
 * 
 * @param hdr      Pointer to the BCBP header
 * @param smbios   Physical address of the SMBIOS entry point
 */
void bcbp_set_smbios(struct bcbp_header *hdr, uint64_t smbios);

/**
 * Set up framebuffer information
 * 
 * @param hdr          Pointer to the BCBP header
 * @param framebuffer  Physical address of the framebuffer
 */
void bcbp_set_framebuffer(struct bcbp_header *hdr, uint64_t framebuffer);

#ifdef __cplusplus
}
#endif

// Kernel Interface (for kernel implementation)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the boot information structure
 * 
 * @return  Pointer to the BCBP header (passed in RDI by bootloader)
 */
static inline struct bcbp_header *bcbp_get_boot_info(void) {
    struct bcbp_header *hdr;
    asm volatile ("mov %0, rdi" : "=r" (hdr));
    return hdr;
}

/**
 * Get module by index
 * 
 * @param hdr  Pointer to the BCBP header
 * @param idx  Module index (0-based)
 * @return     Pointer to the module, or NULL if out of bounds
 */
static inline struct bcbp_module *bcbp_get_module(struct bcbp_header *hdr, uint64_t idx) {
    if (idx >= hdr->module_count) return NULL;
    return ((struct bcbp_module *)hdr->modules) + idx;
}

#ifdef __cplusplus
}
#endif

// Constants for bootloader use
#define BCBP_MAGIC     0x424C4348  // "BLCH"
#define BCBP_VERSION   0x00010000  // 1.0
#define BCBP_HEADER_SIZE  sizeof(struct bcbp_header)
#define BCBP_MODULE_SIZE  sizeof(struct bcbp_module)

#endif // BLOODCHAIN_H
