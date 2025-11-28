// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * proc.c - Handling of processess 
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __PROC_C
#define __PROC_C

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <linux/limits.h>

#include "cyrenit.h"
#include "proc.h"

#define PROCESS_ALLOC_STEP 8

/**
 * @var struct process **registered_processes
 * @brief Global array of registered processes
 */
struct process **registered_processes = NULL;

/**
 * @var size_t registered_process_count
 * @brief Current count of registered processes
 */
size_t registered_process_count = 0;

/**
 * @var size_t registered_process_allocated
 * @brief Allocated size of the registered processes array
 * @details The registered_processes array is resized in steps of
 *         PROCESS_ALLOC_STEP entries.
 */
size_t registered_process_allocated = 0;

/**
 * @fn struct process *process_create()
 * @brief Creates a process structure and returns it
 * @return the pointer on success or NULL on failure
 */
struct process *process_create()
{
        struct process *ret = NULL;

        ret = malloc(sizeof(struct process));
        if (ret == NULL) {
                return NULL;
        }

        bzero(ret, sizeof(struct process));

        ret->status = CYRENIT_PROC_STATUS_UNSTARTED;
        ret->pid = -1;

        return ret;
}

/**
 * @fn void process_destroy(struct process *proc)
 * @brief Destroys a process structure and frees all allocated memory
 * @param proc the process to be destroyed
 */
void process_destroy(struct process *proc)
{
        if (proc == NULL) {
                return;
        }

        if (proc->exec_image != NULL) {
                free(proc->exec_image);
        }

        if (proc->argv != NULL) {
                for (size_t i = 0; i < proc->arg_counter; i++) {
                        if (proc->argv[i] != NULL) {
                                free(proc->argv[i]);
                        }
                }
                free(proc->argv);
        }
        if (proc->environment != NULL) {
                for (size_t i = 0; i < proc->env_counter; i++) {
                        if (proc->environment[i] != NULL) {
                                free(proc->environment[i]);
                        }
                }
                free(proc->environment);
        }
        free(proc);       
}

/**
 * @fn bool process_set_image(struct process *proc, const char *path)
 * @brief Sets the executable image path of a process
 * @param proc the process to be modified
 * @param path the path to the executable image
 * @return true on success or false on failure
 */
bool process_set_image(struct process *proc, const char *path)
{
        char *new_path = NULL;

        if (proc == NULL || path == NULL) {
                return false;
        }

        new_path = strdup(path);
        if (new_path == NULL) {
                return false;
        }

        if (proc->exec_image != NULL) {
                free(proc->exec_image);
        }

        proc->exec_image = new_path;
        if (proc->argv != NULL && proc->arg_counter > 0) {
                proc->argv[0] = new_path;
        }
        else {
                if (!process_add_arg(proc, new_path)) {
                        return false;
                }
        }

        return true;
}

/**
 * @fn bool process_set_args(struct process *proc, const char **args)
 * @brief Sets the argument vector of a process
 * @param proc the process to be modified
 * @param args the argument vector
 * @return true on success or false on failure
 * @details This function will add the `basename()` of the exec_image
 *          as the first argument (argv[0]) as the first argument if not
 *          already present.
 */
bool process_set_args(struct process *proc, const char **args)
{
        char **arg_ptr = NULL;

        if (proc == NULL || args == NULL) {
                return false;
        }

        arg_ptr = (char **) args;
        if (*arg_ptr == NULL) {
                //empty list, we must clear everything except for argv[0]
                arg_ptr = proc->argv + 1;
                while (arg_ptr != NULL && *arg_ptr != NULL) {
                        arg_ptr++;
                }
                while (arg_ptr > proc->argv) {
                        free(*(--arg_ptr));
                        proc->arg_counter--;
                }

                return true;
        }

        if (proc->exec_image != NULL && 
                strncmp(*arg_ptr, basename(proc->exec_image), 
                        PATH_MAX) != STRCMP_EQUAL) {
                if (!process_add_arg(proc, basename(proc->exec_image))) {
                        return false;
                }
        }
        if (proc->exec_image == NULL) {
                //reserve the first entry for exec_image
                if (!process_add_arg(proc, "")) {
                        return false;
                }
        }

        while (arg_ptr != NULL && *arg_ptr != NULL) {

                if (!process_add_arg(proc, *arg_ptr)) {
                        return false;
                }
                arg_ptr++;
        }

        return proc->argv != NULL;
}

