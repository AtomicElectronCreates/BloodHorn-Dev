/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
#ifndef BLOODHORN_SETTINGS_H
#define BLOODHORN_SETTINGS_H

#include <stdint.h>
#include "compat.h"
#include "theme.h"

// Settings categories
typedef enum {
    SETTINGS_CATEGORY_GENERAL = 0,
    SETTINGS_CATEGORY_DISPLAY,
    SETTINGS_CATEGORY_SECURITY,
    SETTINGS_CATEGORY_NETWORK,
    SETTINGS_CATEGORY_ADVANCED,
    SETTINGS_CATEGORY_DIAGNOSTICS,
    SETTINGS_CATEGORY_COUNT
} SettingsCategory;

// Setting types
typedef enum {
    SETTING_TYPE_BOOLEAN = 0,
    SETTING_TYPE_INTEGER,
    SETTING_TYPE_STRING,
    SETTING_TYPE_CHOICE,
    SETTING_TYPE_PASSWORD,
    SETTING_TYPE_ACTION
} SettingType;

// Setting value union
typedef union {
    int boolean_value;
    int32_t integer_value;
    char string_value[256];
    int choice_index;
} SettingValue;

// Setting definition
typedef struct {
    const char* name;
    const char* description;
    SettingType type;
    SettingValue default_value;
    SettingValue current_value;
    SettingValue min_value;     // For integers
    SettingValue max_value;     // For integers
    const char** choices;       // For choice type
    int choice_count;           // Number of choices
    int (*validator)(const SettingValue* value);  // Optional validator
    void (*on_change)(const SettingValue* old_val, const SettingValue* new_val);  // Change callback
} Setting;

// Bootloader configuration
typedef struct {
    // General settings
    int boot_timeout;           // Timeout in seconds (0 = no timeout)
    int default_entry;          // Default boot entry index
    int enable_splash;          // Show splash screen
    int auto_boot;              // Auto-boot without menu
    char language[8];           // Language code (e.g., "en", "es", "fr")
    
    // Display settings
    int screen_resolution_x;    // Preferred screen width
    int screen_resolution_y;    // Preferred screen height
    int font_size;              // Font size (8, 12, 16, 24)
    char theme_name[64];        // Theme name
    int high_contrast;          // High contrast mode
    int text_scaling;           // Text scaling factor (100, 125, 150, 200)
    
    // Security settings
    int secure_boot;            // Secure boot enabled
    int tpm_required;           // Require TPM for boot
    int password_required;      // Require password for settings
    char admin_password_hash[64]; // Hashed admin password
    int signature_verify;       // Verify kernel signatures
    int measured_boot;          // Enable measured boot
    
    // Network settings
    int network_boot;           // Enable network boot
    char pxe_server[256];       // PXE server address
    int dhcp_enabled;           // Use DHCP
    char static_ip[16];         // Static IP address
    char subnet_mask[16];       // Subnet mask
    char gateway[16];           // Default gateway
    char dns_server[16];        // DNS server
    
    // Advanced settings
    int acpi_enabled;           // Enable ACPI
    int legacy_boot;            // Enable legacy BIOS boot
    int quiet_boot;             // Suppress verbose output
    int debug_mode;             // Enable debug output
    int memory_test;            // Run memory test on boot
    int cpu_microcode_update;   // Update CPU microcode
    
    // Diagnostics settings
    int enable_logging;         // Enable detailed logging
    int log_level;              // Log level (0=Error, 1=Warning, 2=Info, 3=Debug)
    char log_device[64];        // Log output device
    int hardware_test;          // Run hardware tests
} BootloaderConfig;

// Global configuration instance
extern BootloaderConfig g_bootloader_config;

// Settings management functions
void InitializeSettings(void);
void LoadSettingsFromConfig(void);
void SaveSettingsToConfig(void);
void ResetSettingsToDefaults(void);

// Settings UI functions
EFI_STATUS ShowSettingsMenu(void);
EFI_STATUS ShowCategorySettings(SettingsCategory category);
EFI_STATUS EditSetting(Setting* setting);

// Configuration validation
int ValidateConfiguration(const BootloaderConfig* config);
void FixInvalidSettings(BootloaderConfig* config);

// Specific setting validators
int ValidateTimeout(const SettingValue* value);
int ValidateResolution(const SettingValue* value);
int ValidateIPAddress(const SettingValue* value);
int ValidatePassword(const SettingValue* value);

// Setting change handlers
void OnLanguageChange(const SettingValue* old_val, const SettingValue* new_val);
void OnThemeChange(const SettingValue* old_val, const SettingValue* new_val);
void OnSecurityChange(const SettingValue* old_val, const SettingValue* new_val);
void OnDisplayChange(const SettingValue* old_val, const SettingValue* new_val);

// Password management
int VerifyAdminPassword(const char* password);
int ChangeAdminPassword(const char* old_password, const char* new_password);
void HashPassword(const char* password, char* hash_output);

// Theme management
int GetAvailableThemes(char themes[][64], int max_themes);
void ApplyTheme(const char* theme_name);

// Language management
int GetAvailableLanguages(char languages[][8], int max_languages);
void ApplyLanguage(const char* language_code);

#endif // BLOODHORN_SETTINGS_H
