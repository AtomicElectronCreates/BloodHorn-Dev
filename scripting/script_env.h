/*
 * script_env.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_SCRIPT_ENV_H
#define BLOODHORN_SCRIPT_ENV_H
int script_env_set(const char* key, const char* value);
const char* script_env_get(const char* key);
#endif 