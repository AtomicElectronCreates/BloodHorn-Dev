/*
 * bhshim_bootstrap.c
 *
 * Host-side bootstrapper built by EDK2 that is responsible for building
 * Rust crates for BloodHorn-supported architectures.
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

static int build_one(const char *cargo, const char *target, const char *manifest_path) {
    char cmd[512];
    int len = snprintf(
        cmd,
        sizeof(cmd),
        "%s build -q --release --target %s --manifest-path %s",
        cargo,
        target,
        manifest_path
    );

    if (len < 0 || (size_t)len >= sizeof(cmd)) {
        fprintf(stderr, "bhshim bootstrap: command too long for %s (%s)\n", target, manifest_path);
        return 1;
    }

    return run(cmd);
}

int main(void) {
    const char *cargo = getenv("CARGO");
    int failed = 0;
    size_t i;
    size_t j;

    const char *targets[] = {
        "x86_64-unknown-none",
        "aarch64-unknown-none",
        "riscv64gc-unknown-none-elf",
        "loongarch64-unknown-none"
    };

    const char *manifests[] = {
        "rust/bhshim/Cargo.toml",
        "rust/bhcore/Cargo.toml",
        "rust/bhlog/Cargo.toml",
        "rust/bhcfg/Cargo.toml",
        "rust/bhnet/Cargo.toml",
        "rust/bhutil/Cargo.toml"
    };

    if (cargo == NULL || cargo[0] == '\0') {
        cargo = "cargo";
    }

    for (i = 0; i < (sizeof(targets) / sizeof(targets[0])); ++i) {
        for (j = 0; j < (sizeof(manifests) / sizeof(manifests[0])); ++j) {
            if (build_one(cargo, targets[i], manifests[j]) != 0) {
                ++failed;
            }
        }
    }

    if (failed != 0) {
        fprintf(stderr, "bhshim bootstrap: %d cargo build command(s) failed.\n", failed);
        return 1;
    }

    printf("bhshim bootstrap: Rust crates built successfully.\n");
    return 0;
}
