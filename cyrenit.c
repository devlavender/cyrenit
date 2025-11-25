// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * cyrenit.c - Minimal init for the cyrenit init system
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __CYREINIT_C
#define __CYREINIT_C

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <libgen.h>

#include "cyrenit.h"
#include "cyrecli.h"
#include "mounts.h"

void dump_char_array(char **arr);
int main_loop(int argc, char **argv, char **envp);
int bootstrap(int argc, char **argv, char **envp);
int exec_fg(int argc, char **argv, char **envp);

int main(int argc, char **argv, char **envp)
{
        char *cmdline = *argv;

        fprintf(stdout, "cyrenit: game on! \n");
        fprintf(stderr, "cyrenit: testing writing to stdout and stderr\n");

        if (check_command(cmdline, CYRENIT_CLI_NAME)) {
                return cli_mode_main(argc, argv, envp);
        }
        else if (check_command(cmdline, INIT_CMD)) {
                // init mainloop
                if (check_pid_one_semantics(cmdline)) {
                        bootstrap(argc, argv, environ);
                        return main_loop(argc, argv, environ);
                }
                fprintf(stderr, "cyrenit: ERROR: Are you fooling me? "
                                "You've called me as %s but I'm not PID 1\n",
                        cmdline);
                return EXIT_FAILURE;
        }
        else {
                fprintf(stderr, "cyrenit: ERROR: I do not attend by %s\n",
                        cmdline);
                return EXIT_FAILURE;
        }
}

/**
 * bool check_pid_one_semantics(const char *cmd)
 * @brief Check for valid PID1 semantics
 * @details Verifies whether the process name is `init` or `/sbin/init`
 *          and the PID == 1.
 * @param cmd string containing argv's first entry
 * 
 */
bool check_pid_one_semantics(const char *cmd)
{
        return check_command(cmd, INIT_CMD) && check_pid(INIT_PID);
        
}

/**
 * bool check_command(const char *cmd, const char *check)
 * @brief Check if command cmd is the same as check
 * @param cmd string pointer containing the command to be evaluated
 * @param check string pointer containing the command to be checked against
 */
bool check_command(const char *cmd, const char *check)
{
        char *base = NULL;

        if (cmd == NULL || check == NULL) {
                return false; //invalid string ptrs
        }

        base = (char *) cmd;

        if (*base == '/') {
                base = basename(base);
        }

        return strncmp(base, check, PATH_MAX) == STRCMP_EQUAL;
}

/**
 * bool check_pid(pid_t check)
 * @brief Check if running process' PID is equals to check
 * @param check the PID to check against
 */
bool check_pid(pid_t check)
{
        return check == getpid();
}

void dump_char_array(char **arr)
{
        size_t pos = 0;
        
        while (arr != NULL && *arr != NULL) {
                fprintf(stderr, "[%zu]: %s\n", pos++, *(arr++));
        }
}

int bootstrap(int argc, char **argv, char **envp)
{
        struct mount_task *mount_list[] = {
                mount_task_create_ready("proc", "/proc", "proc",
                                        0, 0, NULL),
                mount_task_create_ready("sysfs", "/sys", "sysfs",
                                        0, 0, NULL),
                mount_task_create_ready("devtmpfs", "/dev", "devtmpfs",
                                        0, 0, NULL),
                mount_task_create_ready("tmpfs", "/run", "tmpfs",
                                        0, 0, NULL),
                mount_task_create_ready("devpts", "/dev/pts", "devpts",
                                        0, 0, NULL),
                NULL
        };
        struct mount_task **mt_ptr = mount_list;
        int env_ret = 0;

        fprintf(stdout, "cyrenit: starting bootstrap process...\n");
        fprintf(stdout, "cyrenit: dumping argv\n");
        dump_char_array(argv);
        fprintf(stdout, "cyrenit: dumping envp\n");
        dump_char_array(envp);

        fprintf(stdout, "cyrenit: creating mount tasks\n");
        while (mt_ptr && *mt_ptr) {
                if (!add_mount_task(*mt_ptr)) {
                        fprintf(stderr, "cyrenit: failed to add mount "
                                "task %s\n", (*mt_ptr)->source);
                }
                fprintf(stderr, "cyrenit: added mount task %s\n",
                        (*mt_ptr)->source);
                mt_ptr++;
        }
        fprintf(stdout, "cyrenit: finished creating mount tasks, "
                "mounting them!\n");
        if (do_mounts(NULL)) {
                fprintf(stdout, "cyrenit: success mounting filesystems\n");
        }
        else {
                fprintf(stderr, "cyrenit: failed to mount filesystems\n");
        }

        fprintf(stdout, "cyrenit: creating basic environment\n");
        env_ret = setenv("PATH", "/bin:/sbin", 0);
        if (env_ret != 0) {
                perror("cyrenit: oops, error setting PATH: ");
        }

        return EXIT_SUCCESS;
}

int main_loop(int argc, char **argv, char **envp)
{
        int ret = 0;
        char *bash_cmd[] = {"/bin/bash", NULL };
        int bashc = sizeof(bash_cmd) / sizeof(char*);

        fprintf(stdout, "cyrenit: reaching main loop!\n");
        while (1){
                fprintf(stdout, "cyrenit: starting /bin/bash");
                ret = exec_fg(bashc, bash_cmd, environ);
                fprintf(stdout, "cyrenit: /bin/bash returned with status %d", ret);
                sleep(5);
        }
        return EXIT_SUCCESS;
}

int exec_fg(int argc, char **argv, char **envp) {
        pid_t ret;
        int exec_ret = 0;
        int status;

        fprintf(stdout, "cyrenit: starting %s\n",
                argv[0]);
        
        ret = fork();
        if (ret == -1) {
                perror("cyrenit: error forking process!");
                return EXIT_FAILURE;
        }

        if (ret == 0) {
                exec_ret = execve(argv[0], argv, environ);
                if (exec_ret == -1) {
                        perror("cyrenit: error executing process:");
                }
        }
        else {
                if (waitpid(ret, &status, 0) == EXIT_FAILURE) {
                        perror("cyrenit: error waiting for pid:");
                }
                if (WIFEXITED(status)) {
                        fprintf(stdout, "cyrenit: process exited\n");
                }
                if (WIFSIGNALED(status)) {
                        fprintf(stderr, "cyrenit: /bin/bash was killed by a signal\n");
                }
                
                return WEXITSTATUS(status);
        }

        return EXIT_FAILURE;
}

#endif//__CYREINIT_C