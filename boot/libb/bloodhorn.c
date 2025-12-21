/*
 * bloodhorn.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <bloodhorn/bloodhorn.h>
#include <bloodhorn/debug.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

// Global system table (initialized by the bootloader)
bh_system_table_t* bh_system_table = NULL;

// Library state
static bool bh_initialized = false;
static bh_uint32_t bh_init_count = 0;

// Default implementations (can be overridden by the bootloader)
static void default_putc(char c) {
    // Default implementation - output to serial port if available
    // This is a basic implementation for debugging
    __asm__ volatile ("outb %0, $0x3F8" : : "a"(c));
}

static void default_puts(const char* str) {
    if (!str) return;
    
    while (*str) {
        default_putc(*str++);
    }
    default_putc('\n');
}

// Helper function for number to string conversion
static int bh_itoa(long long value, char* str, int base, bool is_signed) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    long long tmp_value;
    int count = 0;
    
    // Handle special case for 0
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return 1;
    }
    
    // Handle negative numbers for signed integers
    if (is_signed && value < 0) {
        *ptr++ = '-';
        value = -value;
        count++;
    }
    
    // Convert to string (reversed)
    tmp_value = value;
    while (tmp_value) {
        int digit = tmp_value % base;
        *ptr++ = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        tmp_value /= base;
        count++;
    }
    *ptr = '\0';
    
    // Reverse the string (skip the minus sign if present)
    ptr1 = (is_signed && str[0] == '-') ? str + 1 : str;
    ptr--;
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return count;
}

// Enhanced printf implementation
static void default_printf(const char* format, ...) {
    if (!format) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[64];
    const char* ptr = format;
    
    while (*ptr) {
        if (*ptr == '%' && *(ptr + 1)) {
            ptr++; // Skip '%'
            
            // Handle format specifiers
            switch (*ptr) {
                case 'd':
                case 'i': {
                    int value = va_arg(args, int);
                    bh_itoa(value, buffer, 10, true);
                    default_puts(buffer);
                    break;
                }
                case 'u': {
                    unsigned int value = va_arg(args, unsigned int);
                    bh_itoa(value, buffer, 10, false);
                    default_puts(buffer);
                    break;
                }
                case 'x': {
                    unsigned int value = va_arg(args, unsigned int);
                    bh_itoa(value, buffer, 16, false);
                    default_puts(buffer);
                    break;
                }
                case 'X': {
                    unsigned int value = va_arg(args, unsigned int);
                    int len = bh_itoa(value, buffer, 16, false);
                    // Convert to uppercase
                    for (int i = 0; i < len; i++) {
                        if (buffer[i] >= 'a' && buffer[i] <= 'f') {
                            buffer[i] = buffer[i] - 'a' + 'A';
                        }
                    }
                    default_puts(buffer);
                    break;
                }
                case 'p': {
                    void* value = va_arg(args, void*);
                    default_puts("0x");
                    bh_itoa((uintptr_t)value, buffer, 16, false);
                    default_puts(buffer);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (str) {
                        while (*str) {
                            default_putc(*str++);
                        }
                    } else {
                        default_puts("(null)");
                    }
                    break;
                }
                case 'c': {
                    int ch = va_arg(args, int);
                    default_putc((char)ch);
                    break;
                }
                case '%':
                    default_putc('%');
                    break;
                default:
                    default_putc('%');
                    default_putc(*ptr);
                    break;
            }
        } else {
            default_putc(*ptr);
        }
        ptr++;
    }
    
    va_end(args);
}

static void* default_alloc(bh_size_t size) {
    // Default implementation returns NULL
    (void)size;
    return NULL;
}

static void default_free(void* ptr) {
    // Default implementation does nothing
    (void)ptr;
}

static bh_status_t default_get_memory_map(bh_memory_descriptor_t** map,
                                        bh_size_t* map_size,
                                        bh_size_t* descriptor_size) {
    // Default implementation returns error
    (void)map;
    (void)map_size;
    (void)descriptor_size;
    return BH_ERROR;
}

static bh_status_t default_get_graphics_info(bh_graphics_info_t* info) {
    // Default implementation returns error
    (void)info;
    return BH_ERROR;
}

static void* default_get_rsdp(void) {
    // Default implementation returns NULL
    return NULL;
}

static void* default_get_boot_device(void) {
    // Default implementation returns NULL
    return NULL;
}

static void default_reboot(void) {
    // Default implementation does nothing
}

static void default_shutdown(void) {
    // Default implementation does nothing
}

static void default_debug_break(void) {
    // Default implementation does nothing
    __asm__ volatile ("int3");
}

// Default system table
static bh_system_table_t default_system_table = {
    .alloc = default_alloc,
    .free = default_free,
    .putc = default_putc,
    .puts = default_puts,
    .printf = default_printf,
    .get_memory_map = default_get_memory_map,
    .get_graphics_info = default_get_graphics_info,
    .get_rsdp = default_get_rsdp,
    .get_boot_device = default_get_boot_device,
    .reboot = default_reboot,
    .shutdown = default_shutdown,
    .debug_break = default_debug_break
};

// Initialize the system table (called by the bootloader)
void bh_init_system_table(bh_system_table_t* table) {
    if (table) {
        // Only set the system table if it's not already set
        if (!bh_system_table) {
            bh_system_table = table;
        }
    } else {
        // Use default implementations if no table is provided
        bh_system_table = &default_system_table;
    }
}

// Helper functions
void* bh_malloc(bh_size_t size) {
    if (bh_system_table && bh_system_table->alloc) {
        return bh_system_table->alloc(size);
    }
    return NULL;
}

void bh_free(void* ptr) {
    if (bh_system_table && bh_system_table->free) {
        bh_system_table->free(ptr);
    }
}

void bh_putc(char c) {
    if (bh_system_table && bh_system_table->putc) {
        bh_system_table->putc(c);
    }
}

void bh_puts(const char* str) {
    if (bh_system_table && bh_system_table->puts) {
        bh_system_table->puts(str);
    }
}

void bh_printf(const char* format, ...) {
    if (bh_system_table && bh_system_table->printf) {
        __builtin_va_list args;
        __builtin_va_start(args, format);
        // Implementation would use vsnprintf or similar
        __builtin_va_end(args);
    }
}

bh_status_t bh_get_memory_map(bh_memory_descriptor_t** map,
                             bh_size_t* map_size,
                             bh_size_t* descriptor_size) {
    if (bh_system_table && bh_system_table->get_memory_map) {
        return bh_system_table->get_memory_map(map, map_size, descriptor_size);
    }
    return BH_ERROR;
}

bh_status_t bh_get_graphics_info(bh_graphics_info_t* info) {
    if (bh_system_table && bh_system_table->get_graphics_info) {
        return bh_system_table->get_graphics_info(info);
    }
    return BH_ERROR;
}

void* bh_get_rsdp(void) {
    if (bh_system_table && bh_system_table->get_rsdp) {
        return bh_system_table->get_rsdp();
    }
    return NULL;
}

void* bh_get_boot_device(void) {
    if (bh_system_table && bh_system_table->get_boot_device) {
        return bh_system_table->get_boot_device();
    }
    return NULL;
}

void bh_reboot(void) {
    if (bh_system_table && bh_system_table->reboot) {
        bh_system_table->reboot();
    }
}

void bh_shutdown(void) {
    if (bh_system_table && bh_system_table->shutdown) {
        bh_system_table->shutdown();
    }
}

void bh_debug_break(void) {
    if (bh_system_table && bh_system_table->debug_break) {
        bh_system_table->debug_break();
    }
}

// Library initialization and management
bh_status_t bh_initialize(bh_system_table_t* system_table) {
    if (bh_initialized) {
        bh_init_count++;
        return BH_SUCCESS;
    }
    
    // Initialize the system table
    bh_init_system_table(system_table);
    
    if (!bh_system_table) {
        return BH_ERROR;
    }
    
    // Mark as initialized
    bh_initialized = true;
    bh_init_count = 1;
    
    // Initialize subsystems if available
    bh_status_t status = BH_SUCCESS;
    
    // Log successful initialization
    if (bh_system_table->puts) {
        bh_system_table->puts("BloodHorn library initialized successfully");
    }
    
    return status;
}

void bh_shutdown(void) {
    if (!bh_initialized) {
        return;
    }
    
    bh_init_count--;
    if (bh_init_count > 0) {
        return; // Still have references
    }
    
    // Log shutdown
    if (bh_system_table && bh_system_table->puts) {
        bh_system_table->puts("BloodHorn library shutting down");
    }
    
    // Shutdown subsystems
    if (bh_system_table && bh_system_table->puts) {
        bh_system_table->puts("Shutting down BloodHorn subsystems");
    }
    
    // Flush any pending debug output
    bh_debug_flush();
    
    // Clear any cached data
    bh_clear_cache();
    
    // Reset state
    bh_initialized = false;
    bh_init_count = 0;
    // Note: We don't reset bh_system_table as it might be needed by cleanup code
}

bool bh_is_initialized(void) {
    return bh_initialized;
}

void bh_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch) {
    if (major) *major = BLOODHORN_VERSION_MAJOR;
    if (minor) *minor = BLOODHORN_VERSION_MINOR;
    if (patch) *patch = BLOODHORN_VERSION_PATCH;
}

// Status code to string conversion
const char* bh_status_to_string(bh_status_t status) {
    switch (status) {
        case BH_SUCCESS:
            return "Success";
        case BH_PENDING:
            return "Operation pending";
        case BH_TIMEOUT:
            return "Operation timed out";
        case BH_ERROR:
            return "Generic error";
        case BH_INVALID_ARGUMENT:
            return "Invalid argument";
        case BH_INVALID_HANDLE:
            return "Invalid handle";
        case BH_INVALID_OPERATION:
            return "Invalid operation";
        case BH_NOT_IMPLEMENTED:
            return "Not implemented";
        case BH_NOT_SUPPORTED:
            return "Not supported";
        case BH_ACCESS_DENIED:
            return "Access denied";
        case BH_OUT_OF_MEMORY:
            return "Out of memory";
        case BH_OUT_OF_RESOURCES:
            return "Out of resources";
        case BH_DEVICE_ERROR:
            return "Device error";
        case BH_IO_ERROR:
            return "I/O error";
        case BH_TIMED_OUT:
            return "Timed out";
        case BH_WOULD_BLOCK:
            return "Would block";
        case BH_ALREADY_EXISTS:
            return "Already exists";
        case BH_NOT_FOUND:
            return "Not found";
        case BH_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case BH_PATH_NOT_FOUND:
            return "Path not found";
        case BH_PATH_ALREADY_EXISTS:
            return "Path already exists";
        case BH_DIRECTORY_NOT_EMPTY:
            return "Directory not empty";
        case BH_IS_A_DIRECTORY:
            return "Is a directory";
        case BH_NOT_A_DIRECTORY:
            return "Not a directory";
        case BH_READ_ONLY_FILESYSTEM:
            return "Read-only filesystem";
        case BH_FILESYSTEM_FULL:
            return "Filesystem full";
        case BH_NETWORK_ERROR:
            return "Network error";
        case BH_CONNECTION_REFUSED:
            return "Connection refused";
        case BH_CONNECTION_RESET:
            return "Connection reset";
        case BH_CONNECTION_ABORTED:
            return "Connection aborted";
        case BH_HOST_UNREACHABLE:
            return "Host unreachable";
        case BH_NETWORK_UNREACHABLE:
            return "Network unreachable";
        case BH_NETWORK_DOWN:
            return "Network down";
        case BH_PERMISSION_DENIED:
            return "Permission denied";
        case BH_AUTH_FAILED:
            return "Authentication failed";
        case BH_NOT_AUTHORIZED:
            return "Not authorized";
        case BH_CERTIFICATE_ERROR:
            return "Certificate error";
        case BH_HARDWARE_ERROR:
            return "Hardware error";
        case BH_DEVICE_NOT_FOUND:
            return "Device not found";
        case BH_DEVICE_BUSY:
            return "Device busy";
        case BH_DEVICE_OFFLINE:
            return "Device offline";
        case BH_INTERNAL_ERROR:
            return "Internal error";
        case BH_ASSERTION_FAILED:
            return "Assertion failed";
        case BH_UNEXPECTED_ERROR:
            return "Unexpected error";
        case BH_UNKNOWN_ERROR:
        default:
            return "Unknown error";
    }
}

// Utility functions for common operations
bh_status_t bh_memory_copy(void* dest, const void* src, bh_size_t size) {
    if (!dest || !src) {
        return BH_INVALID_ARGUMENT;
    }
    
    if (size == 0) {
        return BH_SUCCESS;
    }
    
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    // Handle overlapping memory regions
    if (d < s) {
        for (bh_size_t i = 0; i < size; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (bh_size_t i = size; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    // If d == s, no copy needed
    
    return BH_SUCCESS;
}

bh_status_t bh_memory_set(void* dest, int value, bh_size_t size) {
    if (!dest) {
        return BH_INVALID_ARGUMENT;
    }
    
    if (size == 0) {
        return BH_SUCCESS;
    }
    
    char* d = (char*)dest;
    for (bh_size_t i = 0; i < size; i++) {
        d[i] = (char)value;
    }
    
    return BH_SUCCESS;
}

bh_status_t bh_memory_compare(const void* ptr1, const void* ptr2, bh_size_t size, int* result) {
    if (!ptr1 || !ptr2 || !result) {
        return BH_INVALID_ARGUMENT;
    }
    
    const char* p1 = (const char*)ptr1;
    const char* p2 = (const char*)ptr2;
    
    for (bh_size_t i = 0; i < size; i++) {
        if (p1[i] < p2[i]) {
            *result = -1;
            return BH_SUCCESS;
        } else if (p1[i] > p2[i]) {
            *result = 1;
            return BH_SUCCESS;
        }
    }
    
    *result = 0;
    return BH_SUCCESS;
}

bh_size_t bh_string_length(const char* str) {
    if (!str) {
        return 0;
    }
    
    bh_size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    
    return len;
}

bh_status_t bh_string_copy(char* dest, const char* src, bh_size_t dest_size) {
    if (!dest || !src) {
        return BH_INVALID_ARGUMENT;
    }
    
    if (dest_size == 0) {
        return BH_BUFFER_TOO_SMALL;
    }
    
    bh_size_t i = 0;
    while (i < dest_size - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    
    return (src[i] == '\0') ? BH_SUCCESS : BH_BUFFER_TOO_SMALL;
}
