/*
 * bhshim_bootstrap.c
 *
 * Host-side bootstrapper built by EDK2 that is responsible for building the
 * Rust bhshim static library for all BloodHorn-supported architectures.
 *
 * This is not part of the firmware image; it is intended to be run on the
 * build host prior to linking BloodHorn.
 */

#include <stdio.h>
#include <stdlib.h>

static int run(const char *cmd) {
    int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "bhshim bootstrap: command failed: %s (rc=%d)\n", cmd, rc);
    }
    return rc;
}

int main(void) {
    int rc = 0;

    /* Build all Rust crates used by BloodHorn for all supported architectures. */

    /* x86_64 */
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhshim/Cargo.toml");
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhcore/Cargo.toml");
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhlog/Cargo.toml");
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhcfg/Cargo.toml");
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhnet/Cargo.toml");
    rc |= run("cargo build -q --release --target x86_64-unknown-none --manifest-path rust/bhutil/Cargo.toml");

    /* AArch64 */
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhshim/Cargo.toml");
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhcore/Cargo.toml");
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhlog/Cargo.toml");
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhcfg/Cargo.toml");
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhnet/Cargo.toml");
    rc |= run("cargo build -q --release --target aarch64-unknown-none --manifest-path rust/bhutil/Cargo.toml");

    /* RISC-V 64 */
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhshim/Cargo.toml");
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhcore/Cargo.toml");
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhlog/Cargo.toml");
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhcfg/Cargo.toml");
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhnet/Cargo.toml");
    rc |= run("cargo build -q --release --target riscv64gc-unknown-none-elf --manifest-path rust/bhutil/Cargo.toml");

    /* LoongArch 64 – adjust target triple as needed if your toolchain differs */
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhshim/Cargo.toml");
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhcore/Cargo.toml");
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhlog/Cargo.toml");
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhcfg/Cargo.toml");
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhnet/Cargo.toml");
    rc |= run("cargo build -q --release --target loongarch64-unknown-none --manifest-path rust/bhutil/Cargo.toml");

    if (rc != 0) {
        fprintf(stderr, "bhshim bootstrap: one or more cargo builds failed.\n");
        return 1;
    }

    printf("bhshim bootstrap: Rust shims built successfully.\n");
    return 0;
}
