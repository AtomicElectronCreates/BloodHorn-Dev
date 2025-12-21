/*
 * types.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_TYPES_H
#define BLOODHORN_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic integer types (ensure consistency across platforms)
typedef uint8_t   bh_uint8_t;
typedef int8_t    bh_int8_t;
typedef uint16_t  bh_uint16_t;
typedef int16_t   bh_int16_t;
typedef uint32_t  bh_uint32_t;
typedef int32_t   bh_int32_t;
typedef uint64_t  bh_uint64_t;
typedef int64_t   bh_int64_t;

// Size and pointer types
typedef size_t    bh_size_t;
typedef uintptr_t bh_uintptr_t;
typedef intptr_t  bh_intptr_t;

// Address types
typedef uint64_t  bh_phys_addr_t;    // Physical address
typedef uintptr_t bh_virt_addr_t;    // Virtual address
typedef uint64_t  bh_addr_t;         // Generic address

// Handle type (opaque pointer)
typedef void*     bh_handle_t;

// Universal Unique Identifier
typedef struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t  data4[8];
} bh_uuid_t;

// Boolean type (for consistency)
typedef bool bh_bool_t;
#define BH_TRUE  true
#define BH_FALSE false

// NULL pointer
#ifndef BH_NULL
#define BH_NULL ((void*)0)
#endif

// Page size constants
#define BH_PAGE_SIZE_4K    0x1000U
#define BH_PAGE_SIZE_2M    0x200000U
#define BH_PAGE_SIZE_1G    0x40000000U

// Alignment macros
#define BH_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

#define BH_ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

#define BH_IS_ALIGNED(value, alignment) \
    (((value) & ((alignment) - 1)) == 0)

// Min/Max macros
#define BH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define BH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define BH_CLAMP(value, min_val, max_val) \
    BH_MAX((min_val), BH_MIN((value), (max_val)))

// Array size macro
#define BH_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Offset calculation
#define BH_OFFSET_OF(type, member) \
    ((bh_size_t)&((type*)0)->member)

// Container of macro
#define BH_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - BH_OFFSET_OF(type, member)))

// Bit manipulation macros
#define BH_BIT(n)                (1UL << (n))
#define BH_SET_BIT(value, n)     ((value) |= BH_BIT(n))
#define BH_CLEAR_BIT(value, n)   ((value) &= ~BH_BIT(n))
#define BH_TOGGLE_BIT(value, n)  ((value) ^= BH_BIT(n))
#define BH_TEST_BIT(value, n)    (((value) & BH_BIT(n)) != 0)

// Byte manipulation macros
#define BH_MAKE_WORD(low, high)  ((bh_uint16_t)(((bh_uint8_t)(low)) | ((bh_uint16_t)((bh_uint8_t)(high))) << 8))
#define BH_MAKE_DWORD(low, high) ((bh_uint32_t)(((bh_uint16_t)(low)) | ((bh_uint32_t)((bh_uint16_t)(high))) << 16))
#define BH_MAKE_QWORD(low, high) ((bh_uint64_t)(((bh_uint32_t)(low)) | ((bh_uint64_t)((bh_uint32_t)(high))) << 32))

#define BH_LOBYTE(w)   ((bh_uint8_t)(w))
#define BH_HIBYTE(w)   ((bh_uint8_t)((w) >> 8))
#define BH_LOWORD(l)   ((bh_uint16_t)(l))
#define BH_HIWORD(l)   ((bh_uint16_t)((l) >> 16))
#define BH_LODWORD(l)  ((bh_uint32_t)(l))
#define BH_HIDWORD(l)  ((bh_uint32_t)((l) >> 32))

// Packed structure attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define BH_PACKED
#pragma pack(push, 1)
#else
#define BH_PACKED
#endif

// Alignment attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_ALIGNED(n) __attribute__((aligned(n)))
#elif defined(_MSC_VER)
#define BH_ALIGNED(n) __declspec(align(n))
#else
#define BH_ALIGNED(n)
#endif

// Force inline
#if defined(__GNUC__) || defined(__clang__)
#define BH_FORCE_INLINE static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define BH_FORCE_INLINE static __forceinline
#else
#define BH_FORCE_INLINE static inline
#endif

// No return attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define BH_NORETURN __declspec(noreturn)
#else
#define BH_NORETURN
#endif

// Deprecated attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define BH_DEPRECATED __declspec(deprecated)
#else
#define BH_DEPRECATED
#endif

// Weak symbol attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_WEAK __attribute__((weak))
#else
#define BH_WEAK
#endif

// Section attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_SECTION(name) __attribute__((section(name)))
#elif defined(_MSC_VER)
#define BH_SECTION(name) __declspec(allocate(name))
#else
#define BH_SECTION(name)
#endif

// Unused parameter attribute
#if defined(__GNUC__) || defined(__clang__)
#define BH_UNUSED __attribute__((unused))
#else
#define BH_UNUSED
#endif

// Likely/unlikely branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
#define BH_LIKELY(x)   __builtin_expect(!!(x), 1)
#define BH_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define BH_LIKELY(x)   (x)
#define BH_UNLIKELY(x) (x)
#endif

// Compiler barriers
#if defined(__GNUC__) || defined(__clang__)
#define BH_COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#elif defined(_MSC_VER)
#define BH_COMPILER_BARRIER() _ReadWriteBarrier()
#else
#define BH_COMPILER_BARRIER()
#endif

// Memory barriers
#if defined(__GNUC__) || defined(__clang__)
#define BH_MEMORY_BARRIER() __sync_synchronize()
#elif defined(_MSC_VER)
#define BH_MEMORY_BARRIER() MemoryBarrier()
#else
#define BH_MEMORY_BARRIER() BH_COMPILER_BARRIER()
#endif

// Static assert
#if defined(__cplusplus) && __cplusplus >= 201103L
#define BH_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define BH_STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#else
#define BH_STATIC_ASSERT(expr, msg) \
    typedef char bh_static_assert_##__LINE__[(expr) ? 1 : -1]
#endif

// Safe string operations
#define BH_STRNCPY_SAFE(dest, src, size) \
    do { \
        strncpy(dest, src, (size) - 1); \
        (dest)[(size) - 1] = '\0'; \
    } while (0)

#define BH_STRNCAT_SAFE(dest, src, size) \
    do { \
        size_t len = strlen(dest); \
        if (len < (size) - 1) { \
            strncat(dest, src, (size) - len - 1); \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_TYPES_H
