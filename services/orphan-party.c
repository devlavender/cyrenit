// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * orphan-party.c - Orphan Party Service (testing service)
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
**/

/**
  * This service creates orphan processes to test the init system's
  * process reaper. It produces a new orphan process every 30 seconds,
  * forever.
**/

#ifndef __ORPHANPARTY_C
#define __ORPHANPARTY_C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>

#define ORPHAN_INTERVAL 30
#define ORPHAN_DURATION ORPHAN_INTERVAL * 5

#define FORK_RET_CHILD 0

int create_orphan();

int main(int argc, char **argv, char **envp)
{
        pid_t fork_ret = -2;
        pid_t waitpid_ret = -2;
        int sleep_ret = 0;
        int counter = 0;
        int ret_orphan = 0;
        int status = 0;

        while (1) {
                fork_ret = fork();
                if (fork_ret == -1) {
                        fprintf(stderr, "orphan-party[%d]:",
                                getpid());
                        perror("Error forking orphan creator: ");
                        return errno;
                }

                if (fork_ret == FORK_RET_CHILD) {
                        fprintf(stdout, "orphan-party[%d]: forked, now"
                                " creating the orphan!\n", getpid());
                        exit(create_orphan());
                }
                else {
                        counter++;
                        fprintf(stdout, "orphan-party[%d]: children %d "
                                "created successfully\n", getpid(), counter);
                        waitpid_ret = waitpid(fork_ret, &status, 0);
                        if (waitpid_ret < 0) {
                                fprintf(stderr, "orphan-party[%d]: ",
                                        getpid());
                                perror("Error waiting for orphan creator: ");
                                return errno;
                        }
                        if (waitpid_ret == fork_ret) {
                                if (WIFEXITED(status)) {
                                        ret_orphan = WEXITSTATUS(status);
                                        fprintf(stdout, "orphan-party[%d]: "
                                                "orphan creator %d exited with"
                                                " status %d\n",
                                                getpid(), fork_ret,
                                                ret_orphan);
                                }
                                else {
                                        fprintf(stdout, "orphan-party[%d]: "
                                                "orphan creator %d exited "
                                                "abnormally\n",
                                                getpid(), fork_ret);
                                }
                        }
                }

                fprintf(stdout, "orphan-party[%d]: sleeping for %d seconds"
                        "before creating next orphan\n", getpid(), ORPHAN_INTERVAL);
                sleep_ret = sleep(ORPHAN_INTERVAL);
        }

}

int create_orphan()
{
        pid_t fork_ret = -2;
        int sleep_ret = 0;

        fork_ret = fork();
        if (fork_ret == -1) {
                fprintf(stderr, "orphan-party[%d]: ",
                        getpid());
                perror("Error forking orphan process: ");
                return errno;
        }

        if (fork_ret == FORK_RET_CHILD) {
                fprintf(stdout, "orphan-party[%d]: created orphan "
                        "process %d, going to sleep for %d seconds "
                        "before exiting\n",
                        getpid(), getpid(), ORPHAN_DURATION);
                sleep_ret = sleep(ORPHAN_DURATION);
                while (sleep_ret > 0) {
                        fprintf(stderr, "orphan-party[%d]: nap interrupted "
                                "with %d seconds remaining\n",
                                getpid(), sleep_ret);
                        sleep_ret = sleep(sleep_ret);
                }
                fprintf(stdout, "orphan-party[%d]: orphan process ends\n",
                        getpid());
                exit(EXIT_SUCCESS);
        }
        else {
                fprintf(stdout, "orphan-party[%d]: create_orphan() parent; "
                        "let's die and leave my children %d orphan!\n",
                        getpid(), fork_ret);
                return EXIT_SUCCESS;
        }

        fprintf(stderr, "orphan-party[%d]: should never reach here!\n",
                getpid());
        return EXIT_FAILURE; //should never reach here
}

#endif//__ORPHANPARTY_C