/**
 * @fn bool process_set_env(struct process *proc, const char **envp)
 * @brief Sets the environment vector of a process
 * @param proc the process to be modified
 * @param envp the environment vector
 * @return true on success or false on failure
 */
bool process_set_env(struct process *proc, const char **envp)
{
        char **env_ptr = NULL;
        size_t envp_size = 0;

        if (proc == NULL || envp == NULL) {
                return false;
        }

        env_ptr = (char **) envp;
        while (env_ptr != NULL && *env_ptr != NULL) {
                env_ptr++;
        }

        envp_size = (size_t)(env_ptr - (char **) envp);

        if (envp_size == 0) {
                return false; //empty list
        }

        proc->environment = malloc(sizeof(char *) * (envp_size + 1));
        if (proc->environment == NULL) {
                return false;
        }

        env_ptr = (char **) envp;
        proc->env_counter = 0;
        while (env_ptr != NULL && *env_ptr != NULL) {
                proc->environment[proc->env_counter] = strdup(*env_ptr);
                if (proc->environment[proc->env_counter] == NULL) {
                        return false;
                }

                proc->env_counter++;
                env_ptr++;
        }
        proc->environment[proc->env_counter] = NULL;
        return true;
}

/**
 * @fn bool process_add_arg(struct process *proc, const char *arg)
 * @brief Adds an argument to the argument vector of a process
 * @param proc the process to be modified
 * @param arg the argument to be added
 * @return true on success or false on failure
 */
bool process_add_arg(struct process *proc, const char *arg)
{
        char **new_argv = NULL;
        size_t new_size = 0;
        char *new_arg = NULL;

        if (proc == NULL || arg == NULL) {
                return false;
        }

        if (proc->arg_counter + 1 >= proc->arg_allocated) {
                new_size = proc->arg_allocated + 8;
                new_argv = realloc(proc->argv, sizeof(char *) * new_size);
                if (new_argv == NULL) {
                        return false;
                }
                proc->argv = new_argv;
                proc->arg_allocated = new_size;
        }

        new_arg = strdup(arg);
        if (new_arg == NULL) {
                return false;
        }

        proc->argv[proc->arg_counter] = new_arg;
        proc->arg_counter++;
        proc->argv[proc->arg_counter] = NULL;

        return true;
}

/**
 * @fn bool process_set_envdynamic(struct process *proc)
 * @brief Sets the environment of a process to be dynamic
 * @param proc the process to be modified
 * @return true on success or false on failure
 * @details Frees the current environment and marks it as dynamic
 */
bool process_set_envdynamic(struct process *proc)
{
        if (proc == NULL) {
                return false;
        }

        if (proc->environment != NULL) {
                for (size_t i = 0; i < proc->env_counter; i++) {
                        if (proc->environment[i] != NULL) {
                                free(proc->environment[i]);
                        }
                }
                free(proc->environment);
                proc->environment = NULL;
                proc->env_counter = 0;
                proc->env_allocated = 0;
        }

        proc->env_dynamic = true;
        return true;
}

/**
 * @fn bool process_set_pid(struct process *proc, pid_t pid)
 * @brief Sets the PID of a process
 * @param proc the process to be modified
 * @param pid the PID to be set
 * @return true on success or false on failure
 * @details Can only be set once, subsequent calls will fail
 */
bool process_set_pid(struct process *proc, pid_t pid)
{
        if (proc == NULL) {
                return false;
        }

        if (proc->pid != 0) {
                return false; //PID already set
        }

        proc->pid = pid;
        return true;
}

/**
 * @fn bool process_set_retid(struct process *proc, int retid)
 * @brief Sets the return value of a process
 * @param proc the process to be modified
 * @param retid the return value to be set
 * @return true on success or false on failure
 * @details Can only be set once, subsequent calls will fail.
 *         Also cannot be set if the process is stopped or not
 *         running yet.
 */
