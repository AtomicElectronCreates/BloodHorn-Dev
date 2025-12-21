/*
 * memory.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_MEMORY_H
#define BLOODHORN_MEMORY_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory allocation flags
typedef enum {
    BH_MEMORY_NONE = 0,         ///< No special attributes
    BH_MEMORY_ZERO = 1 << 0,    ///< Zero-initialize memory
    BH_MEMORY_EXECUTABLE = 1 << 1, ///< Executable memory
    BH_MEMORY_WRITECOMBINE = 1 << 2, ///< Write-combining memory
    BH_MEMORY_UNCACHED = 1 << 3, ///< Uncached memory
    BH_MEMORY_DMA = 1 << 4,     ///< Memory suitable for DMA
    BH_MEMORY_ACPI = 1 << 5,    ///< ACPI memory
    BH_MEMORY_NVS = 1 << 6,     ///< Non-volatile memory
    BH_MEMORY_PERSISTENT = 1 << 7, ///< Persistent memory
    BH_MEMORY_RUNTIME = 1 << 8, ///< Runtime memory
    BH_MEMORY_MMIO = 1 << 9,    ///< Memory-mapped I/O
    BH_MEMORY_IO = 1 << 10,     ///< I/O port space
    BH_MEMORY_ISA = 1 << 11,    ///< ISA memory
    BH_MEMORY_PCI = 1 << 12,    ///< PCI memory
    BH_MEMORY_PCI_IO = 1 << 13, ///< PCI I/O space
    BH_MEMORY_PCI_BUS = 1 << 14, ///< PCI bus memory
    BH_MEMORY_PCI_BRIDGE = 1 << 15, ///< PCI bridge memory
    BH_MEMORY_PCI_PREFETCH = 1 << 16, ///< Prefetchable memory
    BH_MEMORY_PCI_WRITE_COMBINE = 1 << 17, ///< Write-combining memory
    BH_MEMORY_PCI_WRITE_THROUGH = 1 << 18, ///< Write-through memory
    BH_MEMORY_PCI_WRITE_PROTECT = 1 << 19, ///< Write-protected memory
    BH_MEMORY_PCI_READ_PROTECT = 1 << 20, ///< Read-protected memory
    BH_MEMORY_PCI_EXECUTE_PROTECT = 1 << 21, ///< Execute-protected memory
    BH_MEMORY_PCI_CACHEABLE = 1 << 22, ///< Cacheable memory
    BH_MEMORY_PCI_WRITE_BACK = 1 << 23, ///< Write-back memory
    BH_MEMORY_PCI_WRITE_ALLOCATE = 1 << 24, ///< Write-allocate memory
    BH_MEMORY_PCI_READ_ALLOCATE = 1 << 25, ///< Read-allocate memory
    BH_MEMORY_PCI_PREFETCHABLE = 1 << 26, ///< Prefetchable memory
    BH_MEMORY_PCI_WRITE_COMBINING = 1 << 27, ///< Write-combining memory
    BH_MEMORY_PCI_WRITE_THROUGH_CACHE = 1 << 28, ///< Write-through cacheable memory
    BH_MEMORY_PCI_WRITE_BACK_CACHE = 1 << 29, ///< Write-back cacheable memory
    BH_MEMORY_PCI_CACHE_DISABLED = 1 << 30, ///< Cache disabled
    BH_MEMORY_PCI_CACHE_ENABLED = 1U << 31 ///< Cache enabled
} bh_memory_flags_t;

// Memory protection flags
typedef enum {
    BH_PROT_NONE = 0,   ///< No access
    BH_PROT_READ = 1,   ///< Read access
    BH_PROT_WRITE = 2,  ///< Write access
    BH_PROT_EXEC = 4,   ///< Execute access
    BH_PROT_RW = BH_PROT_READ | BH_PROT_WRITE,  ///< Read-write access
    BH_PROT_RX = BH_PROT_READ | BH_PROT_EXEC,   ///< Read-execute access
    BH_PROT_RWX = BH_PROT_READ | BH_PROT_WRITE | BH_PROT_EXEC  ///< Read-write-execute access
} bh_protection_t;

// Memory allocation types
typedef enum {
    BH_MEMORY_TYPE_CONVENTIONAL,  ///< Conventional memory
    BH_MEMORY_TYPE_RESERVED,     ///< Reserved memory
    BH_MEMORY_TYPE_ACPI_RECLAIM, ///< ACPI reclaimable memory
    BH_MEMORY_TYPE_ACPI_NVS,     ///< ACPI NVS memory
    BH_MEMORY_TYPE_UNUSABLE,     ///< Unusable memory
    BH_MEMORY_TYPE_DISABLED,     ///< Disabled memory
    BH_MEMORY_TYPE_PERSISTENT,   ///< Persistent memory
    BH_MEMORY_TYPE_UNACCEPTED,   ///< Unaccepted memory
    BH_MEMORY_TYPE_MAX           ///< Maximum memory type
} bh_memory_type_t;

// Memory descriptor
typedef struct {
    bh_phys_addr_t physical_start;  ///< Physical start address
    bh_virt_addr_t virtual_start;   ///< Virtual start address
    bh_size_t num_pages;           ///< Number of pages
    bh_memory_type_t type;         ///< Memory type
    bh_memory_flags_t flags;       ///< Memory flags
} bh_memory_descriptor_t;

// Memory map
typedef struct {
    bh_memory_descriptor_t* descriptors;  ///< Array of memory descriptors
    bh_size_t count;                     ///< Number of descriptors
    bh_size_t descriptor_size;           ///< Size of each descriptor
    uint32_t descriptor_version;         ///< Descriptor version
} bh_memory_map_t;

/**
 * @brief Allocate memory
 * 
 * @param size Size of memory to allocate in bytes
 * @param alignment Memory alignment (must be power of 2)
 * @param flags Memory allocation flags
 * @return void* Pointer to allocated memory, or NULL on failure
 */
