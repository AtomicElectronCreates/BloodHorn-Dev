#ifndef BHSHIM_H
#define BHSHIM_H

#include <stdint.h>
#include <bloodhorn/bloodhorn.h>

#ifdef __cplusplus
extern "C" {
#endif

// FFI bridge into Rust shim. Implemented in rust/bhshim/src/lib.rs.

// Mirror of Rust BhMemoryDescriptor; layout must match bh_memory_descriptor_t.
// We reuse bh_memory_descriptor_t directly to avoid duplication.

// Type of UEFI GetMemoryMap that we pass into Rust.
typedef EFI_STATUS (*BHSHIM_UEFI_GET_MEMORY_MAP)(
    UINTN *MemoryMapSize,
    VOID  *MemoryMap,
    UINTN *MapKey,
    UINTN *DescriptorSize,
    UINT32 *DescriptorVersion
);

// Allocation and free callbacks forwarded into Rust.
typedef VOID *(*BHSHIM_ALLOC)(UINTN Size);
typedef VOID (*BHSHIM_FREE)(VOID *Ptr);

EFI_STATUS
EFIAPI
bhshim_get_memory_map_adapter(
    bh_memory_descriptor_t **Descriptors,
    UINT32 *DescriptorCount,
    UINT32 *DescriptorSize,
    BHSHIM_UEFI_GET_MEMORY_MAP GetMemoryMap,
    BHSHIM_ALLOC Alloc,
    BHSHIM_FREE FreeFn
);

#ifdef __cplusplus
}
#endif

#endif // BHSHIM_H
