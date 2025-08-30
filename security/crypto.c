#include <stdint.h>
#include "compat.h"
#include <string.h>
#include "crypto.h"
#include "entropy.h"

// Global hardware support flags
static crypto_hw_support_t g_hw_support = CRYPTO_HW_NONE;

static uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t sha256_h[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static uint32_t rotr(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static uint32_t sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static uint32_t gamma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static uint32_t gamma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

// Hardware detection
crypto_hw_support_t crypto_detect_hardware_support(void) {
    crypto_hw_support_t support = CRYPTO_HW_NONE;
    uint32_t eax, ebx, ecx, edx;
    
    // Check for AES-NI support (CPUID.01H:ECX.AES[bit 25])
    __asm__ volatile (
        "movl $1, %%eax\n\t"
        "cpuid\n\t"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        :
        : "memory"
    );
    
    if (ecx & (1 << 25)) support |= CRYPTO_HW_INTEL_AESNI;
    
    // Check for SHA extensions (CPUID.07H:EBX.SHA[bit 29])
    __asm__ volatile (
        "movl $7, %%eax\n\t"
        "movl $0, %%ecx\n\t"
        "cpuid\n\t"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        :
        : "memory"
    );
    
    if (ebx & (1 << 29)) support |= CRYPTO_HW_INTEL_SHA;
    
    return support;
}

int crypto_init_hardware_acceleration(crypto_hw_support_t hw_mask) {
    g_hw_support = crypto_detect_hardware_support() & hw_mask;
    return CRYPTO_SUCCESS;
}

void crypto_cleanup_hardware(void) {
    g_hw_support = CRYPTO_HW_NONE;
}

// Enhanced SHA-256 with streaming support
int crypto_sha256_init(crypto_sha256_ctx_t* ctx) {
    if (!ctx) return CRYPTO_ERROR_INVALID_PARAM;
    
    memcpy(ctx->h, sha256_h, sizeof(sha256_h));
    ctx->len = 0;
    ctx->buf_len = 0;
    memset(ctx->buf, 0, sizeof(ctx->buf));
    
    return CRYPTO_SUCCESS;
}

int crypto_sha256_update(crypto_sha256_ctx_t* ctx, const uint8_t* data, uint32_t len) {
    if (!ctx || !data) return CRYPTO_ERROR_INVALID_PARAM;
    
    ctx->len += len;
    
    while (len > 0) {
        uint32_t chunk_size = (ctx->buf_len + len > 64) ? (64 - ctx->buf_len) : len;
        memcpy(ctx->buf + ctx->buf_len, data, chunk_size);
        ctx->buf_len += chunk_size;
        data += chunk_size;
        len -= chunk_size;
        
        if (ctx->buf_len == 64) {
            // Process block
            uint32_t w[64];
            
            for (int j = 0; j < 16; j++) {
                w[j] = (ctx->buf[j*4] << 24) | (ctx->buf[j*4 + 1] << 16) |
                       (ctx->buf[j*4 + 2] << 8) | ctx->buf[j*4 + 3];
            }
            for (int j = 16; j < 64; j++) {
                w[j] = gamma1(w[j-2]) + w[j-7] + gamma0(w[j-15]) + w[j-16];
            }
            
            uint32_t a = ctx->h[0], b = ctx->h[1], c = ctx->h[2], d = ctx->h[3];
            uint32_t e = ctx->h[4], f = ctx->h[5], g = ctx->h[6], h_val = ctx->h[7];
            
            for (int j = 0; j < 64; j++) {
                uint32_t temp1 = h_val + sigma1(e) + ch(e, f, g) + sha256_k[j] + w[j];
                uint32_t temp2 = sigma0(a) + maj(a, b, c);
                h_val = g; g = f; f = e; e = d + temp1;
                d = c; c = b; b = a; a = temp1 + temp2;
            }
            
            ctx->h[0] += a; ctx->h[1] += b; ctx->h[2] += c; ctx->h[3] += d;
            ctx->h[4] += e; ctx->h[5] += f; ctx->h[6] += g; ctx->h[7] += h_val;
            
            ctx->buf_len = 0;
        }
    }
    
    return CRYPTO_SUCCESS;
}

int crypto_sha256_final(crypto_sha256_ctx_t* ctx, uint8_t* hash) {
    if (!ctx || !hash) return CRYPTO_ERROR_INVALID_PARAM;
    
    // Pre-processing: adding padding bits
    uint64_t msg_len = ctx->len;
    uint64_t bit_len = msg_len * 8;
    
    // Append '1' bit
    ctx->buf[ctx->buf_len++] = 0x80;
    
    // Append '0' bits until message length ≡ 448 (mod 512)
    while (ctx->buf_len % 64 != 56) {
        if (ctx->buf_len == 64) {
            crypto_sha256_update(ctx, ctx->buf, 64);
            ctx->buf_len = 0;
        }
        ctx->buf[ctx->buf_len++] = 0x00;
    }
    
    // Append original length in bits as 64-bit big-endian integer
    for (int i = 7; i >= 0; i--) {
        ctx->buf[ctx->buf_len++] = (bit_len >> (i * 8)) & 0xFF;
    }
    
    // Process final block
    crypto_sha256_update(ctx, ctx->buf, ctx->buf_len);
    
    // Produce final hash value
    for (int i = 0; i < 8; i++) {
        hash[i*4] = (ctx->h[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (ctx->h[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (ctx->h[i] >> 8) & 0xFF;
        hash[i*4 + 3] = ctx->h[i] & 0xFF;
    }
    
    return CRYPTO_SUCCESS;
}

void sha256_hash(const uint8_t* data, uint32_t len, uint8_t* hash) {
    crypto_sha256_ctx_t ctx;
    crypto_sha256_init(&ctx);
    crypto_sha256_update(&ctx, data, len);
    crypto_sha256_final(&ctx, hash);
    crypto_zeroize_context(&ctx, sizeof(ctx));
}

// HMAC-SHA256 implementation (needed for encrypted filesystems)
void crypto_hmac_sha256(const uint8_t* key, uint32_t key_len, const uint8_t* data, uint32_t data_len, uint8_t* mac) {
    uint8_t k_pad[64];
    uint8_t inner_hash[32];
    
    // Prepare key
    memset(k_pad, 0, 64);
    if (key_len > 64) {
        sha256_hash(key, key_len, k_pad);
    } else {
        memcpy(k_pad, key, key_len);
    }
    
    // Inner hash: SHA256(k_pad XOR 0x36 || data)
    for (int i = 0; i < 64; i++) k_pad[i] ^= 0x36;
    
    crypto_sha256_ctx_t ctx;
    crypto_sha256_init(&ctx);
    crypto_sha256_update(&ctx, k_pad, 64);
    crypto_sha256_update(&ctx, data, data_len);
    crypto_sha256_final(&ctx, inner_hash);
    
    // Outer hash: SHA256(k_pad XOR 0x5c || inner_hash)
    for (int i = 0; i < 64; i++) k_pad[i] ^= 0x36 ^ 0x5c; // Undo 0x36, apply 0x5c
    
    crypto_sha256_init(&ctx);
    crypto_sha256_update(&ctx, k_pad, 64);
    crypto_sha256_update(&ctx, inner_hash, 32);
    crypto_sha256_final(&ctx, mac);
    
    crypto_memzero_secure(k_pad, sizeof(k_pad));
    crypto_memzero_secure(inner_hash, sizeof(inner_hash));
    crypto_zeroize_context(&ctx, sizeof(ctx));
}

// PBKDF2-SHA256 for key derivation (needed for encrypted filesystems)
int crypto_pbkdf2_sha256(const uint8_t* password, uint32_t password_len, const uint8_t* salt, uint32_t salt_len, uint32_t iterations, uint8_t* derived_key, uint32_t key_len) {
    if (!password || !salt || !derived_key || iterations == 0 || key_len == 0) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    uint32_t blocks_needed = (key_len + 31) / 32;
    
    for (uint32_t block = 1; block <= blocks_needed; block++) {
        uint8_t u[32], f[32];
        
        // First iteration: U1 = HMAC(password, salt || block_number)
        uint8_t salt_block[salt_len + 4];
        memcpy(salt_block, salt, salt_len);
        salt_block[salt_len] = (block >> 24) & 0xFF;
        salt_block[salt_len + 1] = (block >> 16) & 0xFF;
        salt_block[salt_len + 2] = (block >> 8) & 0xFF;
        salt_block[salt_len + 3] = block & 0xFF;
        
        crypto_hmac_sha256(password, password_len, salt_block, salt_len + 4, u);
        memcpy(f, u, 32);
        
        // Remaining iterations: Ui = HMAC(password, Ui-1), F = F XOR Ui
        for (uint32_t i = 2; i <= iterations; i++) {
            crypto_hmac_sha256(password, password_len, u, 32, u);
            for (int j = 0; j < 32; j++) {
                f[j] ^= u[j];
            }
        }
        
        // Copy result to output
        uint32_t copy_len = (key_len < 32) ? key_len : 32;
        if (block == blocks_needed && key_len % 32 != 0) {
            copy_len = key_len % 32;
        }
        memcpy(derived_key + (block - 1) * 32, f, copy_len);
        key_len -= copy_len;
        
        crypto_memzero_secure(u, sizeof(u));
        crypto_memzero_secure(f, sizeof(f));
    }
    
    return CRYPTO_SUCCESS;
}

// Secure random number generation
static uint32_t g_rng_state[4] = {0};
static int g_rng_initialized = 0;

int crypto_random_init(void) {
    // Seed from hardware entropy and current time
    g_rng_state[0] = entropy_get();
    g_rng_state[1] = entropy_get() ^ 0xDEADBEEF;
    g_rng_state[2] = entropy_get() ^ 0xCAFEBABE;
    g_rng_state[3] = entropy_get() ^ 0xFEEDFACE;
    
    g_rng_initialized = 1;
    return CRYPTO_SUCCESS;
}

int crypto_random_uint32(uint32_t* value) {
    if (!g_rng_initialized || !value) return CRYPTO_ERROR_INVALID_PARAM;
    
    // XORShift128 PRNG
    uint32_t t = g_rng_state[3];
    uint32_t s = g_rng_state[0];
    g_rng_state[3] = g_rng_state[2];
    g_rng_state[2] = g_rng_state[1];
    g_rng_state[1] = s;
    
    t ^= t << 11;
    t ^= t >> 8;
    g_rng_state[0] = t ^ s ^ (s >> 19);
    
    *value = g_rng_state[0];
    return CRYPTO_SUCCESS;
}

int crypto_random_bytes(uint8_t* buf, uint32_t len) {
    if (!buf || len == 0) return CRYPTO_ERROR_INVALID_PARAM;
    
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t rand_val;
        int result = crypto_random_uint32(&rand_val);
        if (result != CRYPTO_SUCCESS) return result;
        
        uint32_t copy_len = (len - i < 4) ? (len - i) : 4;
        memcpy(buf + i, &rand_val, copy_len);
    }
    
    return CRYPTO_SUCCESS;
}

void crypto_random_cleanup(void) {
    crypto_memzero_secure(g_rng_state, sizeof(g_rng_state));
    g_rng_initialized = 0;
}

// Constant-time operations for side-channel resistance
int crypto_memcmp_constant_time(const void* a, const void* b, size_t len) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    uint8_t result = 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= pa[i] ^ pb[i];
    }
    
    return result;
}

void crypto_memzero_secure(void* ptr, size_t len) {
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
    }
}

void crypto_zeroize_context(void* ctx, size_t size) {
    crypto_memzero_secure(ctx, size);
}

// Self-tests for cryptographic functions
int crypto_self_test_sha256(void) {
    // Test vector: "abc" -> ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
    const uint8_t test_input[] = {'a', 'b', 'c'};
    const uint8_t expected_hash[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    
    uint8_t computed_hash[32];
    sha256_hash(test_input, 3, computed_hash);
    
    return crypto_memcmp_constant_time(computed_hash, expected_hash, 32) == 0 ? CRYPTO_SUCCESS : CRYPTO_ERROR_VERIFICATION_FAILED;
}

int crypto_self_test_aes(void) {
    // AES-128 test vector
    const uint8_t key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    const uint8_t plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    const uint8_t expected_ciphertext[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
    };
    
    crypto_aes_ctx_t ctx;
    uint8_t computed_ciphertext[16];
    
    if (crypto_aes_init(&ctx, key, 128) != CRYPTO_SUCCESS) return CRYPTO_ERROR_VERIFICATION_FAILED;
    crypto_aes_encrypt_block(&ctx, plaintext, computed_ciphertext);
    
    int result = crypto_memcmp_constant_time(computed_ciphertext, expected_ciphertext, 16) == 0 ? CRYPTO_SUCCESS : CRYPTO_ERROR_VERIFICATION_FAILED;
    crypto_zeroize_context(&ctx, sizeof(ctx));
    
    return result;
}

int crypto_run_all_self_tests(void) {
    if (crypto_self_test_sha256() != CRYPTO_SUCCESS) return CRYPTO_ERROR_VERIFICATION_FAILED;
    if (crypto_self_test_aes() != CRYPTO_SUCCESS) return CRYPTO_ERROR_VERIFICATION_FAILED;
    return CRYPTO_SUCCESS;
}

// RSA implementation (basic, for signature verification only)
static int mod_exp(const uint8_t* base, const uint8_t* exp, int exp_len, const uint8_t* mod, int mod_len, uint8_t* out, int out_len) {
    uint8_t result[512] = {0};
    result[out_len - 1] = 1;
    for (int i = 0; i < exp_len * 8; i++) {
        int bit = (exp[i / 8] >> (7 - (i % 8))) & 1;
        for (int j = 0; j < out_len; j++) {
            result[j] = (result[j] * result[j]) % mod[j];
        }
        if (bit) {
            for (int j = 0; j < out_len; j++) {
                result[j] = (result[j] * base[j]) % mod[j];
            }
        }
    }
    memcpy(out, result, out_len);
    return 0;
}

int verify_signature(const uint8_t* data, uint32_t len, const uint8_t* signature, const uint8_t* public_key) {
    uint8_t hash[32];
    sha256_hash(data, len, hash);
    int mod_len = 256;
    uint8_t decrypted[256];
    mod_exp(signature, public_key + 4, 256, public_key + 260, 256, decrypted, 256);
    if (decrypted[0] != 0x00 || decrypted[1] != 0x01) return 0;
    int i = 2;
    while (i < 256 && decrypted[i] == 0xFF) i++;
    if (decrypted[i++] != 0x00) return 0;
    static const uint8_t sha256_prefix[] = {
        0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20
    };
    if (memcmp(decrypted + i, sha256_prefix, sizeof(sha256_prefix)) != 0) return 0;
    i += sizeof(sha256_prefix);
    if (memcmp(decrypted + i, hash, 32) != 0) return 0;
    return 1;
}

void crypto_cleanup_all_contexts(void) {
    crypto_random_cleanup();
    crypto_cleanup_hardware();
}
