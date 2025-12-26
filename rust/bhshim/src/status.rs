#![allow(dead_code)]

pub type BhStatus = i32;

pub const BH_SUCCESS: BhStatus             = 0;
pub const BH_ERROR: BhStatus               = -1;
pub const BH_INVALID_PARAMETER: BhStatus   = -2;
pub const BH_NOT_SUPPORTED: BhStatus       = -3;
pub const BH_OUT_OF_RESOURCES: BhStatus    = -4;
pub const BH_DEVICE_ERROR: BhStatus        = -5;

pub const EFI_SUCCESS: i32          = 0;
pub const EFI_BUFFER_TOO_SMALL: i32 = 5;
