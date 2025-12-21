/*
 * debug.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_DEBUG_H
#define BLOODHORN_DEBUG_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// Debug levels
typedef enum {
    BH_DEBUG_LEVEL_NONE = 0,     // No debug output
    BH_DEBUG_LEVEL_ERROR,        // Error messages only
    BH_DEBUG_LEVEL_WARNING,      // Warnings and errors
    BH_DEBUG_LEVEL_INFO,         // Informational messages
    BH_DEBUG_LEVEL_VERBOSE,      // Verbose output
    BH_DEBUG_LEVEL_TRACE,        // Trace execution
    BH_DEBUG_LEVEL_ALL = 255     // All debug output
} bh_debug_level_t;

// Debug flags
typedef enum {
    BH_DEBUG_FLAG_NONE          = 0x0000,
    BH_DEBUG_FLAG_TIMESTAMP     = 0x0001,    // Include timestamp
    BH_DEBUG_FLAG_LEVEL         = 0x0002,    // Include level name
    BH_DEBUG_FLAG_FILE          = 0x0004,    // Include filename
    BH_DEBUG_FLAG_LINE          = 0x0008,    // Include line number
    BH_DEBUG_FLAG_FUNCTION      = 0x0010,    // Include function name
    BH_DEBUG_FLAG_THREAD        = 0x0020,    // Include thread ID
    BH_DEBUG_FLAG_COLOR         = 0x0040,    // Use color codes
    BH_DEBUG_FLAG_FLUSH         = 0x0080,    // Flush after each message
    BH_DEBUG_FLAG_BUFFER        = 0x0100,    // Buffer output
    BH_DEBUG_FLAG_SERIAL        = 0x0200,    // Output to serial port
    BH_DEBUG_FLAG_CONSOLE       = 0x0400,    // Output to console
    BH_DEBUG_FLAG_FILE_OUTPUT   = 0x0800,    // Output to file
    BH_DEBUG_FLAG_MEMORY        = 0x1000,    // Keep in memory buffer
    BH_DEBUG_FLAG_DEFAULT       = (BH_DEBUG_FLAG_TIMESTAMP | 
                                   BH_DEBUG_FLAG_LEVEL | 
                                   BH_DEBUG_FLAG_CONSOLE)
} bh_debug_flags_t;

// Debug categories (for filtering)
typedef enum {
    BH_DEBUG_CAT_GENERAL        = 0x00000001,
    BH_DEBUG_CAT_MEMORY         = 0x00000002,
    BH_DEBUG_CAT_GRAPHICS       = 0x00000004,
    BH_DEBUG_CAT_INPUT          = 0x00000008,
    BH_DEBUG_CAT_FILESYSTEM     = 0x00000010,
    BH_DEBUG_CAT_TIME           = 0x00000020,
    BH_DEBUG_CAT_SYSTEM         = 0x00000040,
    BH_DEBUG_CAT_BOOT           = 0x00000080,
    BH_DEBUG_CAT_DEVICE         = 0x00000100,
    BH_DEBUG_CAT_NETWORK        = 0x00000200,
    BH_DEBUG_CAT_SECURITY       = 0x00000400,
    BH_DEBUG_CAT_PERFORMANCE    = 0x00000800,
    BH_DEBUG_CAT_USER1          = 0x10000000,
    BH_DEBUG_CAT_USER2          = 0x20000000,
    BH_DEBUG_CAT_USER3          = 0x40000000,
    BH_DEBUG_CAT_USER4          = 0x80000000,
    BH_DEBUG_CAT_ALL            = 0xFFFFFFFF
} bh_debug_category_t;

// Debug output destinations
typedef enum {
    BH_DEBUG_OUTPUT_NONE = 0,
    BH_DEBUG_OUTPUT_CONSOLE,
    BH_DEBUG_OUTPUT_SERIAL,
    BH_DEBUG_OUTPUT_FILE,
    BH_DEBUG_OUTPUT_MEMORY,
    BH_DEBUG_OUTPUT_CALLBACK
} bh_debug_output_t;

// Debug callback function
typedef void (*bh_debug_callback_t)(
    bh_debug_level_t level,
    bh_debug_category_t category,
    const char* file,
    bh_uint32_t line,
    const char* function,
    const char* message
);

// Debug configuration
typedef struct {
    bh_debug_level_t min_level;
    bh_debug_category_t category_mask;
    bh_debug_flags_t flags;
    bh_debug_output_t output;
    const char* output_file;
    bh_debug_callback_t callback;
    bh_size_t memory_buffer_size;
    bh_bool_t break_on_error;
    bh_bool_t break_on_assert;
} bh_debug_config_t;

// Stack trace entry
typedef struct {
    bh_uintptr_t address;
    const char* function;
    const char* file;
    bh_uint32_t line;
    bh_uint32_t offset;
} bh_stack_frame_t;

// Stack trace
typedef struct {
    bh_stack_frame_t* frames;
    bh_size_t frame_count;
    bh_size_t max_frames;
} bh_stack_trace_t;

// Performance counter
typedef struct {
    const char* name;
    bh_uint64_t start_time;
    bh_uint64_t total_time;
    bh_uint64_t call_count;
    bh_uint64_t min_time;
    bh_uint64_t max_time;
} bh_perf_counter_t;

// Global debug state
extern bh_debug_config_t bh_debug_config;
extern bh_bool_t bh_debug_initialized;

/**
 * @brief Initialize the debug subsystem
 * 
 * @param config Debug configuration
 * @return bh_status_t Status code
 */
