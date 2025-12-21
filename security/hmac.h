/*
 * hmac.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_HMAC_H
#define BLOODHORN_HMAC_H
#include <stdint.h>
#include "compat.h"
void hmac_sha256(const uint8_t* key, int keylen, const uint8_t* data, int datalen, uint8_t* out);
#endif 