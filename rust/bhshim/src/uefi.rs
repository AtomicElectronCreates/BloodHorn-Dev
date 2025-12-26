#![allow(dead_code)]

use core::ffi::c_void;

#[repr(C)]
pub struct EfiMemoryDescriptor {
    pub typ: u32,
    pub pad: u32,
    pub physical_start: u64,
    pub virtual_start: u64,
    pub number_of_pages: u64,
    pub attribute: u64,
}

pub type UefiGetMemoryMapFn = unsafe extern "C" fn(
    *mut usize,
    *mut c_void,
    *mut usize,
    *mut usize,
    *mut u32,
) -> i32;
