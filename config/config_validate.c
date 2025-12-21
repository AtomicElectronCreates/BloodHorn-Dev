/*
 * config_validate.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include "config_validate.h"
#include "compat.h"
#include <string.h>

int config_validate_key(const char* key) {
    if (!key || !*key) return 0;
    for (const char* p = key; *p; ++p) if (*p < 32 || *p > 126) return 0;
    return 1;
}

int config_validate_value(const char* value) {
    if (!value || !*value) return 0;
    for (const char* p = value; *p; ++p) if (*p < 32 || *p > 126) return 0;
    return 1;
} 