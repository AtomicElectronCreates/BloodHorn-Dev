/*
 * config_ini.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

struct boot_menu_entry {
    char section[128];
    char name[64];
    char path[128];
};

/**
 * Parse INI configuration file and populate boot menu entries
 *
 * @param filename The path to the INI configuration file
 * @param entries Array to store parsed boot menu entries
 * @param max_entries Maximum number of entries to parse
 * @return Number of entries successfully parsed
 */
int parse_ini(const char* filename, struct boot_menu_entry* entries, int max_entries);

#endif // BLOODHORN_CONFIG_INI_H 
