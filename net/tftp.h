/*
 * tftp.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_TFTP_H
#define BLOODHORN_TFTP_H
#include <stdint.h>
#include "compat.h"
int tftp_build_rrq(const char* filename, uint8_t* buf);
int tftp_parse_data(const uint8_t* buf, uint16_t* block, uint8_t* data, int* datalen);
#endif 