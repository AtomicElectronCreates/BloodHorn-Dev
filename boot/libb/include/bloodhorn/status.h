/*
 * status.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_STATUS_H
#define BLOODHORN_STATUS_H

#include <bloodhorn/types.h>

/**
 * @brief Status codes used throughout BloodHorn
 */
typedef enum {
    // Success codes
    BH_SUCCESS = 0,             ///< Operation completed successfully
    BH_PENDING,                 ///< Operation is pending completion
    BH_TIMEOUT,                 ///< Operation timed out
    
    // Generic errors (1-99)
    BH_ERROR = 100,             ///< Generic error
    BH_INVALID_ARGUMENT,        ///< Invalid argument provided
    BH_INVALID_HANDLE,          ///< Invalid handle provided
    BH_INVALID_OPERATION,       ///< Invalid operation attempted
    BH_NOT_IMPLEMENTED,         ///< Functionality not implemented
    BH_NOT_SUPPORTED,           ///< Operation not supported
    BH_ACCESS_DENIED,           ///< Access denied
    BH_OUT_OF_MEMORY,           ///< Out of memory
    BH_OUT_OF_RESOURCES,        ///< Out of resources
    BH_DEVICE_ERROR,            ///< Device error occurred
    BH_IO_ERROR,                ///< I/O error occurred
    BH_TIMED_OUT,               ///< Operation timed out
    BH_WOULD_BLOCK,             ///< Operation would block
    BH_ALREADY_EXISTS,          ///< Resource already exists
    BH_NOT_FOUND,               ///< Resource not found
    BH_BUFFER_TOO_SMALL,        ///< Buffer too small
    
    // Filesystem errors (200-299)
    BH_PATH_NOT_FOUND = 200,    ///< Path not found
    BH_PATH_ALREADY_EXISTS,     ///< Path already exists
    BH_DIRECTORY_NOT_EMPTY,     ///< Directory not empty
    BH_IS_A_DIRECTORY,          ///< Path is a directory
    BH_NOT_A_DIRECTORY,         ///< Path is not a directory
    BH_READ_ONLY_FILESYSTEM,    ///< Filesystem is read-only
    BH_FILESYSTEM_FULL,         ///< Filesystem is full
    
    // Network errors (300-399)
    BH_NETWORK_ERROR = 300,     ///< Network error occurred
    BH_CONNECTION_REFUSED,      ///< Connection refused
    BH_CONNECTION_RESET,        ///< Connection reset
    BH_CONNECTION_ABORTED,      ///< Connection aborted
    BH_HOST_UNREACHABLE,        ///< Host unreachable
    BH_NETWORK_UNREACHABLE,     ///< Network unreachable
    BH_NETWORK_DOWN,            ///< Network is down
    
    // Security errors (400-499)
    BH_PERMISSION_DENIED = 400, ///< Permission denied
    BH_AUTH_FAILED,             ///< Authentication failed
    BH_NOT_AUTHORIZED,          ///< Not authorized
    BH_CERTIFICATE_ERROR,       ///< Certificate error
    
    // Hardware errors (500-599)
    BH_HARDWARE_ERROR = 500,    ///< Hardware error
    BH_DEVICE_NOT_FOUND,        ///< Device not found
    BH_DEVICE_BUSY,             ///< Device busy
    BH_DEVICE_OFFLINE,          ///< Device offline
    
    // Internal errors (900-999)
    BH_INTERNAL_ERROR = 900,    ///< Internal error
    BH_ASSERTION_FAILED,        ///< Assertion failed
    BH_UNEXPECTED_ERROR,        ///< Unexpected error occurred
    BH_UNKNOWN_ERROR = 999      ///< Unknown error
} bh_status_t;

/**
 * @brief Check if a status code indicates success
 * @param status Status code to check
 * @return true if status indicates success, false otherwise
 */
#define BH_SUCCEEDED(status) ((status) == BH_SUCCESS)

/**
 * @brief Check if a status code indicates failure
 * @param status Status code to check
 * @return true if status indicates failure, false otherwise
 */
#define BH_FAILED(status) ((status) != BH_SUCCESS)

/**
 * @brief Get a human-readable string for a status code
 * @param status Status code
 * @return String describing the status code
 */
const char* bh_status_to_string(bh_status_t status);

#endif // BLOODHORN_STATUS_H