bh_status_t bh_debug_initialize(const bh_debug_config_t* config);

/**
 * @brief Shutdown the debug subsystem
 */
void bh_debug_shutdown(void);

/**
 * @brief Set debug level
 * 
 * @param level Minimum debug level to output
 */
void bh_debug_set_level(bh_debug_level_t level);

/**
 * @brief Get current debug level
 * 
 * @return bh_debug_level_t Current debug level
 */
bh_debug_level_t bh_debug_get_level(void);

/**
 * @brief Set debug category mask
 * 
 * @param mask Category mask for filtering
 */
void bh_debug_set_category_mask(bh_debug_category_t mask);

/**
 * @brief Get debug category mask
 * 
 * @return bh_debug_category_t Current category mask
 */
bh_debug_category_t bh_debug_get_category_mask(void);

/**
 * @brief Set debug flags
 * 
 * @param flags Debug formatting flags
 */
void bh_debug_set_flags(bh_debug_flags_t flags);

/**
 * @brief Get debug flags
 * 
 * @return bh_debug_flags_t Current debug flags
 */
bh_debug_flags_t bh_debug_get_flags(void);

/**
 * @brief Print debug message
 * 
 * @param level Debug level
 * @param category Debug category
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 * @param format Format string
 * @param ... Format arguments
 */
void bh_debug_print(
    bh_debug_level_t level,
    bh_debug_category_t category,
    const char* file,
    bh_uint32_t line,
    const char* function,
    const char* format,
    ...
);

/**
 * @brief Print debug message with va_list
 * 
 * @param level Debug level
 * @param category Debug category
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 * @param format Format string
 * @param args Format arguments
 */
void bh_debug_vprint(
    bh_debug_level_t level,
    bh_debug_category_t category,
    const char* file,
    bh_uint32_t line,
    const char* function,
    const char* format,
    __builtin_va_list args
);

/**
 * @brief Print memory dump
 * 
 * @param data Memory to dump
 * @param size Number of bytes to dump
 * @param address Base address for display
 */
void bh_debug_hexdump(const void* data, bh_size_t size, bh_uintptr_t address);

/**
 * @brief Capture stack trace
 * 
 * @param trace [out] Stack trace structure
 * @param skip_frames Number of frames to skip
 * @return bh_status_t Status code
 */
bh_status_t bh_debug_capture_stack_trace(bh_stack_trace_t* trace, bh_uint32_t skip_frames);

