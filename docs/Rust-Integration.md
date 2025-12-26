# Rust Integration in BloodHorn

BloodHorn optionally uses Rust for a few very specific tasks. The goal is to
improve safety around sharp edges (like firmware function-pointer shims) and to
provide a clean place to grow Rust-based helpers over time.

Rust is **not** used to bloat the bootloader or bolt on pointless features.
Only crates that are explicitly wired into the C/EDK2 side affect the final
firmware image.

## Layout

Rust code lives under the `rust/` directory:

- `bhshim`: UEFI/BloodHorn memory-map adapter used by the main bootloader.
- `bhcore`: Common types and helpers (status codes, alignment, endian utils).
- `bhlog`: Lightweight logging abstractions usable with any `core::fmt::Write`.
- `bhcfg`: Tiny key=value configuration line parser.
- `bhnet`: Simple network helper types and checksum routine.
- `bhutil`: Miscellaneous small utilities (string helpers, etc.).

Currently **only** `bhshim` is wired into the firmware image; the other crates
are internal helper libraries for Rust code.

## bhshim: safe UEFI GetMemoryMap adapter

The main reason Rust was introduced is to replace an unsafe function-pointer
cast used to bridge UEFI's `GetMemoryMap` to BloodHorn's internal
`bh_system_table.get_memory_map` callback.

Instead of casting `gBS->GetMemoryMap` to a mismatched function pointer, the
bootloader now calls into a Rust-backed adapter:

1. C glue (`rust/bhshim_glue.c`) exposes a function named `bhshim_get_memory_map`
   with the exact signature expected by `bh_system_table_t.get_memory_map`.
2. That C function forwards to the Rust function
   `bhshim_get_memory_map_adapter` in `rust/bhshim/src/lib.rs`, passing:
   - A pointer to the UEFI `GetMemoryMap` function.
   - Allocation and free callbacks (UEFI `AllocatePool` / `FreePool`).
3. Rust code:
   - Calls `GetMemoryMap` to determine the buffer size.
   - Allocates a temporary buffer using the provided callbacks.
   - Calls `GetMemoryMap` again to fetch the descriptors.
   - Translates `EFI_MEMORY_DESCRIPTOR` entries into BloodHorn-style
     memory descriptors.
   - Allocates the final descriptor array and returns it to C.

This keeps the ABI surface small, makes the translation logic easier to reason
about, and removes the undefined behaviour from the original cast.

## Other Rust crates

The additional Rust crates are **internal building blocks** and are not linked
into the firmware image unless explicitly requested:

- `bhcore` provides:
  - A `BhStatus` enum mirroring common bootloader status codes.
  - Alignment helpers (`align_up`, `align_down`, power-of-two checks).
  - Endianness helpers for 16/32/64-bit LE/BE integers.
  - A tiny `Arch` descriptor that reports the compile-time target architecture.

- `bhlog` provides:
  - A `Level` enum (trace/debug/info/warn/error).
  - A `Logger` trait used by Rust-side components.
  - A `WriterLogger` that can log into any `core::fmt::Write` sink.
  - A ring-buffer `BufferLogger` for capturing early-boot logs in memory.

- `bhcfg` provides a small `no_std` parser for simple `key=value` style
  configuration lines.

- `bhnet` provides basic network helpers:
  - `Mac` and `Ipv4` types.
  - An RFC 1071-style 16-bit checksum routine.

- `bhutil` contains small string and utility helpers useful for parsers and
  protocol handling.

These crates are intended for **Rust code inside the `rust/` folder**. They are
not wired into the boot path by default and do not change runtime behaviour
unless a specific crate is linked and used via an FFI surface.

## Build integration

Rust is integrated into the EDK2 build via a small host-side bootstrap tool:

- `rust/bhshim_bootstrap.c` and `rust/bhshim_bootstrap.inf` define a
  `HOST_APPLICATION` that EDK2 builds and runs on the build machine.
- That tool invokes `cargo build --release` for all Rust crates and all
  supported architectures (x86_64, AArch64, RISC-V, LoongArch), producing
  `lib*.a` static libraries.
- The main `BloodHorn.dsc` file is configured to link against `libbhshim.a`
  for the appropriate architecture. Other crates remain unused until they are
  explicitly added to the link line and wired through C headers.

To build Rust components successfully, the corresponding Rust targets must be
installed on the build host (for example, via `rustup target add`).

## Design philosophy

- Keep the Rust surface **tight and focused**: a few crates with clear, small
  responsibilities.
- Use Rust where it adds real value (FFI boundaries, helpers that are easier to
  reason about in Rust than in C).
- Do not replace existing, battle-tested C code just for the sake of using
  Rust.
- Ensure any Rust that is wired into the boot path fails cleanly and does not
  silently weaken BloodHorn's security model.

If you are contributing Rust code, prefer extending these shared crates or
adding similarly focused new crates under `rust/` instead of sprinkling Rust
into unrelated parts of the C codebase.
