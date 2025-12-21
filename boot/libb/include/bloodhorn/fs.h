/*
 * fs.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_FS_H
#define BLOODHORN_FS_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// File system types
typedef enum {
    BH_FS_TYPE_UNKNOWN = 0,
    BH_FS_TYPE_FAT12,
    BH_FS_TYPE_FAT16,
    BH_FS_TYPE_FAT32,
    BH_FS_TYPE_EXFAT,
    BH_FS_TYPE_NTFS,
    BH_FS_TYPE_EXT2,
    BH_FS_TYPE_EXT3,
    BH_FS_TYPE_EXT4,
    BH_FS_TYPE_XFS,
    BH_FS_TYPE_BTRFS,
    BH_FS_TYPE_ZFS,
    BH_FS_TYPE_HFS_PLUS,
    BH_FS_TYPE_APFS,
    BH_FS_TYPE_UDF,
    BH_FS_TYPE_ISO9660,
    BH_FS_TYPE_CRAMFS,
    BH_FS_TYPE_SQUASHFS,
    BH_FS_TYPE_TMPFS,
    BH_FS_TYPE_RAMFS,
    BH_FS_TYPE_DEVFS,
    BH_FS_TYPE_PROCFS,
    BH_FS_TYPE_SYSFS,
    BH_FS_TYPE_MAX
} bh_fs_type_t;

// File access modes
typedef enum {
    BH_FILE_READ         = 0x01,
    BH_FILE_WRITE        = 0x02,
    BH_FILE_APPEND       = 0x04,
    BH_FILE_CREATE       = 0x08,
    BH_FILE_TRUNCATE     = 0x10,
    BH_FILE_EXCLUSIVE    = 0x20,
    BH_FILE_DIRECTORY    = 0x40,
    BH_FILE_BINARY       = 0x80,
    BH_FILE_SYNC         = 0x100,
    BH_FILE_DIRECT       = 0x200,
    BH_FILE_NONBLOCK     = 0x400,
    BH_FILE_TEMPORARY    = 0x800,
    BH_FILE_RANDOM       = 0x1000,
    BH_FILE_SEQUENTIAL   = 0x2000
} bh_file_mode_t;

// File types
typedef enum {
    BH_FILE_TYPE_UNKNOWN = 0,
    BH_FILE_TYPE_REGULAR,
    BH_FILE_TYPE_DIRECTORY,
    BH_FILE_TYPE_CHARACTER,
    BH_FILE_TYPE_BLOCK,
    BH_FILE_TYPE_FIFO,
    BH_FILE_TYPE_LINK,
    BH_FILE_TYPE_SOCKET
} bh_file_type_t;

// Seek origins
typedef enum {
    BH_SEEK_SET = 0,    // Beginning of file
    BH_SEEK_CUR = 1,    // Current position
    BH_SEEK_END = 2     // End of file
} bh_seek_origin_t;

// File attributes
typedef enum {
    BH_ATTR_NONE         = 0x00000000,
    BH_ATTR_READONLY     = 0x00000001,
    BH_ATTR_HIDDEN       = 0x00000002,
    BH_ATTR_SYSTEM       = 0x00000004,
    BH_ATTR_DIRECTORY    = 0x00000010,
    BH_ATTR_ARCHIVE      = 0x00000020,
    BH_ATTR_TEMPORARY    = 0x00000100,
    BH_ATTR_SPARSE       = 0x00000200,
    BH_ATTR_REPARSE      = 0x00000400,
    BH_ATTR_COMPRESSED   = 0x00000800,
    BH_ATTR_OFFLINE      = 0x00001000,
    BH_ATTR_INDEXED      = 0x00002000,
    BH_ATTR_ENCRYPTED    = 0x00004000,
    BH_ATTR_VIRTUAL      = 0x00010000
} bh_file_attributes_t;

// Time structure
typedef struct {
    bh_uint16_t year;        // 1601-30827
    bh_uint8_t  month;       // 1-12
    bh_uint8_t  day;         // 1-31
    bh_uint8_t  hour;        // 0-23
    bh_uint8_t  minute;      // 0-59
    bh_uint8_t  second;      // 0-59
    bh_uint32_t nanosecond;  // 0-999,999,999
    bh_int16_t  timezone;    // -1440 to 1440 minutes from UTC
    bh_uint8_t  daylight;    // Daylight saving time
    bh_uint8_t  pad1;
} bh_time_t;

// File information structure
typedef struct {
    bh_uint64_t size;
    bh_uint64_t physical_size;
    bh_time_t create_time;
    bh_time_t last_access_time;
    bh_time_t modify_time;
    bh_file_attributes_t attributes;
    char filename[260];  // Max path length
} bh_file_info_t;

// Directory entry
typedef struct {
    char name[256];
    bh_file_type_t type;
    bh_uint64_t size;
    bh_file_attributes_t attributes;
    bh_time_t create_time;
    bh_time_t modify_time;
    bh_time_t access_time;
} bh_dir_entry_t;

// Filesystem information
typedef struct {
    bh_fs_type_t type;
    char label[32];
    char uuid[37];
    bh_uint64_t total_size;
    bh_uint64_t free_size;
    bh_uint64_t available_size;
    bh_uint32_t block_size;
    bh_uint32_t cluster_size;
    bh_uint64_t total_inodes;
    bh_uint64_t free_inodes;
    bh_uint32_t max_filename_length;
    bh_bool_t case_sensitive;
    bh_bool_t case_preserving;
    bh_bool_t unicode_filenames;
    bh_bool_t read_only;
    bh_bool_t compressed;
    bh_bool_t encrypted;
} bh_fs_info_t;

// File handle (opaque)
typedef struct bh_file bh_file_t;

// Directory handle (opaque)
typedef struct bh_dir bh_dir_t;

// Volume handle (opaque)
typedef struct bh_volume bh_volume_t;

/**
 * @brief Initialize the filesystem subsystem
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_fs_initialize(void);

/**
 * @brief Shutdown the filesystem subsystem
 */
