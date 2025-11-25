// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * mounts.h - Global definitions for cyrenit
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __MOUNTS_H
#define __MOUNTS_H

#include <stddef.h>
#include <stdbool.h>

struct mount_task
{
        char *source;
        char *target;
        char *fs_type;
        void *data;
        unsigned long flags;
        size_t data_size;
};

struct mount_task_list
{
        struct mount_task **mount_tasks;
        size_t count;
};

#define MT_SIZE sizeof(struct mount_task)
#define MT_EMPTY 0

extern struct mount_task_list mounts;

bool add_mount_task(struct mount_task *task);
void free_mount_task_list();

struct mount_task *mount_task_create();
struct mount_task *mount_task_duplicate(struct mount_task *task);
bool mount_task_set_source(struct mount_task *task, const char *source);
bool mount_task_set_target(struct mount_task *task, const char *target);
bool mount_task_set_fstype(struct mount_task *task, const char *type);
bool mount_task_set_data(struct mount_task *task, size_t size, const void *data);
bool mount_task_set_flags(struct mount_task *task, unsigned long flags);
void mount_task_destroy(struct mount_task *task);

struct mount_task *mount_task_create_ready(const char *source,
                                        const char *target,
                                        const char *fs_type,
                                        unsigned long flags,
                                        size_t data_size,
                                        const void *data);

bool do_mounts(struct mount_task_list *m);

#endif//__MOUNTS_H