/**
 * @brief Free stack trace
 * 
 * @param trace Stack trace to free
 */
void bh_debug_free_stack_trace(bh_stack_trace_t* trace);

/**
 * @brief Print stack trace
 * 
 * @param trace Stack trace to print
 */
void bh_debug_print_stack_trace(const bh_stack_trace_t* trace);

/**
 * @brief Trigger debugger breakpoint
 */
void bh_debug_break(void);

/**
 * @brief Check if debugger is attached
 * 
 * @return bh_bool_t true if debugger is attached
 */
bh_bool_t bh_debug_is_debugger_present(void);

/**
 * @brief Assert with custom message
 * 
 * @param condition Condition to check
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 * @param message Assert message
 */
void bh_debug_assert(
    bh_bool_t condition,
    const char* file,
    bh_uint32_t line,
    const char* function,
    const char* message
);

/**
 * @brief Get debug memory buffer
 * 
 * @param buffer [out] Pointer to buffer data
 * @param size [out] Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_debug_get_memory_buffer(const char** buffer, bh_size_t* size);

/**
 * @brief Clear debug memory buffer
 */
void bh_debug_clear_memory_buffer(void);

/**
 * @brief Save debug log to file
 * 
 * @param filename Output filename
 * @return bh_status_t Status code
 */
bh_status_t bh_debug_save_log(const char* filename);

// Performance profiling functions

/**
 * @brief Create performance counter
 * 
 * @param name Counter name
 * @return bh_perf_counter_t* Performance counter
 */
bh_perf_counter_t* bh_perf_create_counter(const char* name);

/**
 * @brief Start performance counter
 * 
 * @param counter Performance counter
 */
void bh_perf_start_counter(bh_perf_counter_t* counter);

/**
 * @brief Stop performance counter
 * 
 * @param counter Performance counter
 */
void bh_perf_stop_counter(bh_perf_counter_t* counter);

/**
 * @brief Reset performance counter
 * 
 * @param counter Performance counter
 */
void bh_perf_reset_counter(bh_perf_counter_t* counter);

/**
 * @brief Print performance counters
 */
void bh_perf_print_counters(void);

/**
 * @brief Destroy performance counter
 * 
 * @param counter Performance counter to destroy
 */
void bh_perf_destroy_counter(bh_perf_counter_t* counter);

// Debug macros - only compiled in debug builds
#ifdef BH_DEBUG

