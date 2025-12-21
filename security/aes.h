/*
 * aes.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_AES_H
#define BLOODHORN_AES_H
#include <stdint.h>
#include "compat.h"

// AES block size is always 16 bytes
#define AES_BLOCK_SIZE 16
#define AES_IV_SIZE 16
#define AES_GCM_TAG_SIZE 16

// Legacy function (for backward compatibility)
void aes_encrypt_block(const uint8_t* in, uint8_t* out, const uint8_t* key);

// Extended AES functionality
int aes_key_expansion(const uint8_t* key, uint32_t key_bits, uint32_t* round_keys);
void aes_encrypt_block_ex(const uint32_t* round_keys, uint32_t rounds, const uint8_t* plaintext, uint8_t* ciphertext);
void aes_decrypt_block_ex(const uint32_t* round_keys, uint32_t rounds, const uint8_t* ciphertext, uint8_t* plaintext);

// Hardware acceleration detection and usage
int aes_hw_available(void);
void aes_hw_encrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* plaintext, uint8_t* ciphertext);
void aes_hw_decrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* ciphertext, uint8_t* plaintext);

// GCM mode helper functions
void aes_gcm_gf_mult(const uint8_t* a, const uint8_t* b, uint8_t* result);
void aes_gcm_ghash(const uint8_t* h, const uint8_t* data, uint32_t len, uint8_t* result);
void aes_gcm_inc32(uint8_t* block);

#endif
