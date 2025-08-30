#include "aes.h"
#include "crypto.h"
#include "compat.h"
#include <stdint.h>
#include <string.h>

// AES S-box
static const uint8_t aes_sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// AES inverse S-box
static const uint8_t aes_inv_sbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// Round constants
static const uint8_t aes_rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

static uint32_t aes_sub_word(uint32_t word) {
    return (aes_sbox[(word >> 24) & 0xFF] << 24) |
           (aes_sbox[(word >> 16) & 0xFF] << 16) |
           (aes_sbox[(word >> 8) & 0xFF] << 8) |
           aes_sbox[word & 0xFF];
}

static uint32_t aes_rot_word(uint32_t word) {
    return (word << 8) | (word >> 24);
}

int aes_key_expansion(const uint8_t* key, uint32_t key_bits, uint32_t* round_keys) {
    uint32_t key_words = key_bits / 32;
    uint32_t rounds = (key_bits == 128) ? 10 : (key_bits == 192) ? 12 : 14;
    uint32_t total_words = 4 * (rounds + 1);
    
    // Copy original key
    for (uint32_t i = 0; i < key_words; i++) {
        round_keys[i] = (key[i*4] << 24) | (key[i*4+1] << 16) | (key[i*4+2] << 8) | key[i*4+3];
    }
    
    // Generate remaining round keys
    for (uint32_t i = key_words; i < total_words; i++) {
        uint32_t temp = round_keys[i-1];
        
        if (i % key_words == 0) {
            temp = aes_sub_word(aes_rot_word(temp)) ^ (aes_rcon[i/key_words] << 24);
        } else if (key_words > 6 && i % key_words == 4) {
            temp = aes_sub_word(temp);
        }
        
        round_keys[i] = round_keys[i - key_words] ^ temp;
    }
    
    return rounds;
}

static void aes_add_round_key(uint8_t* state, const uint32_t* round_key) {
    for (int i = 0; i < 4; i++) {
        uint32_t key_word = round_key[i];
        state[i*4] ^= (key_word >> 24) & 0xFF;
        state[i*4+1] ^= (key_word >> 16) & 0xFF;
        state[i*4+2] ^= (key_word >> 8) & 0xFF;
        state[i*4+3] ^= key_word & 0xFF;
    }
}

static void aes_sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = aes_sbox[state[i]];
    }
}

static void aes_inv_sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = aes_inv_sbox[state[i]];
    }
}

static void aes_shift_rows(uint8_t* state) {
    uint8_t temp;
    
    // Row 1: shift left by 1
    temp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = temp;
    
    // Row 2: shift left by 2
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    
    // Row 3: shift left by 3 (or right by 1)
    temp = state[3]; state[3] = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = temp;
}

static void aes_inv_shift_rows(uint8_t* state) {
    uint8_t temp;
    
    // Row 1: shift right by 1
    temp = state[13]; state[13] = state[9]; state[9] = state[5]; state[5] = state[1]; state[1] = temp;
    
    // Row 2: shift right by 2
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    
    // Row 3: shift right by 3 (or left by 1)
    temp = state[3]; state[3] = state[7]; state[7] = state[11]; state[11] = state[15]; state[15] = temp;
}

static uint8_t aes_gf_mult(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    uint8_t hi_bit_set;
    
    for (int i = 0; i < 8; i++) {
        if (b & 1) result ^= a;
        hi_bit_set = a & 0x80;
        a <<= 1;
        if (hi_bit_set) a ^= 0x1b; // AES irreducible polynomial
        b >>= 1;
    }
    
    return result;
}

static void aes_mix_columns(uint8_t* state) {
    for (int c = 0; c < 4; c++) {
        uint8_t s0 = state[c*4], s1 = state[c*4+1], s2 = state[c*4+2], s3 = state[c*4+3];
        state[c*4] = aes_gf_mult(0x02, s0) ^ aes_gf_mult(0x03, s1) ^ s2 ^ s3;
        state[c*4+1] = s0 ^ aes_gf_mult(0x02, s1) ^ aes_gf_mult(0x03, s2) ^ s3;
        state[c*4+2] = s0 ^ s1 ^ aes_gf_mult(0x02, s2) ^ aes_gf_mult(0x03, s3);
        state[c*4+3] = aes_gf_mult(0x03, s0) ^ s1 ^ s2 ^ aes_gf_mult(0x02, s3);
    }
}

