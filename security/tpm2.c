/*
 * BloodHorn Bootloader - TPM 2.0 Implementation
 */
#include "tpm2.h"
#include "crypto.h"
#include "compat.h"
#include <string.h>

// TPM Interface Registers (for TIS)
#define TPM_ACCESS_REG          0x0000
#define TPM_INT_ENABLE_REG      0x0008
#define TPM_INT_VECTOR_REG      0x000C
#define TPM_INT_STATUS_REG      0x0010
#define TPM_INTF_CAPS_REG       0x0014
#define TPM_STS_REG             0x0018
#define TPM_DATA_FIFO_REG       0x0024
#define TPM_DID_VID_REG         0x0F00
#define TPM_RID_REG             0x0F04

// TPM Access Register Bits
#define TPM_ACCESS_VALID            (1 << 7)
#define TPM_ACCESS_ACTIVE_LOCALITY  (1 << 5)
#define TPM_ACCESS_REQUEST_PENDING  (1 << 2)
#define TPM_ACCESS_REQUEST_USE      (1 << 1)
#define TPM_ACCESS_ESTABLISHMENT    (1 << 0)

// TPM Status Register Bits
#define TPM_STS_VALID               (1 << 7)
#define TPM_STS_COMMAND_READY       (1 << 6)
#define TPM_STS_GO                  (1 << 5)
#define TPM_STS_DATA_AVAIL          (1 << 4)
#define TPM_STS_EXPECT              (1 << 3)
#define TPM_STS_RESPONSE_RETRY      (1 << 1)

// Global TPM state
static TPM2_INTERFACE_TYPE g_tpm_interface = TPM2_INTERFACE_NONE;
static uint32_t g_tpm_base_address = 0;
static int g_tpm_initialized = 0;
static TPM2_EVENT_LOG g_global_event_log;

// Hardware interface functions
static uint8_t tpm_read8(uint32_t offset) {
    return *(volatile uint8_t*)(g_tpm_base_address + offset);
}

static void tpm_write8(uint32_t offset, uint8_t value) {
    *(volatile uint8_t*)(g_tmp_base_address + offset) = value;
}

static uint32_t tpm_read32(uint32_t offset) {
    return *(volatile uint32_t*)(g_tpm_base_address + offset);
}

static void tpm_write32(uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)(g_tpm_base_address + offset) = value;
}

TPM2_INTERFACE_TYPE tpm2_detect_interface(void) {
    // Check for TPM at standard addresses
    uint32_t tpm_addresses[] = {0xFED40000, 0xFED41000, 0};
    
    for (int i = 0; tpm_addresses[i] != 0; i++) {
        g_tpm_base_address = tpm_addresses[i];
        
        // Try to read DID/VID register
        uint32_t did_vid = tpm_read32(TPM_DID_VID_REG);
        if (did_vid != 0xFFFFFFFF && did_vid != 0x00000000) {
            // Found a TPM, check interface type
            uint32_t intf_caps = tpm_read32(TPM_INTF_CAPS_REG);
            
            if (intf_caps & (1 << 2)) {
                return TPM2_INTERFACE_CRB;  // Command Response Buffer
            } else {
                return TPM2_INTERFACE_TIS;  // TPM Interface Specification
            }
        }
    }
    
    return TPM2_INTERFACE_NONE;
}

int tpm2_init_interface(TPM2_INTERFACE_TYPE interface_type) {
    g_tpm_interface = interface_type;
    
    if (interface_type == TPM2_INTERFACE_NONE) {
        return -1;
    }
    
    // Request locality 0
    if (interface_type == TPM2_INTERFACE_TIS) {
        tpm_write8(TPM_ACCESS_REG, TPM_ACCESS_REQUEST_USE);
        
        // Wait for locality to be granted
        int timeout = 1000;
        while (timeout-- > 0) {
            uint8_t access = tmp_read8(TPM_ACCESS_REG);
            if (access & TPM_ACCESS_ACTIVE_LOCALITY) {
                break;
            }
            // Simple delay
            for (volatile int i = 0; i < 1000; i++);
        }
        
        if (timeout <= 0) {
            return -1; // Failed to acquire locality
        }
    }
    
    return 0;
}

