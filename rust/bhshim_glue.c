/*
 * rust bhshim glue
 *
 * Bridges UEFI GetMemoryMap to BloodHorn's bh_system_table using the
 * Rust adapter implemented in rust/bhshim.
 */

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include "boot/libb/include/bloodhorn/bloodhorn.h"
#include "rust/bhshim/bhshim.h"

// Wrapper that matches bh_system_table->get_memory_map signature.
bh_status_t
bhshim_get_memory_map(
    bh_memory_descriptor_t **Map,
    bh_size_t *MapSize,
    bh_size_t *DescriptorSize
)
{
    if (!Map || !MapSize || !DescriptorSize || gBS == NULL) {
        return BH_INVALID_PARAMETER;
    }

    UINT32 desc_count = 0;
    UINT32 desc_size32 = 0;

    EFI_STATUS Status = bhshim_get_memory_map_adapter(
        Map,
        &desc_count,
        &desc_size32,
        (BHSHIM_UEFI_GET_MEMORY_MAP)gBS->GetMemoryMap,
        (BHSHIM_ALLOC)AllocatePool,
        (BHSHIM_FREE)FreePool
    );

    if (EFI_ERROR(Status)) {
        return BH_DEVICE_ERROR;
    }

    *MapSize = (bh_size_t)desc_count * (bh_size_t)desc_size32;
    *DescriptorSize = (bh_size_t)desc_size32;

    return BH_SUCCESS;
}
