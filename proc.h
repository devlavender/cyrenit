// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * proc.h - Handling of processess 
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __PROC_H
#define __PROC_H

#include <stdbool.h>
#include <sys/types.h>

#define FORK_ISCHILD 0

enum proc_status
{
        CYRENIT_PROC_STATUS_UNKNOWN = 0,
        CYRENIT_PROC_STATUS_UNSTARTED,
        CYRENIT_PROC_STATUS_RUNNING,
        CYRENIT_PROC_STATUS_STOPPED
};

struct process
{
        pid_t pid;
        int ret_value;
        char *exec_image;
        char **argv;
        char **environment;
        size_t arg_counter;
        size_t arg_allocated;
        size_t env_counter;
        size_t env_allocated;
        bool env_dynamic;
        bool registered;
        enum proc_status status;
};

extern struct process **registered_processes;
extern size_t registered_process_count;
extern size_t registered_process_allocated;

struct process *process_create();
void process_destroy(struct process *proc);
bool process_set_image(struct process *proc, const char *path);
bool process_set_args(struct process *proc, const char **args);
bool process_set_env(struct process *proc, const char **envp);
bool process_add_arg(struct process *proc, const char *arg);
bool process_set_envdynamic(struct process *proc);

bool process_set_pid(struct process *proc, pid_t pid);
bool process_set_retid(struct process *proc, int retid);
bool process_forkexec(struct process *proc);

bool register_process(struct process *proc);

static inline bool process_is_registered(struct process *proc)
{
        return proc == NULL ? false : proc->registered;
}

#endif//__PROC_H