int tpm2_send_command(const void* command, uint32_t command_size, void* response, uint32_t* response_size) {
    if (!command || !response || !response_size || g_tpm_interface == TPM2_INTERFACE_NONE) {
        return -1;
    }
    
    if (g_tpm_interface == TPM2_INTERFACE_TIS) {
        const uint8_t* cmd_bytes = (const uint8_t*)command;
        
        // Wait for TPM to be ready
        int timeout = 1000;
        while (timeout-- > 0) {
            uint32_t status = tpm_read32(TPM_STS_REG);
            if (status & TPM_STS_COMMAND_READY) {
                break;
            }
            for (volatile int i = 0; i < 1000; i++);
        }
        
        if (timeout <= 0) return -1;
        
        // Send command
        for (uint32_t i = 0; i < command_size; i++) {
            tpm_write8(TPM_DATA_FIFO_REG, cmd_bytes[i]);
        }
        
        // Execute command
        tpm_write32(TPM_STS_REG, TPM_STS_GO);
        
        // Wait for response
        timeout = 5000;
        while (timeout-- > 0) {
            uint32_t status = tpm_read32(TPM_STS_REG);
            if (status & TPM_STS_DATA_AVAIL) {
                break;
            }
            for (volatile int i = 0; i < 1000; i++);
        }
        
        if (timeout <= 0) return -1;
        
        // Read response
        uint8_t* resp_bytes = (uint8_t*)response;
        uint32_t bytes_read = 0;
        
        while (bytes_read < *response_size) {
            uint32_t status = tpm_read32(TPM_STS_REG);
            if (!(status & TPM_STS_DATA_AVAIL)) {
                break;
            }
            resp_bytes[bytes_read++] = tpm_read8(TPM_DATA_FIFO_REG);
        }
        
        *response_size = bytes_read;
        return 0;
    }
    
    return -1; // Unsupported interface
}

int tpm2_initialize(void) {
    if (g_tpm_initialized) return 0;
    
    TPM2_INTERFACE_TYPE interface = tpm2_detect_interface();
    if (interface == TPM2_INTERFACE_NONE) {
        return -1;
    }
    
    if (tpm2_init_interface(interface) != 0) {
        return -1;
    }
    
    // Initialize event log
    if (tpm2_event_log_init(&g_global_event_log, 1000, 64 * 1024) != 0) {
        return -1;
    }
    
    g_tpm_initialized = 1;
    return 0;
}

int tpm2_startup(uint16_t startup_type) {
    if (!g_tpm_initialized) return -1;
    
    uint8_t command[12] = {
        0x80, 0x01,                 // TPM_ST_NO_SESSIONS
        0x00, 0x00, 0x00, 0x0C,     // Length (12 bytes)
        0x00, 0x00, 0x01, 0x44,     // TPM2_CC_Startup
        (startup_type >> 8) & 0xFF, startup_type & 0xFF  // Startup type
    };
    
    uint8_t response[64];
    uint32_t response_size = sizeof(response);
    
    int result = tpm2_send_command(command, sizeof(command), response, &response_size);
    if (result != 0) return result;
    
    // Check response code
    if (response_size >= 10) {
        uint32_t response_code = (response[6] << 24) | (response[7] << 16) | (response[8] << 8) | response[9];
        return (response_code == TPM2_RC_SUCCESS) ? 0 : -1;
    }
    
    return -1;
}

int tpm2_self_test(void) {
    if (!g_tpm_initialized) return -1;
    
    uint8_t command[11] = {
        0x80, 0x01,                 // TPM_ST_NO_SESSIONS
        0x00, 0x00, 0x00, 0x0B,     // Length (11 bytes)
        0x00, 0x00, 0x01, 0x43,     // TPM2_CC_SelfTest
        0x01                        // fullTest = YES
    };
    
    uint8_t response[64];
    uint32_t response_size = sizeof(response);
    
    int result = tpm2_send_command(command, sizeof(command), response, &response_size);
    if (result != 0) return result;
    
    if (response_size >= 10) {
        uint32_t response_code = (response[6] << 24) | (response[7] << 16) | (response[8] << 8) | response[9];
        return (response_code == TPM2_RC_SUCCESS) ? 0 : -1;
    }
    
    return -1;
}

