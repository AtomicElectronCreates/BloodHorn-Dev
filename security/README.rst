Security Module
==============

Overview
--------
The security module provides cryptographic and secure boot functionality for the BloodHorn
bootloader. It implements essential cryptographic primitives, secure boot verification,
and TPM 2.0 integration for platform security.

Features
--------
- Cryptographic primitives (AES, SHA-512, HMAC)
- Secure Boot verification
- TPM 2.0 integration
- Entropy collection for cryptographic operations
- Memory protection mechanisms

Core Components
---------------

Cryptographic Primitives (crypto.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Unified cryptographic interface
- Symmetric and asymmetric operations
- Key management
- Certificate handling

AES Implementation (aes.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~
- AES-128/192/256 implementation
- ECB, CBC, and GCM modes
- Constant-time operations
- Hardware acceleration support

SHA-512 (sha512.c)
~~~~~~~~~~~~~~~~~~
- SHA-512 hash function
- HMAC-SHA512 for message authentication
- Support for hashing large data streams

TPM 2.0 Integration (tpm2.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- TPM 2.0 command interface
- Secure storage and attestation
- Platform Configuration Registers (PCR) management
- Key creation and sealing

Secure Boot (secure_boot.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- UEFI Secure Boot verification
- Signature validation
- Certificate management
- Security policy enforcement

Entropy Collection (entropy.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- System entropy sources
- Cryptographically secure random number generation
- Entropy pool management

HMAC (hmac.c/h)
~~~~~~~~~~~~~~~
- HMAC implementation
- Keyed-hash message authentication
- Support for various hash functions

Dependencies
------------
- UEFI Runtime Services
- TPM 2.0 hardware (optional)
- UEFI Secure Boot services (for secure boot)
- Platform-specific entropy sources

Security Considerations
----------------------
- All cryptographic operations use constant-time algorithms
- Secure memory wiping for sensitive data
- Protection against timing attacks
- Secure key storage and handling
- Input validation for all security-critical functions

Usage Example
-------------
```c
#include "security/crypto.h"
#include "security/secure_boot.h"

// Initialize crypto library
crypto_init();

// Verify secure boot
if (secure_boot_verify_image(image, image_size) != SECURE_BOOT_SUCCESS) {
    // Handle verification failure
}

// Generate random data
uint8_t key[32];
crypto_random(key, sizeof(key));

// Encrypt data
uint8_t iv[16];
crypto_random(iv, sizeof(iv));

uint8_t plaintext[] = "Sensitive data";
uint8_t ciphertext[sizeof(plaintext) + 16]; // +16 for padding
size_t ciphertext_len;

if (crypto_aes_encrypt(plaintext, sizeof(plaintext), 
                       key, sizeof(key), iv, 
                       ciphertext, &ciphertext_len) != CRYPTO_SUCCESS) {
    // Handle error
}

// Clean up sensitive data
crypto_cleanup();
```

Building
--------
The security module is built as part of the main BloodHorn build process. Some features
may require specific hardware or firmware support:

- TPM 2.0 support requires TPM hardware
- Secure Boot requires UEFI Secure Boot to be enabled in firmware
- Hardware-accelerated crypto requires CPU support (AES-NI, SHA extensions)

Documentation
-------------
See header files for detailed API documentation:
- ``crypto.h``: Cryptographic operations
- ``tpm2.h``: TPM 2.0 interface
- ``secure_boot.h``: Secure Boot verification
- ``aes.h``: AES implementation
- ``sha512.h``: SHA-512 implementation

References
----------
- NIST FIPS 197 (AES)
- NIST FIPS 180-4 (SHA)
- TPM 2.0 Library Specification
- UEFI Secure Boot Specification