bool process_set_retid(struct process *proc, int retid)
{
        if (proc == NULL) {
                return false;
        }

        if (proc->ret_value != 0 ||
                proc->status != CYRENIT_PROC_STATUS_RUNNING) {
                return false; //return value already set
        }

        proc->ret_value = retid;
        return true;
}

/**
 * @fn bool process_forkexec(struct process *proc)
 * @brief Forks and execs the process described by proc
 * @param proc the process to be forked and execed
 * @return true on success or false on failure
 */
bool process_forkexec(struct process *proc)
{
        pid_t pid = 0;
        int exec_ret = 0;
        char **envp = NULL;
        char **argv = NULL;
        char *empty_arr[] = {NULL, NULL};

        if (proc == NULL || proc->exec_image == NULL) {
                return -1;
        }

        if (proc->status == CYRENIT_PROC_STATUS_RUNNING) {
                return false; //already running
        }

        fprintf(stdout, "cyrenit[%d]: forking process for %s\n",
                getpid(), proc->exec_image);
        pid = fork();
        
        if (pid == -1) {
                fprintf(stderr, "cyrenit[%d]: ", getpid());
                perror("error forking process!");
                return -1; //fork error
        }

        fprintf(stdout, "cyrenit[%d]: forked process %d for %s\n",
                getpid(), pid, proc->exec_image);
        
        if (pid == FORK_ISCHILD) {
                fprintf(stdout, "cyrenit[%d]: entering the children\n",
                        getpid());
                argv = proc->argv;
                if (argv == NULL) {
                        argv = empty_arr;
                        empty_arr[0] = proc->exec_image;
                }

                if (proc->env_dynamic || proc->environment == NULL) {
                        envp = environ;
                }
                else {
                        envp = proc->environment;
                }

                fprintf(stdout, "cyrenit[%d]: ready for execve of %s"
                        " with %zu args and %zu env vars\n",
                        getpid(), proc->exec_image,
                        proc->arg_counter, proc->env_counter);

                exec_ret = execve(proc->exec_image, argv, envp);
                if (exec_ret == -1) {
                        fprintf(stderr, "cyrenit[%d]: error executing %s:",
                                getpid(), proc->exec_image);
                        perror(" ");
                        exit(EXIT_FAILURE);
                        return false;
                }
        }
        else {
                fprintf(stdout, "cyrenit[%d]: parent process, child is %d\n",
                        getpid(), pid);
                proc->pid = pid;
                proc->status = CYRENIT_PROC_STATUS_RUNNING;
                if (!proc->registered) {
                        if (!register_process(proc)) {
                                fprintf(stderr, "cyrenit: failed to register "
                                        "process %s\n", proc->exec_image);
                                return false;
                        }
                        proc->registered = true;
                }
        }

        fprintf(stdout, "cyrenit[%d]: process_forkexec for %s "
                "completed\n", getpid(), proc->exec_image);

        return proc->pid == pid;

}

/**
 * @fn bool register_process(struct process *proc)
 * @brief Registers a process in the global registered_processes array
 * @param proc the process to be registered
 * @return true on success or false on failure
 * @details Will fail if already registered.
 */
bool register_process(struct process *proc)
{
        if (proc == NULL || proc->registered) {
                return false;
        }

        if (registered_processes == NULL) {
                registered_processes = malloc(sizeof(struct process *) *
                                             PROCESS_ALLOC_STEP);
                if (registered_processes == NULL) {
                        return false;
                }
                registered_process_allocated = PROCESS_ALLOC_STEP;
                registered_process_count = 0;
        }

        if (registered_process_count >= registered_process_allocated) {
                size_t new_size = registered_process_allocated + PROCESS_ALLOC_STEP;
                struct process **new_array = realloc(registered_processes,
                                                     new_size * sizeof(struct process *));
                if (new_array == NULL) {
                        return false;
                }
                registered_processes = new_array;
                registered_process_allocated = new_size;
        }

        registered_processes[registered_process_count++] = proc;
        return true;
}

#endif//__PROC_C