int tpm2_pcr_extend(uint32_t pcr_index, uint16_t hash_alg, const uint8_t* digest) {
    if (!g_tpm_initialized || !digest || pcr_index > 23) return -1;
    
    uint32_t digest_size = (hash_alg == TPM2_ALG_SHA256) ? 32 : 20;
    
    uint8_t command[1024];
    uint32_t cmd_size = 0;
    
    // Command header
    command[cmd_size++] = 0x80; command[cmd_size++] = 0x01; // TPM_ST_NO_SESSIONS
    cmd_size += 4; // Length (filled later)
    command[cmd_size++] = 0x00; command[cmd_size++] = 0x00;
    command[cmd_size++] = 0x01; command[cmd_size++] = 0x82; // TPM2_CC_PCR_Extend
    
    // PCR handle
    command[cmd_size++] = (pcr_index >> 24) & 0xFF;
    command[cmd_size++] = (pcr_index >> 16) & 0xFF;
    command[cmd_size++] = (pcr_index >> 8) & 0xFF;
    command[cmd_size++] = pcr_index & 0xFF;
    
    // Authorization area size (0 for no auth)
    command[cmd_size++] = 0x00; command[cmd_size++] = 0x00;
    command[cmd_size++] = 0x00; command[cmd_size++] = 0x00;
    
    // Digest count (1)
    command[cmd_size++] = 0x00; command[cmd_size++] = 0x00;
    command[cmd_size++] = 0x00; command[cmd_size++] = 0x01;
    
    // Hash algorithm
    command[cmd_size++] = (hash_alg >> 8) & 0xFF;
    command[cmd_size++] = hash_alg & 0xFF;
    
    // Digest
    memcpy(&command[cmd_size], digest, digest_size);
    cmd_size += digest_size;
    
    // Fill in length
    command[2] = (cmd_size >> 24) & 0xFF;
    command[3] = (cmd_size >> 16) & 0xFF;
    command[4] = (cmd_size >> 8) & 0xFF;
    command[5] = cmd_size & 0xFF;
    
    uint8_t response[64];
    uint32_t response_size = sizeof(response);
    
    int result = tpm2_send_command(command, cmd_size, response, &response_size);
    if (result != 0) return result;
    
    if (response_size >= 10) {
        uint32_t response_code = (response[6] << 24) | (response[7] << 16) | (response[8] << 8) | response[9];
        return (response_code == TPM2_RC_SUCCESS) ? 0 : -1;
    }
    
    return -1;
}

int tpm2_measure_data(uint32_t pcr_index, uint32_t event_type, const void* data, uint32_t data_size, const char* description) {
    if (!data || data_size == 0) return -1;
    
    // Calculate digest
    uint8_t digest[32];
    sha256_hash((const uint8_t*)data, data_size, digest);
    
    // Extend PCR
    int result = tpm2_pcr_extend(pcr_index, TPM2_ALG_SHA256, digest);
    if (result != 0) return result;
    
    // Add to event log
    return tpm2_event_log_add(&g_global_event_log, pcr_index, event_type, data, data_size, description);
}

int tpm2_measure_string(uint32_t pcr_index, uint32_t event_type, const char* string) {
    if (!string) return -1;
    return tpm2_measure_data(pcr_index, event_type, string, strlen(string), string);
}

int tpm2_event_log_init(TPM2_EVENT_LOG* log, uint32_t max_events, uint32_t max_log_size) {
    if (!log) return -1;
    
    log->events = (TCG_PCR_EVENT*)malloc(max_events * sizeof(TCG_PCR_EVENT));
    if (!log->events) return -1;
    
    log->log_buffer = (uint8_t*)malloc(max_log_size);
    if (!log->log_buffer) {
        free(log->events);
        return -1;
    }
    
    log->event_count = 0;
    log->max_events = max_events;
    log->log_size = 0;
    log->max_log_size = max_log_size;
    
    return 0;
}

int tpm2_event_log_add(TPM2_EVENT_LOG* log, uint32_t pcr_index, uint32_t event_type, const void* data, uint32_t data_size, const char* description) {
    if (!log || !data || log->event_count >= log->max_events) return -1;
    
    // Calculate required space
    uint32_t desc_len = description ? strlen(description) + 1 : 0;
    uint32_t total_event_size = sizeof(TCG_PCR_EVENT) + data_size + desc_len;
    
    if (log->log_size + total_event_size > log->max_log_size) {
        return -1; // Not enough space
    }
    
    // Create event entry
    TCG_PCR_EVENT* event = &log->events[log->event_count];
    event->pcr_index = pcr_index;
    event->event_type = event_type;
    event->event_data_size = data_size + desc_len;
    
    // Calculate SHA-1 digest for legacy compatibility
    uint8_t combined_data[data_size + desc_len];
    memcpy(combined_data, data, data_size);
    if (description) {
        memcpy(combined_data + data_size, description, desc_len);
    }
    
    // For now, use SHA-256 and truncate to 20 bytes (proper implementation would use SHA-1)
    uint8_t sha256_digest[32];
    sha256_hash(combined_data, data_size + desc_len, sha256_digest);
    memcpy(event->digest, sha256_digest, 20);
    
    // Copy event data to log buffer
    uint8_t* event_data_ptr = log->log_buffer + log->log_size;
    memcpy(event_data_ptr, combined_data, data_size + desc_len);
    
    log->log_size += total_event_size;
    log->event_count++;
    
    return 0;
}

