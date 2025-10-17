/**
 * Coreboot Payload Interface Header for BloodHorn Bootloader
 */

#ifndef COREBOOT_PAYLOAD_H
#define COREBOOT_PAYLOAD_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>

// Coreboot payload entry point signature
typedef VOID (*COREBOOT_PAYLOAD_ENTRY)(VOID* coreboot_table, VOID* payload);

/**
 * Coreboot payload header structure
 */
typedef struct {
    UINT8 signature[8];      // "$COREBOOT"
    UINT32 header_version;   // Header format version
    UINT16 payload_version;  // Payload version
    UINT16 payload_size;     // Size of payload in 512-byte blocks
    UINT32 cmd_line_size;    // Size of command line
    UINT32 checksum;         // Checksum of header
    UINT32 entry_point;      // Offset to entry point from start of header
    UINT32 payload_load_addr; // Load address for payload
    UINT32 payload_compressed_size; // Compressed size if compressed
    UINT32 payload_uncompressed_size; // Uncompressed size
    UINT32 reserved[4];      // Reserved for future use
    CHAR8 cmd_line[0];       // Command line (null-terminated)
} __attribute__((packed)) COREBOOT_PAYLOAD_HEADER;

// Constants
#define COREBOOT_PAYLOAD_SIGNATURE "$COREBOOT"
#define PAYLOAD_HEADER_VERSION 2

// Payload entry points
VOID EFIAPI BloodhornPayloadEntry(IN VOID* coreboot_table, IN VOID* payload);
VOID EFIAPI BloodhornMain(VOID);

// Boot menu and kernel loading functions
VOID EFIAPI BloodhornBootMenu(VOID);
BOOLEAN EFIAPI BloodhornLoadKernel(IN CONST CHAR8* kernel_path, OUT VOID** kernel_buffer, OUT UINT32* kernel_size);
BOOLEAN EFIAPI BloodhornExecuteKernel(IN VOID* kernel_buffer, IN UINT32 kernel_size);

// Payload creation and validation
VOID EFIAPI CreateBloodhornPayload(IN CONST CHAR8* output_file, IN CONST CHAR8* input_binary, IN CONST CHAR8* cmdline);
BOOLEAN EFIAPI ValidatePayloadHeader(IN CONST COREBOOT_PAYLOAD_HEADER* header);
VOID EFIAPI GetPayloadInfo(IN CONST COREBOOT_PAYLOAD_HEADER* header);

#endif // COREBOOT_PAYLOAD_H