void bh_fs_shutdown(void);

/**
 * @brief Mount a filesystem
 * 
 * @param device_path Path to the device
 * @param mount_point Where to mount the filesystem
 * @param fs_type Filesystem type (BH_FS_TYPE_UNKNOWN for auto-detect)
 * @param flags Mount flags
 * @param volume [out] Volume handle
 * @return bh_status_t Status code
 */
bh_status_t bh_fs_mount(
    const char* device_path,
    const char* mount_point,
    bh_fs_type_t fs_type,
    bh_uint32_t flags,
    bh_volume_t** volume
);

/**
 * @brief Unmount a filesystem
 * 
 * @param volume Volume handle
 * @return bh_status_t Status code
 */
bh_status_t bh_fs_unmount(bh_volume_t* volume);

/**
 * @brief Get filesystem information
 * 
 * @param volume Volume handle
 * @param info [out] Filesystem information
 * @return bh_status_t Status code
 */
bh_status_t bh_fs_get_info(bh_volume_t* volume, bh_fs_info_t* info);

/**
 * @brief Open a file
 * 
 * @param path File path
 * @param mode File access mode
 * @param file [out] File handle
 * @return bh_status_t Status code
 */
bh_status_t bh_file_open(const char* path, bh_file_mode_t mode, bh_file_t** file);

/**
 * @brief Close a file
 * 
 * @param file File handle
 * @return bh_status_t Status code
 */
bh_status_t bh_file_close(bh_file_t* file);

/**
 * @brief Read from a file
 * 
 * @param file File handle
 * @param buffer Buffer to read into
 * @param size Number of bytes to read
 * @param bytes_read [out] Number of bytes actually read
 * @return bh_status_t Status code
 */
bh_status_t bh_file_read(
    bh_file_t* file,
    void* buffer,
    bh_size_t size,
    bh_size_t* bytes_read
);

/**
 * @brief Write to a file
 * 
 * @param file File handle
 * @param buffer Buffer to write from
 * @param size Number of bytes to write
 * @param bytes_written [out] Number of bytes actually written
 * @return bh_status_t Status code
 */
bh_status_t bh_file_write(
    bh_file_t* file,
    const void* buffer,
    bh_size_t size,
    bh_size_t* bytes_written
);

/**
 * @brief Seek in a file
 * 
 * @param file File handle
 * @param offset Offset to seek to
 * @param origin Seek origin
 * @param new_position [out] New file position (can be NULL)
 * @return bh_status_t Status code
 */
bh_status_t bh_file_seek(
    bh_file_t* file,
    bh_int64_t offset,
    bh_seek_origin_t origin,
    bh_uint64_t* new_position
);

/**
 * @brief Get the current file position
 * 
 * @param file File handle
 * @param position [out] Current position
 * @return bh_status_t Status code
 */
bh_status_t bh_file_tell(bh_file_t* file, bh_uint64_t* position);

/**
 * @brief Flush file buffers
 * 
 * @param file File handle
 * @return bh_status_t Status code
 */
bh_status_t bh_file_flush(bh_file_t* file);

/**
 * @brief Truncate a file
 * 
 * @param file File handle
 * @param size New file size
 * @return bh_status_t Status code
 */
bh_status_t bh_file_truncate(bh_file_t* file, bh_uint64_t size);

/**
 * @brief Get file information
 * 
 * @param file File handle
 * @param info [out] File information
 * @return bh_status_t Status code
 */
bh_status_t bh_file_get_info(bh_file_t* file, bh_file_info_t* info);

/**
 * @brief Set file information
 * 
 * @param file File handle
 * @param info File information
 * @return bh_status_t Status code
 */
bh_status_t bh_file_set_info(bh_file_t* file, const bh_file_info_t* info);

/**
 * @brief Delete a file
 * 
 * @param path File path
 * @return bh_status_t Status code
 */
