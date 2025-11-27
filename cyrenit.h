// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * cyrenit.h - Global definitions for cyrenit
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __CYREINIT_H
#define __CYREINIT_H

#include <stddef.h>
#include <stdbool.h>

#include "proc.h"

#define STRCMP_EQUAL 0
#define INIT_PID 1
#define INIT_CMD "init"
#define CYRENIT_CLI_NAME "cyrenit"

bool check_pid_one_semantics(const char *cmd);
bool check_command(const char *cmd, const char *check);
bool check_pid(const pid_t check);

#endif//__CYREINIT_H