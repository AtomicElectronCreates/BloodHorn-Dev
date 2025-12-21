/*
 * config_env.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_CONFIG_ENV_H
#define BLOODHORN_CONFIG_ENV_H
int config_env_get(const char* key, char* value, int maxlen);
#endif 