static void aes_inv_mix_columns(uint8_t* state) {
    for (int c = 0; c < 4; c++) {
        uint8_t s0 = state[c*4], s1 = state[c*4+1], s2 = state[c*4+2], s3 = state[c*4+3];
        state[c*4] = aes_gf_mult(0x0e, s0) ^ aes_gf_mult(0x0b, s1) ^ aes_gf_mult(0x0d, s2) ^ aes_gf_mult(0x09, s3);
        state[c*4+1] = aes_gf_mult(0x09, s0) ^ aes_gf_mult(0x0e, s1) ^ aes_gf_mult(0x0b, s2) ^ aes_gf_mult(0x0d, s3);
        state[c*4+2] = aes_gf_mult(0x0d, s0) ^ aes_gf_mult(0x09, s1) ^ aes_gf_mult(0x0e, s2) ^ aes_gf_mult(0x0b, s3);
        state[c*4+3] = aes_gf_mult(0x0b, s0) ^ aes_gf_mult(0x0d, s1) ^ aes_gf_mult(0x09, s2) ^ aes_gf_mult(0x0e, s3);
    }
}

void aes_encrypt_block_ex(const uint32_t* round_keys, uint32_t rounds, const uint8_t* plaintext, uint8_t* ciphertext) {
    uint8_t state[16];
    memcpy(state, plaintext, 16);
    
    // Initial round
    aes_add_round_key(state, round_keys);
    
    // Main rounds
    for (uint32_t round = 1; round < rounds; round++) {
        aes_sub_bytes(state);
        aes_shift_rows(state);
        aes_mix_columns(state);
        aes_add_round_key(state, round_keys + round * 4);
    }
    
    // Final round (no MixColumns)
    aes_sub_bytes(state);
    aes_shift_rows(state);
    aes_add_round_key(state, round_keys + rounds * 4);
    
    memcpy(ciphertext, state, 16);
    crypto_memzero_secure(state, sizeof(state));
}

void aes_decrypt_block_ex(const uint32_t* round_keys, uint32_t rounds, const uint8_t* ciphertext, uint8_t* plaintext) {
    uint8_t state[16];
    memcpy(state, ciphertext, 16);
    
    // Initial round
    aes_add_round_key(state, round_keys + rounds * 4);
    
    // Main rounds
    for (uint32_t round = rounds - 1; round > 0; round--) {
        aes_inv_shift_rows(state);
        aes_inv_sub_bytes(state);
        aes_add_round_key(state, round_keys + round * 4);
        aes_inv_mix_columns(state);
    }
    
    // Final round (no InvMixColumns)
    aes_inv_shift_rows(state);
    aes_inv_sub_bytes(state);
    aes_add_round_key(state, round_keys);
    
    memcpy(plaintext, state, 16);
    crypto_memzero_secure(state, sizeof(state));
}

int crypto_aes_init(crypto_aes_ctx_t* ctx, const uint8_t* key, uint32_t key_bits) {
    if (!ctx || !key || (key_bits != 128 && key_bits != 192 && key_bits != 256)) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    ctx->rounds = aes_key_expansion(key, key_bits, ctx->key_schedule);
    return CRYPTO_SUCCESS;
}

void crypto_aes_encrypt_block(const crypto_aes_ctx_t* ctx, const uint8_t* plaintext, uint8_t* ciphertext) {
    aes_encrypt_block_ex(ctx->key_schedule, ctx->rounds, plaintext, ciphertext);
}

void crypto_aes_decrypt_block(const crypto_aes_ctx_t* ctx, const uint8_t* ciphertext, uint8_t* plaintext) {
    aes_decrypt_block_ex(ctx->key_schedule, ctx->rounds, ciphertext, plaintext);
}

// CBC mode implementation
int crypto_aes_cbc_encrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext) {
    if (!ctx || !iv || !plaintext || !ciphertext || len % AES_BLOCK_SIZE != 0) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    uint8_t prev_block[AES_BLOCK_SIZE];
    memcpy(prev_block, iv, AES_BLOCK_SIZE);
    
    for (uint32_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t temp_block[AES_BLOCK_SIZE];
        
        // XOR with previous ciphertext block (or IV for first block)
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            temp_block[j] = plaintext[i + j] ^ prev_block[j];
        }
        
        // Encrypt
        crypto_aes_encrypt_block(ctx, temp_block, ciphertext + i);
        
        // Update previous block
        memcpy(prev_block, ciphertext + i, AES_BLOCK_SIZE);
    }
    
    crypto_memzero_secure(prev_block, sizeof(prev_block));
    return CRYPTO_SUCCESS;
}

