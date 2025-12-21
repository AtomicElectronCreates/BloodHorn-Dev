/*
 * shell_net.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_SHELL_NET_H
#define BLOODHORN_SHELL_NET_H
int shell_cmd_ping(const char* host, char* out, int maxlen);
int shell_cmd_ifconfig(char* out, int maxlen);
#endif 