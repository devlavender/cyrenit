// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * zombie-party.c - Zombie Party Service (testing service)
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

 /**
  * This service creates zombie processes to test the init system's
  * process reaper. It produces a new zombie process every 30 seconds,
  * forever.
  */

#ifndef __ZOMBIEPARTY_C
#define __ZOMBIEPARTY_C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>

#define ZOMBIE_INTERVAL 30

int main(int argc, char **argv, char **envp)
{
        pid_t ret;
        int sleep_ret = 0;
        int counter = 0;

        while (1) {
                ret = fork();
                if (ret == -1) {
                        fprintf(stderr, "zoombie-party[%d]: \n", getpid());
                        perror("Error forking zombie process: ");
                        return errno;
                }
                if (ret == 0) {
                        fprintf(stdout, "zoombie-party[%d]: "
                                "created zombie process\n", getpid());
                        exit(EXIT_SUCCESS);
                }
                else {
                        counter++;
                        fprintf(stdout, "zoombie-party[%d]: created zombie "
                                "%d (total zombies created: %d)\n",
                                getpid(), ret, counter);
                }

                fprintf(stdout, "zoombie-party[%d]: sleeping for %d seconds "
                        "before creating next zombie\n",
                        getpid(), ZOMBIE_INTERVAL);
                sleep_ret = sleep(ZOMBIE_INTERVAL);
                if (sleep_ret > 0) {
                        fprintf(stderr, "zoombie-party[%d]: nap interrupted "
                                "with %d seconds remaining\n",
                                getpid(), sleep_ret);
                        sleep(sleep_ret);
                }
                fprintf(stdout, "zoombie-party[%d]: woke up from its nap "
                                "ready to invite more zombies to the party\n",
                                getpid());
        }

        return EXIT_SUCCESS;
}

#endif//__ZOMBIEPARTY_C