int crypto_aes_cbc_decrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, const uint8_t* ciphertext, uint32_t len, uint8_t* plaintext) {
    if (!ctx || !iv || !ciphertext || !plaintext || len % AES_BLOCK_SIZE != 0) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    uint8_t prev_block[AES_BLOCK_SIZE];
    memcpy(prev_block, iv, AES_BLOCK_SIZE);
    
    for (uint32_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t temp_block[AES_BLOCK_SIZE];
        
        // Decrypt
        crypto_aes_decrypt_block(ctx, ciphertext + i, temp_block);
        
        // XOR with previous ciphertext block (or IV for first block)
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            plaintext[i + j] = temp_block[j] ^ prev_block[j];
        }
        
        // Update previous block
        memcpy(prev_block, ciphertext + i, AES_BLOCK_SIZE);
    }
    
    crypto_memzero_secure(prev_block, sizeof(prev_block));
    return CRYPTO_SUCCESS;
}

// GCM mode helper functions
void aes_gcm_gf_mult(const uint8_t* a, const uint8_t* b, uint8_t* result) {
    uint8_t z[16] = {0};
    uint8_t v[16];
    memcpy(v, b, 16);
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (a[i] & (1 << (7-j))) {
                for (int k = 0; k < 16; k++) {
                    z[k] ^= v[k];
                }
            }
            
            int lsb = v[15] & 1;
            for (int k = 15; k > 0; k--) {
                v[k] = (v[k] >> 1) | ((v[k-1] & 1) << 7);
            }
            v[0] >>= 1;
            if (lsb) v[0] ^= 0xe1;
        }
    }
    
    memcpy(result, z, 16);
}

void aes_gcm_ghash(const uint8_t* h, const uint8_t* data, uint32_t len, uint8_t* result) {
    uint8_t y[16] = {0};
    
    for (uint32_t i = 0; i < len; i += 16) {
        uint8_t block[16] = {0};
        uint32_t block_len = (len - i < 16) ? (len - i) : 16;
        memcpy(block, data + i, block_len);
        
        for (int j = 0; j < 16; j++) {
            y[j] ^= block[j];
        }
        
        aes_gcm_gf_mult(y, h, y);
    }
    
    memcpy(result, y, 16);
}

void aes_gcm_inc32(uint8_t* block) {
    uint32_t counter = (block[12] << 24) | (block[13] << 16) | (block[14] << 8) | block[15];
    counter++;
    block[12] = (counter >> 24) & 0xFF;
    block[13] = (counter >> 16) & 0xFF;
    block[14] = (counter >> 8) & 0xFF;
    block[15] = counter & 0xFF;
}