bh_status_t bh_file_delete(const char* path);

/**
 * @brief Rename/move a file
 * 
 * @param old_path Current file path
 * @param new_path New file path
 * @return bh_status_t Status code
 */
bh_status_t bh_file_rename(const char* old_path, const char* new_path);

/**
 * @brief Copy a file
 * 
 * @param src_path Source file path
 * @param dst_path Destination file path
 * @param overwrite Allow overwriting existing files
 * @return bh_status_t Status code
 */
bh_status_t bh_file_copy(const char* src_path, const char* dst_path, bh_bool_t overwrite);

/**
 * @brief Check if a file exists
 * 
 * @param path File path
 * @return bh_bool_t true if file exists, false otherwise
 */
bh_bool_t bh_file_exists(const char* path);

/**
 * @brief Get file size
 * 
 * @param path File path
 * @param size [out] File size
 * @return bh_status_t Status code
 */
bh_status_t bh_file_get_size(const char* path, bh_uint64_t* size);

/**
 * @brief Create a directory
 * 
 * @param path Directory path
 * @param recursive Create parent directories if needed
 * @return bh_status_t Status code
 */
bh_status_t bh_dir_create(const char* path, bh_bool_t recursive);

/**
 * @brief Remove a directory
 * 
 * @param path Directory path
 * @param recursive Remove recursively (including contents)
 * @return bh_status_t Status code
 */
bh_status_t bh_dir_remove(const char* path, bh_bool_t recursive);

/**
 * @brief Open a directory for reading
 * 
 * @param path Directory path
 * @param dir [out] Directory handle
 * @return bh_status_t Status code
 */
bh_status_t bh_dir_open(const char* path, bh_dir_t** dir);

/**
 * @brief Close a directory
 * 
 * @param dir Directory handle
 * @return bh_status_t Status code
 */
bh_status_t bh_dir_close(bh_dir_t* dir);

/**
 * @brief Read the next directory entry
 * 
 * @param dir Directory handle
 * @param entry [out] Directory entry
 * @return bh_status_t Status code (BH_NOT_FOUND when no more entries)
 */
bh_status_t bh_dir_read(bh_dir_t* dir, bh_dir_entry_t* entry);

/**
 * @brief Reset directory reading to the beginning
 * 
 * @param dir Directory handle
 * @return bh_status_t Status code
 */
bh_status_t bh_dir_rewind(bh_dir_t* dir);

/**
 * @brief Check if a directory exists
 * 
 * @param path Directory path
 * @return bh_bool_t true if directory exists, false otherwise
 */
bh_bool_t bh_dir_exists(const char* path);

/**
 * @brief Get current working directory
 * 
 * @param buffer Buffer to store path
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_get_current_dir(char* buffer, bh_size_t size);

/**
 * @brief Set current working directory
 * 
 * @param path Directory path
 * @return bh_status_t Status code
 */
bh_status_t bh_set_current_dir(const char* path);

/**
 * @brief Resolve a relative path to absolute
 * 
 * @param path Input path
 * @param resolved [out] Buffer for resolved path
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_resolve(const char* path, char* resolved, bh_size_t size);

/**
 * @brief Get the directory part of a path
 * 
 * @param path Input path
 * @param dirname [out] Buffer for directory name
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_dirname(const char* path, char* dirname, bh_size_t size);

/**
 * @brief Get the filename part of a path
 * 
 * @param path Input path
 * @param basename [out] Buffer for base name
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_basename(const char* path, char* basename, bh_size_t size);

/**
 * @brief Get the extension part of a filename
 * 
 * @param path Input path
 * @param extension [out] Buffer for extension
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_extension(const char* path, char* extension, bh_size_t size);

/**
 * @brief Join path components
 * 
 * @param path1 First path component
 * @param path2 Second path component
 * @param result [out] Buffer for joined path
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_join(const char* path1, const char* path2, char* result, bh_size_t size);

/**
 * @brief Check if a path is absolute
 * 
 * @param path Path to check
 * @return bh_bool_t true if absolute, false if relative
 */
bh_bool_t bh_path_is_absolute(const char* path);

/**
 * @brief Normalize a path (remove . and .. components)
 * 
 * @param path Input path
 * @param normalized [out] Buffer for normalized path
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_path_normalize(const char* path, char* normalized, bh_size_t size);

/**
 * @brief Create a temporary file
 * 
 * @param template_path Template for the filename (must end with XXXXXX)
 * @param file [out] File handle
 * @return bh_status_t Status code
 */
bh_status_t bh_create_temp_file(const char* template_path, bh_file_t** file);

/**
 * @brief Create a temporary directory
 * 
 * @param template_path Template for the directory name (must end with XXXXXX)
 * @param path [out] Buffer for created directory path
 * @param size Buffer size
 * @return bh_status_t Status code
 */
bh_status_t bh_create_temp_dir(const char* template_path, char* path, bh_size_t size);

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_FS_H
