/*
 * secure_boot.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_SECURE_BOOT_H
#define BLOODHORN_SECURE_BOOT_H
int secure_boot_verify(const uint8_t* kernel, int ksize, const uint8_t* sig, const uint8_t* pubkey);
#endif 