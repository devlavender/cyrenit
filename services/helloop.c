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

#define LOOP_INTERVAL_TIME 10
#define LOOP_ITERATIONS 3

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
        int counter = 0;
        int ret;
        char **ptr = NULL;

        ptr = argv;
        ret = argc;
        ptr = envp;
        ptr++;

        while (counter++ < LOOP_ITERATIONS) {
                ret = printf("helloop service iteration %d "
                        "(of %d, %d interval)\n", counter,
                        LOOP_ITERATIONS, LOOP_INTERVAL_TIME);
                if (ret < 0) {
                        fprintf(stderr, "helloop: error writing to stdout\n");
                        return ret;
                }
                ret = (int) sleep(LOOP_INTERVAL_TIME);
                if (ret > 0) {
                        fprintf(stderr, "helloop: Oooops, there was an issue"
                                "while sleeping for %d time, %d remaining "
                                "(likely we received a signal)",
                                LOOP_INTERVAL_TIME, ret);
                }
        }

        return 0;
}


#endif//__HELLOOP_C