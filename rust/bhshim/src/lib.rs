#![no_std]

mod status;
mod uefi;
mod mem;

use core::ffi::c_void;
use core::ptr::null_mut;

use crate::status::{EFI_SUCCESS, EFI_BUFFER_TOO_SMALL};
use crate::uefi::{EfiMemoryDescriptor, UefiGetMemoryMapFn};
use crate::mem::{AllocFn as BhAllocFn, FreeFn as BhFreeFn, PAGE_SIZE_4K};

// These layouts mirror BloodHorn's bh_memory_descriptor_t from bloodhorn.h.

#[repr(C)]
pub struct BhMemoryDescriptor {
    pub base_address: u64,   // bh_addr_t
    pub length: u32,         // bh_size_t (uint32_t)
    pub typ: u32,            // type
    pub attributes: u32,     // attributes
}

/// Safe-ish adapter used from C: fills a BloodHorn-style memory map using
/// UEFI's GetMemoryMap and bootloader-provided alloc/free.
///
/// bh_map_out: receives pointer to an array of BhMemoryDescriptor
/// count_out: number of descriptors
/// desc_size_out: sizeof(BhMemoryDescriptor)
#[no_mangle]
pub extern "C" fn bhshim_get_memory_map_adapter(
    bh_map_out: *mut *mut BhMemoryDescriptor,
    count_out: *mut u32,
    desc_size_out: *mut u32,
    uefi_get_memory_map: UefiGetMemoryMapFn,
    alloc: BhAllocFn,
    free_fn: BhFreeFn,
) -> i32 {
    if bh_map_out.is_null() || count_out.is_null() || desc_size_out.is_null() {
        // BH_INVALID_PARAMETER (-2) by convention
        return -2;
    }

    let mut map_size: usize = 0;
    let mut descriptor_size: usize = 0;
    let mut descriptor_version: u32 = 0;
    let mut map_key: usize = 0;

    // First call to get required size.
    let mut status = unsafe {
        uefi_get_memory_map(
            &mut map_size,
            null_mut(),
            &mut map_key,
            &mut descriptor_size,
            &mut descriptor_version,
        )
    };

    if status != EFI_BUFFER_TOO_SMALL {
        return status;
    }

    // Add some slack to avoid race conditions.
    map_size += descriptor_size * 8;

    let buffer = alloc(map_size);
    if buffer.is_null() {
        // BH_OUT_OF_RESOURCES (-4)
        return -4;
    }

    status = unsafe {
        uefi_get_memory_map(
            &mut map_size,
            buffer,
            &mut map_key,
            &mut descriptor_size,
            &mut descriptor_version,
        )
    };

    if status != EFI_SUCCESS {
        unsafe { free_fn(buffer); }
        return status;
    }

    let count = if descriptor_size == 0 {
        0
    } else {
        map_size / descriptor_size
    };

    if count == 0 {
        unsafe {
            *bh_map_out = core::ptr::null_mut();
            *count_out = 0;
            *desc_size_out = core::mem::size_of::<BhMemoryDescriptor>() as u32;
        }
        unsafe { free_fn(buffer); }
        return EFI_SUCCESS;
    }

    let bh_desc_size = core::mem::size_of::<BhMemoryDescriptor>();
    let bh_buffer_bytes = bh_desc_size.checked_mul(count).unwrap_or(0);

    if bh_buffer_bytes == 0 {
        unsafe { free_fn(buffer); }
        return -4;
    }

    let bh_buffer = alloc(bh_buffer_bytes) as *mut BhMemoryDescriptor;
    if bh_buffer.is_null() {
        unsafe { free_fn(buffer); }
        return -4;
    }

    // Copy and translate each EFI descriptor.
    for i in 0..count {
        let efi_ptr = unsafe { (buffer as *const u8).add(i * descriptor_size) } as *const EfiMemoryDescriptor;
        let efi = unsafe { &*efi_ptr };

        let bh = BhMemoryDescriptor {
            base_address: efi.physical_start,
            length: (efi.number_of_pages.saturating_mul((PAGE_SIZE_4K as u64))) as u32,
            typ: efi.typ,
            attributes: (efi.attribute & 0xFFFF_FFFF) as u32,
        };

        unsafe {
            *bh_buffer.add(i) = bh;
        }
    }

    unsafe {
        *bh_map_out = bh_buffer;
        *count_out = count as u32;
        *desc_size_out = bh_desc_size as u32;
    }

    unsafe { free_fn(buffer); }

    EFI_SUCCESS
}
