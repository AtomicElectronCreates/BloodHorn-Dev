/*
 * dhcp.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_DHCP_H
#define BLOODHORN_DHCP_H
#include <stdint.h>
#include "compat.h"
int dhcp_build_discover(uint8_t* buf, int xid);
int dhcp_parse_offer(const uint8_t* buf, uint32_t* offered_ip);
#endif 