void* bh_memory_allocate(bh_size_t size, bh_size_t alignment, bh_memory_flags_t flags);

/**
 * @brief Free memory
 * 
 * @param ptr Pointer to memory to free
 * @param size Size of memory to free
 */
void bh_memory_free(void* ptr, bh_size_t size);

/**
 * @brief Get the memory map
 * 
 * @param map Pointer to memory map structure to fill
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_get_map(bh_memory_map_t* map);

/**
 * @brief Set memory protection
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @param prot Protection flags
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_protect(void* addr, bh_size_t size, bh_protection_t prot);

/**
 * @brief Query memory protection
 * 
 * @param addr Address to query
 * @param prot [out] Protection flags
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_query_protection(const void* addr, bh_protection_t* prot);

/**
 * @brief Lock memory pages in physical memory
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_lock(void* addr, bh_size_t size);

/**
 * @brief Unlock memory pages
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_unlock(void* addr, bh_size_t size);

/**
 * @brief Flush memory region from cache
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_flush(void* addr, bh_size_t size);

/**
 * @brief Invalidate memory region in cache
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_invalidate(void* addr, bh_size_t size);

/**
 * @brief Allocate physically contiguous memory
 * 
 * @param size Size of memory to allocate
 * @param alignment Memory alignment
 * @param flags Memory allocation flags
 * @param physical_addr [out] Physical address of allocated memory
 * @return void* Virtual address of allocated memory, or NULL on failure
 */
void* bh_memory_allocate_contiguous(
    bh_size_t size,
    bh_size_t alignment,
    bh_memory_flags_t flags,
    bh_phys_addr_t* physical_addr
);

/**
 * @brief Free physically contiguous memory
 * 
 * @param addr Virtual address of memory to free
 * @param size Size of memory to free
 */
void bh_memory_free_contiguous(void* addr, bh_size_t size);

/**
 * @brief Get physical address from virtual address
 * 
 * @param virt Virtual address
 * @param phys [out] Physical address
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_virt_to_phys(const void* virt, bh_phys_addr_t* phys);

/**
 * @brief Get virtual address from physical address
 * 
 * @param phys Physical address
 * @param virt [out] Virtual address
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_phys_to_virt(bh_phys_addr_t phys, void** virt);

/**
 * @brief Map physical memory to virtual address space
 * 
 * @param phys Physical address
 * @param size Size of memory region
 * @param prot Protection flags
 * @param flags Memory mapping flags
 * @return void* Virtual address of mapped memory, or NULL on failure
 */
void* bh_memory_map_physical(
    bh_phys_addr_t phys,
    bh_size_t size,
    bh_protection_t prot,
    bh_memory_flags_t flags
);

/**
 * @brief Unmap memory
 * 
 * @param addr Virtual address to unmap
 * @param size Size of memory region
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_unmap(void* addr, bh_size_t size);

/**
 * @brief Get memory statistics
 * 
 * @param total [out] Total physical memory in bytes
 * @param free [out] Free physical memory in bytes
 * @param used [out] Used physical memory in bytes
 * @param reserved [out] Reserved physical memory in bytes
 * @return bh_status_t Status code
 */
bh_status_t bh_memory_get_stats(
    bh_size_t* total,
    bh_size_t* free,
    bh_size_t* used,
    bh_size_t* reserved
);

/**
 * @brief Check if memory is readable
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return true if memory is readable
 * @return false if memory is not readable
 */
bool bh_memory_is_readable(const void* addr, bh_size_t size);

/**
 * @brief Check if memory is writable
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return true if memory is writable
 * @return false if memory is not writable
 */
bool bh_memory_is_writable(const void* addr, bh_size_t size);

/**
 * @brief Check if memory is executable
 * 
 * @param addr Starting address
 * @param size Size of memory region
 * @return true if memory is executable
 * @return false if memory is not executable
 */
bool bh_memory_is_executable(const void* addr, bh_size_t size);

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_MEMORY_H
