#ifndef BLOODHORN_CRYPTO_H
#define BLOODHORN_CRYPTO_H
#include <stdint.h>
#include <stddef.h>
#include "compat.h"

// Cryptographic Constants
#define CRYPTO_SHA256_DIGEST_LENGTH     32
#define CRYPTO_SHA512_DIGEST_LENGTH     64
#define CRYPTO_AES128_KEY_LENGTH        16
#define CRYPTO_AES256_KEY_LENGTH        32
#define CRYPTO_RSA2048_KEY_LENGTH       256
#define CRYPTO_RSA4096_KEY_LENGTH       512
#define CRYPTO_ECDSA_P256_KEY_LENGTH    32
#define CRYPTO_ECDSA_P384_KEY_LENGTH    48
#define CRYPTO_ECDSA_P521_KEY_LENGTH    66
#define CRYPTO_CHACHA20_KEY_LENGTH      32
#define CRYPTO_POLY1305_KEY_LENGTH      32
#define CRYPTO_HMAC_MAX_KEY_LENGTH      128

// Error codes
#define CRYPTO_SUCCESS                  0
#define CRYPTO_ERROR_INVALID_PARAM      -1
#define CRYPTO_ERROR_BUFFER_TOO_SMALL   -2
#define CRYPTO_ERROR_VERIFICATION_FAILED -3
#define CRYPTO_ERROR_NOT_SUPPORTED      -4
#define CRYPTO_ERROR_HARDWARE_UNAVAILABLE -5

// Hardware acceleration support
typedef enum {
    CRYPTO_HW_NONE = 0,
    CRYPTO_HW_INTEL_AESNI = 1,
    CRYPTO_HW_ARM_CRYPTO = 2,
    CRYPTO_HW_AMD_SVM = 4,
    CRYPTO_HW_INTEL_SHA = 8
} crypto_hw_support_t;

// Cryptographic context structures
typedef struct {
    uint32_t h[8];
    uint64_t len;
    uint8_t buf[64];
    uint32_t buf_len;
} crypto_sha256_ctx_t;

typedef struct {
    uint64_t h[8];
    uint64_t len;
    uint8_t buf[128];
    uint32_t buf_len;
} crypto_sha512_ctx_t;

typedef struct {
    uint32_t key_schedule[60];
    uint32_t rounds;
} crypto_aes_ctx_t;

typedef struct {
    uint8_t key[CRYPTO_CHACHA20_KEY_LENGTH];
    uint8_t nonce[12];
    uint32_t counter;
} crypto_chacha20_ctx_t;

typedef struct {
    uint32_t r[5];
    uint32_t h[5];
    uint8_t buf[16];
    uint32_t buf_len;
} crypto_poly1305_ctx_t;

typedef struct {
    uint8_t key[CRYPTO_HMAC_MAX_KEY_LENGTH];
    uint32_t key_len;
    crypto_sha256_ctx_t inner_ctx;
    crypto_sha256_ctx_t outer_ctx;
} crypto_hmac_sha256_ctx_t;

// RSA key structures
typedef struct {
    uint8_t n[CRYPTO_RSA4096_KEY_LENGTH];  // Modulus
    uint8_t e[4];                          // Public exponent (usually 65537)
    uint32_t key_bits;                     // Key size in bits (2048 or 4096)
} crypto_rsa_public_key_t;

typedef struct {
    crypto_rsa_public_key_t public;
    uint8_t d[CRYPTO_RSA4096_KEY_LENGTH];  // Private exponent
    uint8_t p[CRYPTO_RSA4096_KEY_LENGTH/2]; // Prime 1
    uint8_t q[CRYPTO_RSA4096_KEY_LENGTH/2]; // Prime 2
    uint8_t dp[CRYPTO_RSA4096_KEY_LENGTH/2]; // d mod (p-1)
    uint8_t dq[CRYPTO_RSA4096_KEY_LENGTH/2]; // d mod (q-1)
    uint8_t qinv[CRYPTO_RSA4096_KEY_LENGTH/2]; // q^-1 mod p
} crypto_rsa_private_key_t;

// ECDSA key structures
typedef struct {
    uint8_t x[CRYPTO_ECDSA_P521_KEY_LENGTH];
    uint8_t y[CRYPTO_ECDSA_P521_KEY_LENGTH];
    uint32_t curve_type; // 256, 384, or 521
} crypto_ecdsa_public_key_t;

typedef struct {
    crypto_ecdsa_public_key_t public;
    uint8_t d[CRYPTO_ECDSA_P521_KEY_LENGTH]; // Private scalar
} crypto_ecdsa_private_key_t;

// ECDSA signature
typedef struct {
    uint8_t r[CRYPTO_ECDSA_P521_KEY_LENGTH];
    uint8_t s[CRYPTO_ECDSA_P521_KEY_LENGTH];
} crypto_ecdsa_signature_t;

// Function declarations

