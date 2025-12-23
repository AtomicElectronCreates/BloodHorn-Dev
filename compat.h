/*
 * compat.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef _COMPAT_H_
#define _COMPAT_H_

// Include cLib for complete C standard library functionality
#define CLIB_IMPLEMENTATION
#include "cLib/src/cLib.c"

// Include cc-runtime for compiler compatibility
#include "cc-runtime/src/cc-runtime.c"

// Detect EDK2 environment and provide compatibility layer
#ifdef __EDK2__
  #include <Uefi.h>
  #include <Library/BaseLib.h>
  #include <Library/BaseMemoryLib.h>
  #include <Library/MemoryAllocationLib.h>
  #include <Library/DebugLib.h>

  // EDK2-specific overrides for cLib functions
  // Memory management - use EDK2 allocators but keep cLib interface
  void* compat_edk2_malloc(size_t size) {
    return AllocatePool(size);
  }
  
  void compat_edk2_free(void* ptr) {
    if (ptr) FreePool(ptr);
  }
  
  void* compat_edk2_realloc(void* ptr, size_t size) {
    if (!ptr) return AllocatePool(size);
    if (size == 0) {
      FreePool(ptr);
      return NULL;
    }
    
    void* new_ptr = AllocatePool(size);
    if (new_ptr && ptr) {
      CopyMem(new_ptr, ptr, size);
      FreePool(ptr);
    }
    return new_ptr;
  }

  // Override cLib memory functions with EDK2 implementations
  #define malloc compat_edk2_malloc
  #define free compat_edk2_free
  #define realloc compat_edk2_realloc
  #define calloc(nmemb, size) AllocateZeroPool((nmemb) * (size))

  // I/O functions - redirect to EDK2 DebugLib
  int compat_edk2_printf(const char* format, ...) {
    // Simple printf implementation using DebugPrint
    // For full printf functionality, would need more complex parsing
    VA_LIST Marker;
    VA_START(Marker, format);
    DebugPrint(DEBUG_INFO, format, Marker);
    VA_END(Marker);
    return 0;
  }

  #define printf compat_edk2_printf
  #define fprintf(stream, format, ...) compat_edk2_printf(format, ##__VA_ARGS__)
  #define puts(s) DebugPrint(DEBUG_INFO, "%s\n", s)
  #define putchar(c) DebugPrint(DEBUG_INFO, "%c", c)

  // File operations - stubbed for EDK2 environment
  #define fopen(name, mode) ((FILE*)NULL)
  #define fclose(stream) (-1)
  #define fread(ptr, size, nmemb, stream) (0)
  #define fwrite(ptr, size, nmemb, stream) (0)
  #define fseek(stream, offset, whence) (-1)
  #define ftell(stream) (-1)
  #define fflush(stream) (0)

  // System functions - EDK2 equivalents
  #define exit(code) CpuDeadLoop()
  #define abort() CpuDeadLoop()
  #define assert(condition) ASSERT(condition)

  // Error handling
  #define errno 0  // EDK2 doesn't have errno

  // Time functions - stubbed for EDK2
  #define time(t) ((time_t)0)
  #define clock() ((clock_t)0)

#endif // __EDK2__

// Provide EDK2-style types for compatibility when needed
#ifndef __EDK2__
  typedef uint8_t   UINT8;
  typedef uint16_t  UINT16;
  typedef uint32_t  UINT32;
  typedef uint64_t  UINT64;
  typedef int8_t    INT8;
  typedef int16_t   INT16;
  typedef int32_t   INT32;
  typedef int64_t   INT64;
  typedef size_t    UINTN;
  typedef void      VOID;
  typedef int       BOOLEAN;

  #ifndef TRUE
  #define TRUE 1
  #define FALSE 0
  #endif

  // EDK2-style macros for compatibility
  #define EFIAPI
  #define IN
  #define OUT
  #define OPTIONAL
  #define CONST const

  // EDK2-style memory function aliases
  #define CopyMem(dest, src, size) memcpy((dest), (src), (size))
  #define SetMem(buf, size, val) memset((buf), (val), (size))
  #define ZeroMem(buf, size) memset((buf), 0, (size))
  #define CompareMem(a, b, len) memcmp((a), (b), (len))

  // EDK2-style string function aliases
  #define StrLen(s) strlen((const char*)(s))
  #define StrCmp strcmp
  #define StrnCmp strncmp
  #define AsciiStrLen strlen
  #define AsciiStrCmp strcmp

#endif // !__EDK2__

// Common constants and macros
#ifndef NULL
#define NULL ((void*)0)
#endif

// Ensure standard C library is fully available through cLib
// All standard headers are included via cLib.h in cLib.c

#endif // _COMPAT_H_