int measured_boot_init(MeasuredBootContext* context) {
    if (!context) return -1;
    
    memset(context, 0, sizeof(MeasuredBootContext));
    
    // Initialize TPM if not already done
    if (!g_tpm_initialized) {
        if (tpm2_initialize() != 0) {
            return -1;
        }
        
        // Startup TPM
        if (tpm2_startup(TPM2_SU_CLEAR) != 0) {
            return -1;
        }
        
        // Run self-test
        if (tpm2_self_test() != 0) {
            return -1;
        }
    }
    
    // Initialize event log
    if (tpm2_event_log_init(&context->event_log, 500, 32 * 1024) != 0) {
        return -1;
    }
    
    // Measure separator in PCR 0-7 (standard practice)
    for (int pcr = 0; pcr <= 7; pcr++) {
        tpm2_measure_separator(pcr);
        context->pcr_mask |= (1 << pcr);
    }
    
    return 0;
}

int tpm2_measure_separator(uint32_t pcr_index) {
    // Standard separator is 4 bytes of 0xFF
    uint8_t separator[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    return tpm2_measure_data(pcr_index, EV_SEPARATOR, separator, sizeof(separator), "Separator");
}

int measured_boot_measure_bootloader(MeasuredBootContext* context) {
    if (!context) return -1;
    
    // Measure bootloader code (this would be the actual bootloader binary)
    const char* bootloader_version = "BloodHorn v1.0";
    int result = tpm2_measure_string(TPM2_PCR_BOOTLOADER, EV_S_CRTM_VERSION, bootloader_version);
    if (result == 0) {
        context->measurement_count++;
        context->pcr_mask |= (1 << TPM2_PCR_BOOTLOADER);
    }
    
    return result;
}

int measured_boot_measure_kernel(MeasuredBootContext* context, const void* kernel_data, uint32_t kernel_size, const char* kernel_path) {
    if (!context || !kernel_data || !kernel_path) return -1;
    
    int result = tmp2_measure_file(TPM2_PCR_KERNEL, EV_IPL, kernel_path, kernel_data, kernel_size);
    if (result == 0) {
        context->measurement_count++;
        context->pcr_mask |= (1 << TPM2_PCR_KERNEL);
        strncpy(context->boot_path, kernel_path, sizeof(context->boot_path) - 1);
    }
    
    return result;
}

int tpm2_measure_file(uint32_t pcr_index, uint32_t event_type, const char* filename, const void* file_data, uint32_t file_size) {
    if (!filename || !file_data) return -1;
    
    // Create event data with filename + file hash
    uint32_t filename_len = strlen(filename) + 1;
    uint32_t total_size = filename_len + file_size;
    
    uint8_t* event_data = (uint8_t*)malloc(total_size);
    if (!event_data) return -1;
    
    memcpy(event_data, filename, filename_len);
    memcpy(event_data + filename_len, file_data, file_size);
    
    int result = tpm2_measure_data(pcr_index, event_type, event_data, total_size, filename);
    
    free(event_data);
    return result;
}

int tpm2_is_available(void) {
    return g_tpm_initialized;
}

const char* tpm2_get_error_string(uint32_t error_code) {
    switch (error_code) {
        case TPM2_RC_SUCCESS: return "Success";
        case TPM2_RC_FAILURE: return "General failure";
        case TPM2_RC_DISABLED: return "TPM is disabled";
        case TPM2_RC_EXCLUSIVE: return "Exclusive access required";
        case TPM2_RC_AUTH_TYPE: return "Authentication type not supported";
        case TPM2_RC_AUTH_MISSING: return "Authentication missing";
        case TPM2_RC_POLICY: return "Policy failure";
        case TPM2_RC_PCR: return "PCR error";
        case TPM2_RC_PCR_CHANGED: return "PCR changed";
        case TPM2_RC_REBOOT: return "Reboot required";
        default: return "Unknown error";
    }
}

int tpm2_run_diagnostics(void) {
    if (!g_tmp_initialized) {
        return -1;
    }
    
    // Test basic TPM functionality
    uint8_t test_data[] = "TPM Diagnostic Test";
    uint8_t digest[32];
    sha256_hash(test_data, sizeof(test_data) - 1, digest);
    
    // Try to extend a test PCR (23 is usually available for testing)
    int result = tpm2_pcr_extend(23, TPM2_ALG_SHA256, digest);
    
    return result;
}

void tpm2_cleanup(void) {
    if (g_tmp_initialized) {
        tpm2_event_log_cleanup(&g_global_event_log);
        g_tpm_initialized = 0;
    }
}

void tpm2_event_log_cleanup(TPM2_EVENT_LOG* log) {
    if (!log) return;
    
    if (log->events) {
        free(log->events);
        log->events = NULL;
    }
    
    if (log->log_buffer) {
        free(log->log_buffer);
        log->log_buffer = NULL;
    }
    
    memset(log, 0, sizeof(TPM2_EVENT_LOG));
}
