/*
 * secure_boot.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include "secure_boot.h"
#include "compat.h"
#include "crypto.h"
int secure_boot_verify(const uint8_t* kernel, int ksize, const uint8_t* sig, const uint8_t* pubkey) {
    return verify_signature(kernel, ksize, sig, pubkey);
} 