// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * cyrecli.c - cyreinit CLI mode implementation
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __CYRECLI_H
#define __CYRECLI_H

#include <stdio.h>
#include <stdlib.h>

int cli_mode_main(int argc, char **argv, char **envp)
{
        fprintf(stderr, "CLI mode not implemented yet\n");
        return EXIT_FAILURE;
}

#endif//__CYRECLI_H