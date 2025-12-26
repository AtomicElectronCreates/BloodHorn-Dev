# BloodHorn Rust Subsystems

This directory contains Rust `no_std` crates that can be used by the bootloader
for additional functionality:

- `bhshim`: UEFI/BloodHorn memory map adapter used by the main bootloader.
- `bhcore`: Common types and helpers (status codes, alignment, byte utils).
- `bhlog`: Lightweight logging abstractions usable with any `core::fmt::Write`.
- `bhcfg`: Tiny key=value configuration line parser.
- `bhnet`: Simple network helper types and checksum routine.
- `bhutil`: Miscellaneous small utilities (string helpers, etc.).

Currently only `bhshim` is wired into the C/EDK2 side; the others are features to be used within the `rust` folder.

These Rust crates are **internal helpers**, not marketing fluff:

- They exist to make Rust-based parts of BloodHorn safer and easier to implement.
- They are compiled as separate static libraries and only affect the final bootloader
  image if they are explicitly linked and wired into the C/EDK2 build.
- The goal is to fix sharp edges (like unsafe pointer casts) and provide solid
  building blocks, *not* to bloat the bootloader or bolt on pointless
  "feature-rich" modules.
