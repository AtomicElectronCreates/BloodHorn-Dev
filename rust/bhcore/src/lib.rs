#![no_std]

/// Core BloodHorn status codes and helpers.
pub mod status {
    #[derive(Copy, Clone, Debug, Eq, PartialEq)]
    #[repr(i32)]
    pub enum BhStatus {
        Success            = 0,
        Error              = -1,
        InvalidParameter   = -2,
        NotSupported       = -3,
        OutOfResources     = -4,
        DeviceError        = -5,
        Timeout            = -6,
        InvalidState       = -7,
        AlreadyExists      = -8,
        NotFound           = -9,
        Interrupted        = -10,
    }

    impl BhStatus {
        #[inline]
        pub const fn is_success(self) -> bool {
            matches!(self, BhStatus::Success)
        }

        #[inline]
        pub const fn is_error(self) -> bool {
            !self.is_success()
        }
    }
}

/// Simple math helpers used across boot logic.
pub mod math {
    #[inline]
    pub const fn align_up(value: usize, alignment: usize) -> usize {
        if alignment == 0 { return value; }
        (value + alignment - 1) & !(alignment - 1)
    }

    #[inline]
    pub const fn align_down(value: usize, alignment: usize) -> usize {
        if alignment == 0 { return value; }
        value & !(alignment - 1)
    }

    #[inline]
    pub const fn is_pow2(v: usize) -> bool {
        v != 0 && (v & (v - 1)) == 0
    }

    #[inline]
    pub const fn ceil_div(a: usize, b: usize) -> usize {
        (a + b - 1) / b
    }

    #[inline]
    pub const fn floor_div(a: usize, b: usize) -> usize {
        a / b
    }
}

/// Endianness and byte helpers.
pub mod bytes {
    #[inline]
    pub const fn le_u16(b: [u8; 2]) -> u16 {
        (b[0] as u16) | ((b[1] as u16) << 8)
    }

    #[inline]
    pub const fn le_u32(b: [u8; 4]) -> u32 {
        (b[0] as u32)
            | ((b[1] as u32) << 8)
            | ((b[2] as u32) << 16)
            | ((b[3] as u32) << 24)
    }

    #[inline]
    pub const fn be_u16(b: [u8; 2]) -> u16 {
        ((b[0] as u16) << 8) | (b[1] as u16)
    }

    #[inline]
    pub const fn be_u32(b: [u8; 4]) -> u32 {
        ((b[0] as u32) << 24)
            | ((b[1] as u32) << 16)
            | ((b[2] as u32) << 8)
            | (b[3] as u32)
    }

    #[inline]
    pub const fn le_u64(b: [u8; 8]) -> u64 {
        (b[0] as u64)
            | ((b[1] as u64) << 8)
            | ((b[2] as u64) << 16)
            | ((b[3] as u64) << 24)
            | ((b[4] as u64) << 32)
            | ((b[5] as u64) << 40)
            | ((b[6] as u64) << 48)
            | ((b[7] as u64) << 56)
    }

    #[inline]
    pub const fn be_u64(b: [u8; 8]) -> u64 {
        ((b[0] as u64) << 56)
            | ((b[1] as u64) << 48)
            | ((b[2] as u64) << 40)
            | ((b[3] as u64) << 32)
            | ((b[4] as u64) << 24)
            | ((b[5] as u64) << 16)
            | ((b[6] as u64) << 8)
            | (b[7] as u64)
    }
}

/// Compile-time architecture description used by Rust-side code.
pub mod arch {
    #[derive(Copy, Clone, Debug, Eq, PartialEq)]
    pub enum Arch {
        X86_64,
        Aarch64,
        Riscv64,
        LoongArch64,
        Unknown,
    }

    #[inline]
    pub const fn current() -> Arch {
        #[cfg(target_arch = "x86_64")]
        { Arch::X86_64 }

        #[cfg(target_arch = "aarch64")]
        { Arch::Aarch64 }

        #[cfg(target_arch = "riscv64")]
        { Arch::Riscv64 }

        #[cfg(target_arch = "loongarch64")]
        { Arch::LoongArch64 }

        #[cfg(not(any(
            target_arch = "x86_64",
            target_arch = "aarch64",
            target_arch = "riscv64",
            target_arch = "loongarch64",
        )))]
        { Arch::Unknown }
    }
}
