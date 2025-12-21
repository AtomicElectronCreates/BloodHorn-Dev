/*
 * system.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_SYSTEM_H
#define BLOODHORN_SYSTEM_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>
#include <bloodhorn/memory.h>
#include <bloodhorn/graphics.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct bh_system_table bh_system_table_t;
typedef struct bh_boot_services bh_boot_services_t;
typedef struct bh_runtime_services bh_runtime_services_t;

// Function pointer types for system services
typedef void* (*bh_alloc_fn_t)(bh_size_t size);
typedef void (*bh_free_fn_t)(void* ptr);
typedef void (*bh_putc_fn_t)(char c);
typedef void (*bh_puts_fn_t)(const char* str);
typedef void (*bh_printf_fn_t)(const char* format, ...);

// System information
typedef struct {
    const char* firmware_vendor;
    bh_uint32_t firmware_revision;
    const char* system_vendor;
    const char* system_product;
    const char* system_version;
    const char* system_serial;
    bh_uuid_t system_uuid;
    const char* baseboard_vendor;
    const char* baseboard_product;
    const char* baseboard_version;
    const char* baseboard_serial;
    const char* processor_brand;
    bh_uint32_t processor_frequency;
    bh_uint32_t processor_cores;
    bh_uint32_t processor_threads;
    bh_size_t memory_size;
} bh_system_info_t;

// Boot services table
struct bh_boot_services {
    // Table header
    bh_uint32_t signature;
    bh_uint32_t revision;
    bh_uint32_t header_size;
    bh_uint32_t crc32;
    bh_uint32_t reserved;

    // Memory services
    bh_status_t (*allocate_pages)(
        bh_memory_type_t memory_type,
        bh_size_t pages,
        bh_phys_addr_t* memory
    );

    bh_status_t (*free_pages)(
        bh_phys_addr_t memory,
        bh_size_t pages
    );

    bh_status_t (*get_memory_map)(
        bh_size_t* memory_map_size,
        bh_memory_descriptor_t* memory_map,
        bh_size_t* map_key,
        bh_size_t* descriptor_size,
        bh_uint32_t* descriptor_version
    );

    bh_status_t (*allocate_pool)(
        bh_memory_type_t pool_type,
        bh_size_t size,
        void** buffer
    );

    bh_status_t (*free_pool)(void* buffer);

    // Event and timer services
    bh_status_t (*create_event)(
        bh_uint32_t type,
        bh_uint32_t notify_tpl,
        void (*notify_function)(void* event, void* context),
        void* notify_context,
        void** event
    );

    bh_status_t (*set_timer)(
        void* event,
        bh_uint32_t type,
        bh_uint64_t trigger_time
    );

    bh_status_t (*wait_for_event)(
        bh_size_t number_of_events,
        void** event,
        bh_size_t* index
    );

    bh_status_t (*signal_event)(void* event);
    bh_status_t (*close_event)(void* event);

    // Protocol handler services
    bh_status_t (*install_protocol_interface)(
        bh_handle_t* handle,
        bh_uuid_t* protocol,
        bh_uint32_t interface_type,
        void* interface
    );

    bh_status_t (*reinstall_protocol_interface)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        void* old_interface,
        void* new_interface
    );

    bh_status_t (*uninstall_protocol_interface)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        void* interface
    );

    bh_status_t (*handle_protocol)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        void** interface
    );

    void* reserved1;

    bh_status_t (*register_protocol_notify)(
        bh_uuid_t* protocol,
        void* event,
        void** registration
    );

    bh_status_t (*locate_handle)(
        bh_uint32_t search_type,
        bh_uuid_t* protocol,
        void* search_key,
        bh_size_t* buffer_size,
        bh_handle_t* buffer
    );

    bh_status_t (*locate_device_path)(
        bh_uuid_t* protocol,
        void** device_path,
        bh_handle_t* device
    );

    bh_status_t (*install_configuration_table)(
        bh_uuid_t* uuid,
        void* table
    );

    // Image services
    bh_status_t (*load_image)(
        bh_bool_t boot_policy,
        bh_handle_t parent_image_handle,
        void* device_path,
        void* source_buffer,
        bh_size_t source_size,
        bh_handle_t* image_handle
    );

    bh_status_t (*start_image)(
        bh_handle_t image_handle,
        bh_size_t* exit_data_size,
        bh_uint16_t** exit_data
    );

    bh_status_t (*exit)(
        bh_handle_t image_handle,
        bh_status_t exit_status,
        bh_size_t exit_data_size,
        bh_uint16_t* exit_data
    );

    bh_status_t (*unload_image)(bh_handle_t image_handle);

    bh_status_t (*exit_boot_services)(
        bh_handle_t image_handle,
        bh_size_t map_key
    );

    // Miscellaneous services
    bh_status_t (*get_next_monotonic_count)(bh_uint64_t* count);
    bh_status_t (*stall)(bh_size_t microseconds);
    bh_status_t (*set_watchdog_timer)(
        bh_size_t timeout,
        bh_uint64_t watchdog_code,
        bh_size_t data_size,
        bh_uint16_t* watchdog_data
    );

    // Driver support services
    bh_status_t (*connect_controller)(
        bh_handle_t controller_handle,
        bh_handle_t* driver_image_handle,
        void* remaining_device_path,
        bh_bool_t recursive
    );

    bh_status_t (*disconnect_controller)(
        bh_handle_t controller_handle,
        bh_handle_t driver_image_handle,
        bh_handle_t child_handle
    );

    // Open and close protocol services
    bh_status_t (*open_protocol)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        void** interface,
        bh_handle_t agent_handle,
        bh_handle_t controller_handle,
        bh_uint32_t attributes
    );

    bh_status_t (*close_protocol)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        bh_handle_t agent_handle,
        bh_handle_t controller_handle
    );

    bh_status_t (*open_protocol_information)(
        bh_handle_t handle,
        bh_uuid_t* protocol,
        void** entry_buffer,
        bh_size_t* entry_count
    );

    // Library services
    bh_status_t (*protocols_per_handle)(
        bh_handle_t handle,
        bh_uuid_t*** protocol_buffer,
        bh_size_t* protocol_buffer_count
    );

    bh_status_t (*locate_handle_buffer)(
        bh_uint32_t search_type,
        bh_uuid_t* protocol,
        void* search_key,
        bh_size_t* no_handles,
        bh_handle_t** buffer
    );

    bh_status_t (*locate_protocol)(
        bh_uuid_t* protocol,
        void* registration,
        void** interface
    );

    bh_status_t (*install_multiple_protocol_interfaces)(
        bh_handle_t* handle,
        ...
    );

    bh_status_t (*uninstall_multiple_protocol_interfaces)(
        bh_handle_t handle,
        ...
    );

    // 32-bit CRC services
    bh_status_t (*calculate_crc32)(
        void* data,
        bh_size_t data_size,
        bh_uint32_t* crc32
    );

    // Miscellaneous services
    void (*copy_mem)(void* destination, void* source, bh_size_t length);
    void (*set_mem)(void* buffer, bh_size_t size, bh_uint8_t value);
};

// Runtime services table
struct bh_runtime_services {
    // Table header
    bh_uint32_t signature;
    bh_uint32_t revision;
    bh_uint32_t header_size;
    bh_uint32_t crc32;
    bh_uint32_t reserved;

    // Time services
    bh_status_t (*get_time)(
        void* time,
        void* capabilities
    );

    bh_status_t (*set_time)(void* time);

    bh_status_t (*get_wakeup_time)(
        bh_bool_t* enabled,
        bh_bool_t* pending,
        void* time
    );

    bh_status_t (*set_wakeup_time)(
        bh_bool_t enable,
        void* time
    );

    // Virtual memory services
    bh_status_t (*set_virtual_address_map)(
        bh_size_t memory_map_size,
        bh_size_t descriptor_size,
        bh_uint32_t descriptor_version,
        bh_memory_descriptor_t* virtual_map
    );

    bh_status_t (*convert_pointer)(
        bh_size_t debug_disposition,
        void** address
    );

    // Variable services
    bh_status_t (*get_variable)(
        bh_uint16_t* variable_name,
        bh_uuid_t* vendor_uuid,
        bh_uint32_t* attributes,
        bh_size_t* data_size,
        void* data
    );

    bh_status_t (*get_next_variable_name)(
        bh_size_t* variable_name_size,
        bh_uint16_t* variable_name,
        bh_uuid_t* vendor_uuid
    );

    bh_status_t (*set_variable)(
        bh_uint16_t* variable_name,
        bh_uuid_t* vendor_uuid,
        bh_uint32_t attributes,
        bh_size_t data_size,
        void* data
    );

    // Miscellaneous services
    bh_status_t (*get_next_high_mono_count)(bh_uint32_t* high_count);
    void (*reset_system)(bh_uint32_t reset_type, bh_status_t reset_status,
                         bh_size_t data_size, void* reset_data);

    // Capsule services
    bh_status_t (*update_capsule)(
        void** capsule_header_array,
        bh_size_t capsule_count,
        bh_phys_addr_t scatter_gather_list
    );

    bh_status_t (*query_capsule_capabilities)(
        void** capsule_header_array,
        bh_size_t capsule_count,
        bh_uint64_t* maximum_capsule_size,
        bh_uint32_t* reset_type
    );

    // Query variable info
    bh_status_t (*query_variable_info)(
        bh_uint32_t attributes,
        bh_uint64_t* maximum_variable_storage_size,
        bh_uint64_t* remaining_variable_storage_size,
        bh_uint64_t* maximum_variable_size
    );
};

// Configuration table entry
typedef struct {
    bh_uuid_t vendor_uuid;
    void* vendor_table;
} bh_configuration_table_t;

// Main system table
struct bh_system_table {
    // Table header
    bh_uint32_t signature;
    bh_uint32_t revision;
    bh_uint32_t header_size;
    bh_uint32_t crc32;
    bh_uint32_t reserved;

    // System information
    bh_uint16_t* firmware_vendor;
    bh_uint32_t firmware_revision;
    bh_handle_t console_in_handle;
    void* con_in;
    bh_handle_t console_out_handle;
    void* con_out;
    bh_handle_t standard_error_handle;
    void* std_err;
    
    // Runtime services
    bh_runtime_services_t* runtime_services;
    
    // Boot services
    bh_boot_services_t* boot_services;
    
    // Configuration tables
    bh_size_t number_of_table_entries;
    bh_configuration_table_t* configuration_table;
    
    // BloodHorn-specific extensions
    struct {
        // Memory management
        bh_alloc_fn_t alloc;
        bh_free_fn_t free;
        
        // Console output
        bh_putc_fn_t putc;
        bh_puts_fn_t puts;
        bh_printf_fn_t printf;
        
        // Memory map
        bh_status_t (*get_memory_map)(
            bh_memory_descriptor_t** map,
            bh_size_t* map_size,
            bh_size_t* descriptor_size
        );
        
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
        
        // System information
        bh_status_t (*get_system_info)(bh_system_info_t* info);
        
        // Time
        bh_uint64_t (*get_timestamp)(void);
        bh_uint64_t (*get_frequency)(void);
        
        // CPU information
        bh_status_t (*get_cpu_info)(
            bh_uint32_t* vendor_id,
            bh_uint32_t* cpu_family,
            bh_uint32_t* model,
            bh_uint32_t* stepping,
            bh_uint64_t* features
        );
        
        // Platform-specific services
        void* platform_data;
        bh_status_t (*platform_init)(void* data);
        bh_status_t (*platform_cleanup)(void* data);
        
    } bloodhorn_services;
};

// Global system table pointer
extern bh_system_table_t* bh_system_table;

// System table signatures
#define BH_SYSTEM_TABLE_SIGNATURE    0x5453594257424853ULL  // "BHWBYSTS"
#define BH_BOOT_SERVICES_SIGNATURE   0x56524553544F4F42ULL  // "BOOTSERV" 
#define BH_RUNTIME_SERVICES_SIGNATURE 0x56524553454D4952ULL // "RIMESERV"

// System table revisions
#define BH_SYSTEM_TABLE_REVISION     0x00010000
#define BH_BOOT_SERVICES_REVISION    0x00010000
#define BH_RUNTIME_SERVICES_REVISION 0x00010000

// BloodHorn system initialization
bh_status_t bh_system_initialize(bh_system_table_t* system_table);
void bh_system_shutdown(void);

// Helper functions
bh_system_info_t* bh_get_system_info(void);
bh_boot_services_t* bh_get_boot_services(void);
bh_runtime_services_t* bh_get_runtime_services(void);

// Convenience macros
#define BH_BOOT_SERVICES() (bh_system_table->boot_services)
#define BH_RUNTIME_SERVICES() (bh_system_table->runtime_services)
#define BH_BLOODHORN_SERVICES() (&bh_system_table->bloodhorn_services)

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_SYSTEM_H