// Hardware detection and initialization
crypto_hw_support_t crypto_detect_hardware_support(void);
int crypto_init_hardware_acceleration(crypto_hw_support_t hw_mask);
void crypto_cleanup_hardware(void);

// Hash functions
void sha256_hash(const uint8_t* data, uint32_t len, uint8_t* hash);
void sha512_hash(const uint8_t* data, uint32_t len, uint8_t* hash);
void sha3_256_hash(const uint8_t* data, uint32_t len, uint8_t* hash);
void blake2b_hash(const uint8_t* data, uint32_t len, uint8_t* hash, uint32_t hash_len);

// Streaming hash functions
int crypto_sha256_init(crypto_sha256_ctx_t* ctx);
int crypto_sha256_update(crypto_sha256_ctx_t* ctx, const uint8_t* data, uint32_t len);
int crypto_sha256_final(crypto_sha256_ctx_t* ctx, uint8_t* hash);

int crypto_sha512_init(crypto_sha512_ctx_t* ctx);
int crypto_sha512_update(crypto_sha512_ctx_t* ctx, const uint8_t* data, uint32_t len);
int crypto_sha512_final(crypto_sha512_ctx_t* ctx, uint8_t* hash);

// HMAC functions
int crypto_hmac_sha256_init(crypto_hmac_sha256_ctx_t* ctx, const uint8_t* key, uint32_t key_len);
int crypto_hmac_sha256_update(crypto_hmac_sha256_ctx_t* ctx, const uint8_t* data, uint32_t len);
int crypto_hmac_sha256_final(crypto_hmac_sha256_ctx_t* ctx, uint8_t* mac);
void crypto_hmac_sha256(const uint8_t* key, uint32_t key_len, const uint8_t* data, uint32_t data_len, uint8_t* mac);
void crypto_hmac_sha512(const uint8_t* key, uint32_t key_len, const uint8_t* data, uint32_t data_len, uint8_t* mac);

// Symmetric encryption
int crypto_aes_init(crypto_aes_ctx_t* ctx, const uint8_t* key, uint32_t key_bits);
void crypto_aes_encrypt_block(const crypto_aes_ctx_t* ctx, const uint8_t* plaintext, uint8_t* ciphertext);
void crypto_aes_decrypt_block(const crypto_aes_ctx_t* ctx, const uint8_t* ciphertext, uint8_t* plaintext);

// AES modes of operation
int crypto_aes_cbc_encrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext);
int crypto_aes_cbc_decrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, const uint8_t* ciphertext, uint32_t len, uint8_t* plaintext);
int crypto_aes_gcm_encrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, uint32_t iv_len, const uint8_t* aad, uint32_t aad_len, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext, uint8_t* tag);
int crypto_aes_gcm_decrypt(const crypto_aes_ctx_t* ctx, const uint8_t* iv, uint32_t iv_len, const uint8_t* aad, uint32_t aad_len, const uint8_t* ciphertext, uint32_t len, const uint8_t* tag, uint8_t* plaintext);
int crypto_aes_xts_encrypt(const crypto_aes_ctx_t* ctx1, const crypto_aes_ctx_t* ctx2, const uint8_t* tweak, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext);
int crypto_aes_xts_decrypt(const crypto_aes_ctx_t* ctx1, const crypto_aes_ctx_t* ctx2, const uint8_t* tweak, const uint8_t* ciphertext, uint32_t len, uint8_t* plaintext);

// ChaCha20-Poly1305 AEAD
int crypto_chacha20_init(crypto_chacha20_ctx_t* ctx, const uint8_t* key, const uint8_t* nonce);
void crypto_chacha20_encrypt(crypto_chacha20_ctx_t* ctx, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext);
void crypto_chacha20_decrypt(crypto_chacha20_ctx_t* ctx, const uint8_t* ciphertext, uint32_t len, uint8_t* plaintext);

int crypto_poly1305_init(crypto_poly1305_ctx_t* ctx, const uint8_t* key);
int crypto_poly1305_update(crypto_poly1305_ctx_t* ctx, const uint8_t* data, uint32_t len);
int crypto_poly1305_final(crypto_poly1305_ctx_t* ctx, uint8_t* tag);

int crypto_chacha20_poly1305_encrypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* aad, uint32_t aad_len, const uint8_t* plaintext, uint32_t len, uint8_t* ciphertext, uint8_t* tag);
int crypto_chacha20_poly1305_decrypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* aad, uint32_t aad_len, const uint8_t* ciphertext, uint32_t len, const uint8_t* tag, uint8_t* plaintext);