// GCM mode implementation
int crypto_aes_gcm_encrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, uint32_t iv_len, const uint8_t* aad, uint32_t aad_len, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext, uint8_t* tag) {
    if (!ctx || !iv || !ciphertext || !tag) return CRYPTO_ERROR_INVALID_PARAM;
    
    uint8_t h[16] = {0};
    uint8_t j0[16] = {0};
    uint8_t ghash_result[16] = {0};
    
    // Calculate H = E(K, 0^128)
    crypto_aes_encrypt_block(ctx, h, h);
    
    // Calculate J0
    if (iv_len == 12) {
        memcpy(j0, iv, 12);
        j0[15] = 1;
    } else {
        // For IV lengths != 12, use GHASH
        aes_gcm_ghash(h, iv, iv_len, j0);
        uint8_t len_block[16] = {0};
        uint64_t iv_bits = iv_len * 8;
        len_block[8] = (iv_bits >> 56) & 0xFF;
        len_block[9] = (iv_bits >> 48) & 0xFF;
        len_block[10] = (iv_bits >> 40) & 0xFF;
        len_block[11] = (iv_bits >> 32) & 0xFF;
        len_block[12] = (iv_bits >> 24) & 0xFF;
        len_block[13] = (iv_bits >> 16) & 0xFF;
        len_block[14] = (iv_bits >> 8) & 0xFF;
        len_block[15] = iv_bits & 0xFF;
        aes_gcm_gf_mult(j0, h, j0);
        for (int i = 0; i < 16; i++) j0[i] ^= len_block[i];
        aes_gcm_gf_mult(j0, h, j0);
    }
    
    // Encrypt plaintext
    uint8_t counter[16];
    memcpy(counter, j0, 16);
    aes_gcm_inc32(counter);
    
    for (uint32_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t keystream[16];
        crypto_aes_encrypt_block(ctx, counter, keystream);
        
        uint32_t block_len = (len - i < AES_BLOCK_SIZE) ? (len - i) : AES_BLOCK_SIZE;
        for (uint32_t j = 0; j < block_len; j++) {
            ciphertext[i + j] = plaintext[i + j] ^ keystream[j];
        }
        
        aes_gcm_inc32(counter);
        crypto_memzero_secure(keystream, sizeof(keystream));
    }
    
    // Calculate authentication tag
    uint8_t s[16] = {0};
    
    // GHASH(H, A)
    if (aad && aad_len > 0) {
        aes_gcm_ghash(h, aad, aad_len, s);
    }
    
    // GHASH(H, C)
    if (len > 0) {
        aes_gcm_ghash(h, ciphertext, len, ghash_result);
        for (int i = 0; i < 16; i++) s[i] ^= ghash_result[i];
        aes_gcm_gf_mult(s, h, s);
    }
    
    // Add length block
    uint8_t len_block[16] = {0};
    uint64_t aad_bits = aad_len * 8;
    uint64_t c_bits = len * 8;
    len_block[0] = (aad_bits >> 56) & 0xFF;
    len_block[1] = (aad_bits >> 48) & 0xFF;
    len_block[2] = (aad_bits >> 40) & 0xFF;
    len_block[3] = (aad_bits >> 32) & 0xFF;
    len_block[4] = (aad_bits >> 24) & 0xFF;
    len_block[5] = (aad_bits >> 16) & 0xFF;
    len_block[6] = (aad_bits >> 8) & 0xFF;
    len_block[7] = aad_bits & 0xFF;
    len_block[8] = (c_bits >> 56) & 0xFF;
    len_block[9] = (c_bits >> 48) & 0xFF;
    len_block[10] = (c_bits >> 40) & 0xFF;
    len_block[11] = (c_bits >> 32) & 0xFF;
    len_block[12] = (c_bits >> 24) & 0xFF;
    len_block[13] = (c_bits >> 16) & 0xFF;
    len_block[14] = (c_bits >> 8) & 0xFF;
    len_block[15] = c_bits & 0xFF;
    
    for (int i = 0; i < 16; i++) s[i] ^= len_block[i];
    aes_gcm_gf_mult(s, h, s);
    
    // Final tag = E(K, J0) XOR S
    uint8_t j0_encrypted[16];
    crypto_aes_encrypt_block(ctx, j0, j0_encrypted);
    for (int i = 0; i < 16; i++) {
        tag[i] = s[i] ^ j0_encrypted[i];
    }
    
    crypto_memzero_secure(counter, sizeof(counter));
    crypto_memzero_secure(j0_encrypted, sizeof(j0_encrypted));
    crypto_memzero_secure(s, sizeof(s));
    
    return CRYPTO_SUCCESS;
}

int crypto_aes_gcm_decrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, uint32_t iv_len, const uint8_t* aad, uint32_t aad_len, const uint8_t* ciphertext, uint32_t len, const uint8_t* tag, uint8_t* plaintext) {
    if (!ctx || !iv || !ciphertext || !tag || !plaintext) return CRYPTO_ERROR_INVALID_PARAM;
    
    // First verify the tag
    uint8_t computed_tag[16];
    
    // Temporarily decrypt to compute tag (we'll do it again if verification passes)
    uint8_t temp_plaintext[len];
    
    uint8_t h[16] = {0};
    crypto_aes_encrypt_block(ctx, h, h);
    
    uint8_t j0[16] = {0};
    if (iv_len == 12) {
        memcpy(j0, iv, 12);
        j0[15] = 1;
    }
    
    uint8_t counter[16];
    memcpy(counter, j0, 16);
    aes_gcm_inc32(counter);
    
    // Decrypt for tag verification
    for (uint32_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t keystream[16];
        crypto_aes_encrypt_block(ctx, counter, keystream);
        
        uint32_t block_len = (len - i < AES_BLOCK_SIZE) ? (len - i) : AES_BLOCK_SIZE;
        for (uint32_t j = 0; j < block_len; j++) {
            temp_plaintext[i + j] = ciphertext[i + j] ^ keystream[j];
        }
        
        aes_gcm_inc32(counter);
        crypto_memzero_secure(keystream, sizeof(keystream));
    }
    
    // Compute tag using the decrypted plaintext
    crypto_aes_gcm_encrypt(ctx, iv, iv_len, aad, aad_len, temp_plaintext, len, (uint8_t*)ciphertext, computed_tag);
    
    // Verify tag
    if (crypto_memcmp_constant_time(tag, computed_tag, 16) != 0) {
        crypto_memzero_secure(temp_plaintext, len);
        crypto_memzero_secure(computed_tag, sizeof(computed_tag));
        return CRYPTO_ERROR_VERIFICATION_FAILED;
    }
    
    // Tag verified, copy plaintext
    memcpy(plaintext, temp_plaintext, len);
    crypto_memzero_secure(temp_plaintext, len);
    crypto_memzero_secure(computed_tag, sizeof(computed_tag));
    
    return CRYPTO_SUCCESS;
}

