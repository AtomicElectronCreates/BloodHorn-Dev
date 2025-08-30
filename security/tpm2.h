/*
 * BloodHorn Bootloader - TPM 2.0 Support
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#ifndef BLOODHORN_TPM2_H
#define BLOODHORN_TPM2_H

#include <stdint.h>
#include "compat.h"

// TPM 2.0 Constants
#define TPM2_SPEC_FAMILY            0x322E3000  // "2.0"
#define TPM2_SPEC_LEVEL             00
#define TPM2_SPEC_VERSION           138
#define TPM2_SPEC_YEAR              2016
#define TPM2_SPEC_DAY_OF_YEAR       273

// TPM 2.0 Command Codes
#define TPM2_CC_Startup             0x00000144
#define TPM2_CC_Shutdown            0x00000145
#define TPM2_CC_SelfTest            0x00000143
#define TPM2_CC_GetCapability       0x0000017A
#define TPM2_CC_GetRandom           0x0000017B
#define TPM2_CC_PCR_Read            0x0000017E
#define TPM2_CC_PCR_Extend          0x00000182
#define TPM2_CC_PCR_Event           0x0000017C
#define TPM2_CC_Quote               0x00000158
#define TPM2_CC_GetTime             0x0000016C
#define TPM2_CC_Sign                0x0000015D
#define TPM2_CC_Verify              0x00000177

// TPM 2.0 Response Codes
#define TPM2_RC_SUCCESS             0x000
#define TPM2_RC_FAILURE             0x101
#define TPM2_RC_DISABLED            0x120
#define TPM2_RC_EXCLUSIVE           0x121
#define TPM2_RC_AUTH_TYPE           0x124
#define TPM2_RC_AUTH_MISSING        0x125
#define TPM2_RC_POLICY              0x126
#define TPM2_RC_PCR                 0x127
#define TPM2_RC_PCR_CHANGED         0x128
#define TPM2_RC_UPGRADE             0x12D
#define TPM2_RC_TOO_MANY_CONTEXTS   0x12E
#define TPM2_RC_AUTH_UNAVAILABLE    0x12F
#define TPM2_RC_REBOOT              0x130

// TPM 2.0 Startup Types
#define TPM2_SU_CLEAR               0x0000
#define TPM2_SU_STATE               0x0001

// PCR Banks
#define TPM2_ALG_SHA1               0x0004
#define TPM2_ALG_SHA256             0x000B
#define TPM2_ALG_SHA384             0x000C
#define TPM2_ALG_SHA512             0x000D
#define TPM2_ALG_SM3_256            0x0012

// PCR Numbers (TCG PC Client Platform Firmware Profile)
#define TPM2_PCR_CRTM               0  // Core Root of Trust Measurement
#define TPM2_PCR_BIOS               1  // BIOS Code
#define TPM2_PCR_OPTION_ROM         2  // Option ROM Code
#define TPM2_PCR_OPTION_ROM_DATA    3  // Option ROM Configuration and Data
#define TPM2_PCR_BOOT_MANAGER       4  // Boot Manager Code
#define TPM2_PCR_BOOT_MANAGER_DATA  5  // Boot Manager Code Configuration and Data
#define TPM2_PCR_HOST_PLATFORM      6  // Host Platform Configuration
#define TPM2_PCR_SECURE_BOOT        7  // Secure Boot Policy
#define TPM2_PCR_BOOTLOADER         8  // Bootloader (BloodHorn specific)
#define TPM2_PCR_KERNEL             9  // Kernel
#define TPM2_PCR_INITRD            10  // Initial RAM Disk

// Event Types (TCG EFI Platform Specification)
#define EV_PREBOOT_CERT             0x00000000
#define EV_POST_CODE                0x00000001
#define EV_UNUSED                   0x00000002
#define EV_NO_ACTION                0x00000003
#define EV_SEPARATOR                0x00000004
#define EV_ACTION                   0x00000005
#define EV_EVENT_TAG                0x00000006
#define EV_S_CRTM_CONTENTS          0x00000007
#define EV_S_CRTM_VERSION           0x00000008
#define EV_CPU_MICROCODE            0x00000009
#define EV_PLATFORM_CONFIG_FLAGS    0x0000000A
#define EV_TABLE_OF_DEVICES         0x0000000B
#define EV_COMPACT_HASH             0x0000000C
#define EV_IPL                      0x0000000D
#define EV_IPL_PARTITION_DATA       0x0000000E
#define EV_NONHOST_CODE             0x0000000F
#define EV_NONHOST_CONFIG           0x00000010
#define EV_NONHOST_INFO             0x00000011
#define EV_OMIT_BOOT_DEVICE_EVENTS  0x00000012

// EFI Specific Events
#define EV_EFI_EVENT_BASE           0x80000000
#define EV_EFI_VARIABLE_DRIVER_CONFIG    (EV_EFI_EVENT_BASE + 1)
#define EV_EFI_VARIABLE_BOOT             (EV_EFI_EVENT_BASE + 2)
#define EV_EFI_BOOT_SERVICES_APPLICATION (EV_EFI_EVENT_BASE + 3)
#define EV_EFI_BOOT_SERVICES_DRIVER      (EV_EFI_EVENT_BASE + 4)
#define EV_EFI_RUNTIME_SERVICES_DRIVER   (EV_EFI_EVENT_BASE + 5)
#define EV_EFI_GPT_EVENT                 (EV_EFI_EVENT_BASE + 6)
#define EV_EFI_ACTION                    (EV_EFI_EVENT_BASE + 7)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB    (EV_EFI_EVENT_BASE + 8)
#define EV_EFI_HANDOFF_TABLES            (EV_EFI_EVENT_BASE + 9)

// TPM 2.0 Structures

// TPM Header
typedef struct {
    uint16_t tag;               // TPM_ST_SESSIONS or TPM_ST_NO_SESSIONS
    uint32_t length;            // Total command/response length
    uint32_t command_code;      // Command code
} __attribute__((packed)) TPM2_COMMAND_HEADER;

typedef struct {
    uint16_t tag;               // TPM_ST_SESSIONS or TPM_ST_NO_SESSIONS
    uint32_t length;            // Total response length
    uint32_t response_code;     // Response code
} __attribute__((packed)) TPM2_RESPONSE_HEADER;

// PCR Selection
typedef struct {
    uint16_t hash_alg;          // Hash algorithm
    uint8_t size_of_select;     // Size of PCR select array
    uint8_t pcr_select[3];      // PCR selection bitmap
} __attribute__((packed)) TPM2_PCR_SELECTION;

// PCR Value
typedef struct {
    uint16_t hash_alg;          // Hash algorithm
    uint8_t digest[64];         // Hash digest (max 64 bytes for SHA-512)
    uint16_t digest_size;       // Actual digest size
} __attribute__((packed)) TPM2_PCR_VALUE;

// Event Log Entry (TCG format)
typedef struct {
    uint32_t pcr_index;         // PCR number
    uint32_t event_type;        // Event type
    uint8_t digest[20];         // SHA-1 digest (legacy)
    uint32_t event_data_size;   // Size of event data
    uint8_t event_data[];       // Variable-length event data
} __attribute__((packed)) TCG_PCR_EVENT;

// Event Log Entry (TCG 2.0 format)
typedef struct {
    uint32_t pcr_index;         // PCR number
    uint32_t event_type;        // Event type
    uint32_t digest_count;      // Number of digests
    // Followed by digest_count TPMT_HA structures
    // Followed by event data
} __attribute__((packed)) TCG_PCR_EVENT2;

// Digest with algorithm
typedef struct {
    uint16_t hash_alg;          // Hash algorithm
    uint8_t digest[];           // Variable-length digest
} __attribute__((packed)) TPMT_HA;

// TPM Event Log
typedef struct {
    TCG_PCR_EVENT* events;      // Array of events
    uint32_t event_count;       // Number of events
    uint32_t max_events;        // Maximum events capacity
    uint8_t* log_buffer;        // Raw log buffer
    uint32_t log_size;          // Current log size
    uint32_t max_log_size;      // Maximum log size
} TPM2_EVENT_LOG;

// TPM Interface Functions

// TPM Initialization and Management
int tpm2_initialize(void);
int tpm2_startup(uint16_t startup_type);
int tpm2_shutdown(uint16_t shutdown_type);
int tpm2_self_test(void);
void tpm2_cleanup(void);

// Capability and Status
int tpm2_get_capability(uint32_t capability, uint32_t property, uint32_t property_count, void* response, uint32_t* response_size);
int tpm2_is_available(void);
int tpm2_get_manufacturer(char* manufacturer, uint32_t max_len);
int tpm2_get_version(uint32_t* family, uint32_t* level, uint32_t* revision);

// PCR Operations
int tpm2_pcr_read(uint32_t pcr_index, uint16_t hash_alg, TPM2_PCR_VALUE* pcr_value);
int tpm2_pcr_extend(uint32_t pcr_index, uint16_t hash_alg, const uint8_t* digest);
int tpm2_pcr_reset(uint32_t pcr_index);

// Measured Boot
int tpm2_measure_separator(uint32_t pcr_index);
int tpm2_measure_data(uint32_t pcr_index, uint32_t event_type, const void* data, uint32_t data_size, const char* description);
int tpm2_measure_file(uint32_t pcr_index, uint32_t event_type, const char* filename, const void* file_data, uint32_t file_size);
int tpm2_measure_string(uint32_t pcr_index, uint32_t event_type, const char* string);

// Event Log Management
int tpm2_event_log_init(TPM2_EVENT_LOG* log, uint32_t max_events, uint32_t max_log_size);
int tpm2_event_log_add(TPM2_EVENT_LOG* log, uint32_t pcr_index, uint32_t event_type, const void* data, uint32_t data_size, const char* description);
int tpm2_event_log_finalize(TPM2_EVENT_LOG* log);
void tpm2_event_log_cleanup(TPM2_EVENT_LOG* log);

// Attestation and Quotes
int tpm2_quote_pcrs(const uint32_t* pcr_list, uint32_t pcr_count, const uint8_t* nonce, uint32_t nonce_size, uint8_t* quote, uint32_t* quote_size);
int tmp2_verify_quote(const uint8_t* quote, uint32_t quote_size, const uint8_t* public_key, uint32_t key_size);

// Random Number Generation
int tpm2_get_random(uint8_t* buffer, uint32_t num_bytes);

// Sealing and Unsealing
int tpm2_seal_data(const uint8_t* data, uint32_t data_size, const uint32_t* pcr_list, uint32_t pcr_count, uint8_t* sealed_blob, uint32_t* blob_size);
int tpm2_unseal_data(const uint8_t* sealed_blob, uint32_t blob_size, uint8_t* data, uint32_t* data_size);

// Low-level TPM Communication
int tpm2_send_command(const void* command, uint32_t command_size, void* response, uint32_t* response_size);
int tpm2_wait_for_response(void);

// Hardware Interface Detection
typedef enum {
    TPM2_INTERFACE_TIS = 0,     // TPM Interface Specification
    TPM2_INTERFACE_CRB,         // Command Response Buffer
    TPM2_INTERFACE_FTPM,        // Firmware TPM
    TPM2_INTERFACE_NONE
} TPM2_INTERFACE_TYPE;

TPM2_INTERFACE_TYPE tpm2_detect_interface(void);
int tpm2_init_interface(TPM2_INTERFACE_TYPE interface_type);

// Measured Boot Chain Implementation
typedef struct {
    uint32_t measurement_count;
    uint32_t pcr_mask;          // Bitmask of used PCRs
    char boot_path[256];        // Boot path being measured
    TPM2_EVENT_LOG event_log;   // Event log
} MeasuredBootContext;

// Measured Boot Functions
int measured_boot_init(MeasuredBootContext* context);
int measured_boot_measure_bootloader(MeasuredBootContext* context);
int measured_boot_measure_config(MeasuredBootContext* context, const void* config_data, uint32_t config_size);
int measured_boot_measure_kernel(MeasuredBootContext* context, const void* kernel_data, uint32_t kernel_size, const char* kernel_path);
int measured_boot_measure_initrd(MeasuredBootContext* context, const void* initrd_data, uint32_t initrd_size, const char* initrd_path);
int measured_boot_measure_cmdline(MeasuredBootContext* context, const char* cmdline);
int measured_boot_finalize(MeasuredBootContext* context);
void measured_boot_cleanup(MeasuredBootContext* context);

// UEFI Integration
int tpm2_register_uefi_table(void* table, uint32_t table_size, const char* table_name);
int tpm2_measure_uefi_variable(const char* variable_name, const void* variable_data, uint32_t variable_size);
int tpm2_measure_uefi_boot_order(const uint16_t* boot_order, uint32_t count);

// Error Handling and Diagnostics
const char* tpm2_get_error_string(uint32_t error_code);
int tpm2_run_diagnostics(void);
void tpm2_print_status(void);
void tpm2_print_pcr_values(void);

#endif // BLOODHORN_TPM2_H