// RSA operations
int crypto_rsa_generate_keypair(crypto_rsa_private_key_t* private_key, crypto_rsa_public_key_t* public_key, uint32_t key_bits);
int crypto_rsa_sign_pss(const crypto_rsa_private_key_t* private_key, const uint8_t* hash, uint32_t hash_len, uint8_t* signature, uint32_t* sig_len);
int crypto_rsa_verify_pss(const crypto_rsa_public_key_t* public_key, const uint8_t* hash, uint32_t hash_len, const uint8_t* signature, uint32_t sig_len);
int crypto_rsa_sign_pkcs1v15(const crypto_rsa_private_key_t* private_key, const uint8_t* hash, uint32_t hash_len, uint8_t* signature, uint32_t* sig_len);
int crypto_rsa_verify_pkcs1v15(const crypto_rsa_public_key_t* public_key, const uint8_t* hash, uint32_t hash_len, const uint8_t* signature, uint32_t sig_len);
int crypto_rsa_encrypt_oaep(const crypto_rsa_public_key_t* public_key, const uint8_t* plaintext, uint32_t plaintext_len, uint8_t* ciphertext, uint32_t* ciphertext_len);
int crypto_rsa_decrypt_oaep(const crypto_rsa_private_key_t* private_key, const uint8_t* ciphertext, uint32_t ciphertext_len, uint8_t* plaintext, uint32_t* plaintext_len);

// ECDSA operations
int crypto_ecdsa_generate_keypair(crypto_ecdsa_private_key_t* private_key, crypto_ecdsa_public_key_t* public_key, uint32_t curve_bits);
int crypto_ecdsa_sign(const crypto_ecdsa_private_key_t* private_key, const uint8_t* hash, uint32_t hash_len, crypto_ecdsa_signature_t* signature);
int crypto_ecdsa_verify(const crypto_ecdsa_public_key_t* public_key, const uint8_t* hash, uint32_t hash_len, const crypto_ecdsa_signature_t* signature);
int crypto_ecdsa_compress_public_key(const crypto_ecdsa_public_key_t* public_key, uint8_t* compressed, uint32_t* compressed_len);
int crypto_ecdsa_decompress_public_key(const uint8_t* compressed, uint32_t compressed_len, crypto_ecdsa_public_key_t* public_key);

// Key derivation functions
int crypto_pbkdf2_sha256(const uint8_t* password, uint32_t password_len, const uint8_t* salt, uint32_t salt_len, uint32_t iterations, uint8_t* derived_key, uint32_t key_len);
int crypto_hkdf_sha256(const uint8_t* salt, uint32_t salt_len, const uint8_t* ikm, uint32_t ikm_len, const uint8_t* info, uint32_t info_len, uint8_t* okm, uint32_t okm_len);
int crypto_scrypt(const uint8_t* password, uint32_t password_len, const uint8_t* salt, uint32_t salt_len, uint32_t N, uint32_t r, uint32_t p, uint8_t* derived_key, uint32_t key_len);

// Digital certificates and X.509
typedef struct {
    uint8_t* der_data;
    uint32_t der_len;
    crypto_rsa_public_key_t* rsa_key;
    crypto_ecdsa_public_key_t* ecdsa_key;
    uint8_t key_type; // 0=RSA, 1=ECDSA
    char subject[256];
    char issuer[256];
    uint64_t not_before;
    uint64_t not_after;
} crypto_x509_cert_t;

int crypto_x509_parse_certificate(const uint8_t* der_data, uint32_t der_len, crypto_x509_cert_t* cert);
int crypto_x509_verify_certificate(const crypto_x509_cert_t* cert, const crypto_x509_cert_t* issuer_cert);
int crypto_x509_verify_chain(const crypto_x509_cert_t* certs, uint32_t cert_count, const crypto_x509_cert_t* trusted_roots, uint32_t root_count);
void crypto_x509_free_certificate(crypto_x509_cert_t* cert);

// Hardware-accelerated functions (when available)
int crypto_aes_hw_encrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* plaintext, uint8_t* ciphertext);
int crypto_aes_hw_decrypt_block(const uint8_t* key, uint32_t key_bits, const uint8_t* ciphertext, uint8_t* plaintext);
int crypto_sha256_hw_hash(const uint8_t* data, uint32_t len, uint8_t* hash);

// Secure random number generation
int crypto_random_init(void);
int crypto_random_bytes(uint8_t* buf, uint32_t len);
int crypto_random_uint32(uint32_t* value);
int crypto_random_uint64(uint64_t* value);
void crypto_random_cleanup(void);

// Constant-time operations (for side-channel resistance)
int crypto_memcmp_constant_time(const void* a, const void* b, size_t len);
void crypto_memzero_secure(void* ptr, size_t len);

// Legacy functions (for backward compatibility)
int verify_signature(const uint8_t* data, uint32_t len, const uint8_t* signature, const uint8_t* public_key);

// Cryptographic self-tests
int crypto_self_test_sha256(void);
int crypto_self_test_aes(void);
int crypto_self_test_rsa(void);
int crypto_self_test_ecdsa(void);
int crypto_self_test_chacha20_poly1305(void);
int crypto_run_all_self_tests(void);

// Zeroization and cleanup
void crypto_zeroize_context(void* ctx, size_t size);
void crypto_cleanup_all_contexts(void);

#endif
