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
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <libgen.h>

#include "cyrenit.h"
#include "cyrecli.h"
#include "mounts.h"
#include "proc.h"

int console_fd = -1;
pid_t console_pid = -1;

void dump_char_array(char **arr);
int main_loop(int argc, char **argv, char **envp);
int bootstrap(int argc, char **argv, char **envp);
int exec_fg(int argc, char **argv, char **envp);
int start_services();

int main(int argc, char **argv, char **envp)
{
        char *cmdline = *argv;

        fprintf(stdout, "cyrenit[%d]: game on! \n", getpid());
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
        int svc_ret = 0;

        fprintf(stdout, "cyrenit: starting bootstrap process...\n");
        fprintf(stdout, "cyrenit: dumping argv\n");
        dump_char_array(argv);
        fprintf(stdout, "cyrenit: dumping envp\n");
        dump_char_array(envp);

        fprintf(stdout, "cyrenit[%d]: creating mount tasks\n", getpid());
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
                fprintf(stderr, "cyrenit[%d]: ", getpid());
                perror("oops, error setting PATH: ");
        }
        else {
                fprintf(stdout, "cyrenit[%d]: set PATH successfully\n",
                        getpid());
        }

        fprintf(stdout, "cyrenit[%d]: starting services\n", getpid());
        svc_ret = start_services();
        fprintf(stdout, "cyrenit[%d]: started %d services successfully\n",
                getpid(), svc_ret);

        fprintf(stdout, "cyrenit[%d]: opening console\n", getpid());
        console_fd = open("/dev/console", O_RDWR);
        if (console_fd == -1) {
                perror("cyrenit: failed to open /dev/console: ");
        }
        else {
                fprintf(stdout, "cyrenit[%d]: /dev/console opened with fd %d\n",
                        getpid(), console_fd);
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
                fprintf(stdout, "cyrenit: starting /bin/bash\n");
                ret = exec_fg(bashc, bash_cmd, environ);
                fprintf(stdout, "cyrenit: /bin/bash returned with status %d", ret);
                sleep(5);
        }
        return EXIT_SUCCESS;
}

int start_services()
{
        char *svc_paths[] = {
                "/etc/cyrenit/services/l0/helloop",
                NULL
        };
        const char *start_args[] = {"start", NULL};
        char **svc_ptr = svc_paths;
        struct process *svc_proc = NULL;
        int ret = 0;

        while (svc_ptr && *svc_ptr) {
                fprintf(stdout, "cyrenit: starting service %s\n", *svc_ptr);

                svc_proc = process_create();
                if (svc_proc == NULL) {
                        fprintf(stderr, "cyrenit: failed to create process for %s\n", *svc_ptr);
                        svc_ptr++;
                        continue;
                }

                if (!process_set_image(svc_proc, *svc_ptr)) {
                        fprintf(stderr, "cyrenit: failed to set image for %s\n", *svc_ptr);
                        process_destroy(svc_proc);
                        svc_ptr++;
                        continue;
                }

                if (!process_set_args(svc_proc, start_args)) {
                        fprintf(stderr, "cyrenit: failed to set args for %s\n", *svc_ptr);
                        process_destroy(svc_proc);
                        svc_ptr++;
                        continue;
                }

                if (!process_set_envdynamic(svc_proc)) {
                        fprintf(stderr, "cyrenit: failed to set dynamic env for %s\n", *svc_ptr);
                        process_destroy(svc_proc);
                        svc_ptr++;
                        continue;
                }

                if (!register_process(svc_proc)) {
                        fprintf(stderr, "cyrenit: failed to register process for %s\n", *svc_ptr);
                        process_destroy(svc_proc);
                        svc_ptr++;
                        continue;
                }

                if (!process_forkexec(svc_proc)) {
                        fprintf(stderr, "cyrenit: failed to forkexec service %s\n", *svc_ptr);
                        process_destroy(svc_proc);
                        svc_ptr++;
                        continue;
                }

                ret++;
                svc_ptr++;
        }

        return ret;
}

int exec_fg(int argc, char **argv, char **envp) {
        pid_t ret;
        int exec_ret = 0;
        int status;
        int ioctl_ret = 0;
        int dup_stdin = 0;
        int dup_stdout = 0;
        int dup_stderr = 0;

        fprintf(stdout, "cyrenit[%d]: starting %s\n",
                getpid(), argv[0]);
        
        ret = fork();
        if (ret == -1) {
                fprintf(stderr, "cyrenit[%d]: ", getpid());
                perror("error forking process!");
                return EXIT_FAILURE;
        }
        if (ret == 0) {
                console_pid = setsid();
                ioctl_ret = ioctl(console_fd, TIOCSCTTY,0);
                fprintf(stdout, "cyrenit[%d]: passing terminal control from"
                        "pid %d to pid %d, ioctl ret %d\n", getpid(),
                        console_pid, getpid(), ioctl_ret);
                dup_stdin = dup2(console_fd, STDIN_FILENO);
                dup_stdout = dup2(console_fd, STDOUT_FILENO);
                dup_stderr = dup2(console_fd, STDERR_FILENO);
                if (console_pid > 2) {
                        close(console_pid);
                }
                fprintf(stdout, "cyrenit: duplicated console fd %d to "
                        "stdin %d, stdout %d, stderr %d\n",
                        console_fd, dup_stdin, dup_stdout, dup_stderr);
                
                exec_ret = execve(argv[0], argv, environ);
                if (exec_ret == -1) {
                        perror("cyrenit: error executing process:");
                }
        }
        else {
                fprintf(stdout, "cyrenit[%d]: waiting for pid %d to finish\n",
                        getpid(), ret);
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