#define BH_LOG_ERROR(category, format, ...) \
    bh_debug_print(BH_DEBUG_LEVEL_ERROR, category, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define BH_LOG_WARNING(category, format, ...) \
    bh_debug_print(BH_DEBUG_LEVEL_WARNING, category, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define BH_LOG_INFO(category, format, ...) \
    bh_debug_print(BH_DEBUG_LEVEL_INFO, category, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define BH_LOG_VERBOSE(category, format, ...) \
    bh_debug_print(BH_DEBUG_LEVEL_VERBOSE, category, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define BH_LOG_TRACE(category, format, ...) \
    bh_debug_print(BH_DEBUG_LEVEL_TRACE, category, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define BH_ASSERT(condition) \
    do { \
        if (BH_UNLIKELY(!(condition))) { \
            bh_debug_assert(false, __FILE__, __LINE__, __FUNCTION__, #condition); \
        } \
    } while (0)

#define BH_ASSERT_MSG(condition, message) \
    do { \
        if (BH_UNLIKELY(!(condition))) { \
            bh_debug_assert(false, __FILE__, __LINE__, __FUNCTION__, message); \
        } \
    } while (0)

#define BH_HEXDUMP(data, size, addr) bh_debug_hexdump(data, size, addr)

#define BH_BREAK() bh_debug_break()

#define BH_PERF_COUNTER(name) \
    static bh_perf_counter_t* _perf_counter_##name = NULL; \
    if (!_perf_counter_##name) _perf_counter_##name = bh_perf_create_counter(#name)

#define BH_PERF_START(name) bh_perf_start_counter(_perf_counter_##name)
#define BH_PERF_STOP(name) bh_perf_stop_counter(_perf_counter_##name)

#define BH_PERF_SCOPE(name) \
    BH_PERF_COUNTER(name); \
    bh_perf_start_counter(_perf_counter_##name); \
    struct _perf_scope_##name { \
        ~_perf_scope_##name() { bh_perf_stop_counter(_perf_counter_##name); } \
    } _perf_scope_instance_##name

#else // BH_DEBUG not defined

// Empty macros for release builds
#define BH_LOG_ERROR(category, format, ...)
#define BH_LOG_WARNING(category, format, ...)
#define BH_LOG_INFO(category, format, ...)
#define BH_LOG_VERBOSE(category, format, ...)
#define BH_LOG_TRACE(category, format, ...)
#define BH_ASSERT(condition)
#define BH_ASSERT_MSG(condition, message)
#define BH_HEXDUMP(data, size, addr)
#define BH_BREAK()
#define BH_PERF_COUNTER(name)
#define BH_PERF_START(name)
#define BH_PERF_STOP(name)
#define BH_PERF_SCOPE(name)

#endif // BH_DEBUG

// Convenience macros for common categories
#define BH_LOG_GENERAL_ERROR(format, ...)   BH_LOG_ERROR(BH_DEBUG_CAT_GENERAL, format, ##__VA_ARGS__)
#define BH_LOG_GENERAL_WARNING(format, ...) BH_LOG_WARNING(BH_DEBUG_CAT_GENERAL, format, ##__VA_ARGS__)
#define BH_LOG_GENERAL_INFO(format, ...)    BH_LOG_INFO(BH_DEBUG_CAT_GENERAL, format, ##__VA_ARGS__)

#define BH_LOG_MEMORY_ERROR(format, ...)    BH_LOG_ERROR(BH_DEBUG_CAT_MEMORY, format, ##__VA_ARGS__)
#define BH_LOG_MEMORY_WARNING(format, ...)  BH_LOG_WARNING(BH_DEBUG_CAT_MEMORY, format, ##__VA_ARGS__)
#define BH_LOG_MEMORY_INFO(format, ...)     BH_LOG_INFO(BH_DEBUG_CAT_MEMORY, format, ##__VA_ARGS__)

#define BH_LOG_GRAPHICS_ERROR(format, ...)  BH_LOG_ERROR(BH_DEBUG_CAT_GRAPHICS, format, ##__VA_ARGS__)
#define BH_LOG_GRAPHICS_WARNING(format, ...) BH_LOG_WARNING(BH_DEBUG_CAT_GRAPHICS, format, ##__VA_ARGS__)
#define BH_LOG_GRAPHICS_INFO(format, ...)   BH_LOG_INFO(BH_DEBUG_CAT_GRAPHICS, format, ##__VA_ARGS__)

#define BH_LOG_SYSTEM_ERROR(format, ...)    BH_LOG_ERROR(BH_DEBUG_CAT_SYSTEM, format, ##__VA_ARGS__)
#define BH_LOG_SYSTEM_WARNING(format, ...)  BH_LOG_WARNING(BH_DEBUG_CAT_SYSTEM, format, ##__VA_ARGS__)
#define BH_LOG_SYSTEM_INFO(format, ...)     BH_LOG_INFO(BH_DEBUG_CAT_SYSTEM, format, ##__VA_ARGS__)

// Memory debugging
#ifdef BH_DEBUG_MEMORY
void* bh_debug_malloc(bh_size_t size, const char* file, bh_uint32_t line);
void bh_debug_free(void* ptr, const char* file, bh_uint32_t line);
void* bh_debug_realloc(void* ptr, bh_size_t size, const char* file, bh_uint32_t line);
void bh_debug_memory_report(void);

#define bh_malloc(size) bh_debug_malloc(size, __FILE__, __LINE__)
#define bh_free(ptr) bh_debug_free(ptr, __FILE__, __LINE__)
#define bh_realloc(ptr, size) bh_debug_realloc(ptr, size, __FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_DEBUG_H
