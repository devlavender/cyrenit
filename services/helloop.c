// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * helloop.c - Hello world loop (testing service)
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __HELLOOP_C
#define __HELLOOP_C

#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#define LOOP_INTERVAL_TIME 30
#define LOOP_ITERATIONS INT_MAX

int main(int argc, char **argv, char **envp)
{
        int counter = 0;
        int ret;
        char **ptr = NULL;

        ptr = argv;
        ret = argc;
        ptr = envp;
        ptr++;

        while (1) {
                ret = printf("helloop[%d]: service iteration %d "
                        "(of %d, %d interval)\n", getpid(), ++counter,
                        LOOP_ITERATIONS, LOOP_INTERVAL_TIME);
                if (ret < 0) {
                        fprintf(stderr, "helloop[%d]: ", getpid());
                        perror("error writing to stdout: ");
                        return ret;
                }
                ret = (int) sleep(LOOP_INTERVAL_TIME);
                if (ret > 0) {
                        fprintf(stderr, "helloop: Oooops, there was an issue"
                                "while sleeping for %d time, %d remaining "
                                "(likely we received a signal)",
                                LOOP_INTERVAL_TIME, ret);
                        sleep(ret);
                }
        }

        return 0;
}


#endif//__HELLOOP_C