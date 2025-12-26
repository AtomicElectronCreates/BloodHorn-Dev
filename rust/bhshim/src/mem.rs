#![allow(dead_code)]

use core::ffi::c_void;

pub type AllocFn = extern "C" fn(usize) -> *mut c_void;
pub type FreeFn  = extern "C" fn(*mut c_void);

pub const PAGE_SIZE_4K: usize = 0x1000;