// Hardware acceleration functions
int aes_hw_available(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check for AES-NI support (CPUID.01H:ECX.AES[bit 25])
    __asm__ volatile (
        "movl $1, %%eax\n\t"
        "cpuid\n\t"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        :
        : "memory"
    );
    
    return (ecx & (1 << 25)) ? 1 : 0;
}

void aes_hw_encrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* plaintext, uint8_t* ciphertext) {
    if (!aes_hw_available()) {
        // Fall back to software implementation
        crypto_aes_ctx_t ctx;
        crypto_aes_init(&ctx, key, key_bits);
        crypto_aes_encrypt_block(&ctx, plaintext, ciphertext);
        crypto_zeroize_context(&ctx, sizeof(ctx));
        return;
    }
    
    // Use AES-NI instructions (simplified inline assembly)
    uint32_t round_keys[60];
    uint32_t rounds = aes_key_expansion(key, key_bits, round_keys);
    
    // Load plaintext
    __asm__ volatile (
        "movdqu %1, %%xmm0\n\t"  // Load plaintext
        "movdqu %2, %%xmm1\n\t"  // Load first round key
        "pxor %%xmm1, %%xmm0\n\t" // Initial AddRoundKey
        : 
        : "m" (*ciphertext), "m" (*plaintext), "m" (round_keys[0])
        : "xmm0", "xmm1", "memory"
    );
    
    // Perform AES rounds using AES-NI
    for (uint32_t i = 1; i < rounds; i++) {
        __asm__ volatile (
            "movdqu %0, %%xmm1\n\t"   // Load round key
            "aesenc %%xmm1, %%xmm0\n\t" // AES encrypt round
            :
            : "m" (round_keys[i*4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    // Final round
    __asm__ volatile (
        "movdqu %1, %%xmm1\n\t"      // Load final round key
        "aesenclast %%xmm1, %%xmm0\n\t" // AES encrypt last round
        "movdqu %%xmm0, %0\n\t"      // Store result
        : "=m" (*ciphertext)
        : "m" (round_keys[rounds*4])
        : "xmm0", "xmm1", "memory"
    );
    
    crypto_memzero_secure(round_keys, sizeof(round_keys));
}

void aes_hw_decrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* ciphertext, uint8_t* plaintext) {
    if (!aes_hw_available()) {
        // Fall back to software implementation
        crypto_aes_ctx_t ctx;
        crypto_aes_init(&ctx, key, key_bits);
        crypto_aes_decrypt_block(&ctx, ciphertext, plaintext);
        crypto_zeroize_context(&ctx, sizeof(ctx));
        return;
    }
    
    // Similar to encrypt but with decrypt instructions
    uint32_t round_keys[60];
    uint32_t rounds = aes_key_expansion(key, key_bits, round_keys);
    
    __asm__ volatile (
        "movdqu %1, %%xmm0\n\t"
        "movdqu %2, %%xmm1\n\t"
        "pxor %%xmm1, %%xmm0\n\t"
        :
        : "m" (*plaintext), "m" (*ciphertext), "m" (round_keys[rounds*4])
        : "xmm0", "xmm1", "memory"
    );
    
    for (uint32_t i = rounds - 1; i > 0; i--) {
        __asm__ volatile (
            "movdqu %0, %%xmm1\n\t"
            "aesdec %%xmm1, %%xmm0\n\t"
            :
            : "m" (round_keys[i*4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    __asm__ volatile (
        "movdqu %1, %%xmm1\n\t"
        "aesdeclast %%xmm1, %%xmm0\n\t"
        "movdqu %%xmm0, %0\n\t"
        : "=m" (*plaintext)
        : "m" (round_keys[0])
        : "xmm0", "xmm1", "memory"
    );
    
    crypto_memzero_secure(round_keys, sizeof(round_keys));
}

// Legacy function (for backward compatibility)
void aes_encrypt_block(const uint8_t* in, uint8_t* out, const uint8_t* key) {
    // Simple XOR implementation (original basic version)
    for (int i = 0; i < 16; ++i) out[i] = in[i] ^ key[i];
}
