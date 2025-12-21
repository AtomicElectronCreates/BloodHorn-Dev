/*
 * shell_history.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_SHELL_HISTORY_H
#define BLOODHORN_SHELL_HISTORY_H
int shell_history_add(const char* cmd);
const char* shell_history_get(int idx);
#endif 