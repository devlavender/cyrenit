// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * mounts.c - Global definitions for cyrenit
 *
 * Copyright (C) 2025, Ágatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This file is part of Cyrenit. It is licensed under the GNU GPL, version 3 or
 * any later version. See the LICENSE file accompanying this project for full
 * details.
 */

#ifndef __MOUNTS_C
#define __MOUNTS_C

#include "mounts.h"

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/mount.h>

struct mount_task_list mounts = { .mount_tasks = NULL, .count = 0 };

/**
 * @fn bool add_mount_task(struct mount_task *task)
 * @brief Add a mount task to the mount task list
 * @param task a pointer to a task
 * @attention It will copy the pointer's memory contents of task, so you
 *      are responsible for freeing your copy
 */
bool add_mount_task(struct mount_task *task)
{
        struct mount_task *add = NULL;

        if (task == NULL) {
                return false;
        }

        add = mount_task_duplicate(task);
        if (add == NULL) {
                return false;
        }

        if (mounts.mount_tasks == NULL) {
                mounts.count = MT_EMPTY;
                mounts.mount_tasks = calloc(mounts.count, MT_SIZE);
                if (mounts.mount_tasks == NULL) {
                        goto add_mtask_free_and_return;
                }
        }
        else {
                mounts.mount_tasks = reallocarray(mounts.mount_tasks,
                                                mounts.count+1, MT_SIZE);
                if (mounts.mount_tasks == NULL) {
                        goto add_mtask_free_and_return;
                }
        }

        mounts.mount_tasks[mounts.count] = add;
        mounts.count++;

        return true;

add_mtask_free_and_return:
        free(add);
        return false;
}

/**
 * @fn void free_mount_task_list()
 * @brief Frees the entire mount task list and its items
 */
void free_mount_task_list()
{
        size_t i = 0;

        if (mounts.mount_tasks == NULL) {
                return;
        }

        for (i = 0; i < mounts.count; i++) {
                mount_task_destroy(mounts.mount_tasks[i]);
        }

        free(mounts.mount_tasks);
        mounts.count = MT_EMPTY;
        
}

/**
 * @fn struct mount_task *mount_task_create()
 * @brief Creates a mount_task structure and returns it
 * @return the pointer on success or NULL on failure
 */
struct mount_task *mount_task_create()
{
        struct mount_task *ret = NULL;

        ret = malloc(MT_SIZE);
        bzero(ret, MT_SIZE);

        return ret;
}

/**
 * @fn struct mount_task *mount_task_duplicate(struct mount_task *task)
 * @brief Duplicates a mount_task structure and returns it
 * @param task the mount_task to be duplicated
 * @return the pointer on success or NULL on failure
 */
struct mount_task *mount_task_duplicate(struct mount_task *task)
{
        struct mount_task *ret = NULL;
        bool set_src;
        bool set_target;
        bool set_fs;
        bool set_flags;
        bool set_data;

        if (task == NULL) {
                return NULL;
        }

        ret = mount_task_create();
        if (ret == NULL) {
                return NULL;
        }

        set_src = mount_task_set_source(ret, task->source);
        set_target = mount_task_set_target(ret, task->target);
        set_fs = mount_task_set_fstype(ret, task->fs_type);
        set_data = mount_task_set_data(ret, task->data_size, task->data);
        set_flags = mount_task_set_flags(ret, task->flags);

        if (!set_src || !set_target || !set_fs || !set_flags || !set_data) {
                mount_task_destroy(ret);
                return NULL;
        }

        return ret;
}


/**
 * @fn bool mount_task_set_source(struct mount_task *task, const char *source)
 * @brief Sets the source of a mount_task
 * @param task the mount_task to be modified
 * @param source the source string
 * @return true on success or false on failure
 */
bool mount_task_set_source(struct mount_task *task, const char *source)
{
        if (task == NULL || source == NULL) {
                return false;
        }

        if (task->source != NULL) {
                free(task->source);
        }

        task->source = strdup(source);
        return task->source != NULL;
}

/**
 * @fn bool mount_task_set_target(struct mount_task *task, const char *target)
 * @brief Sets the target of a mount_task
 * @param task the mount_task to be modified
 * @param target the target string
 * @return true on success or false on failure
 */
bool mount_task_set_target(struct mount_task *task, const char *target)
{
        if (task == NULL || target == NULL) {
                return false;
        }

        if (task->target != NULL) {
                free(task->target);
        }

        task->target = strdup(target);
        return task->target != NULL;
}


/**
 * @fn bool mount_task_set_fstype(struct mount_task *task, const char *type)
 * @brief Sets the filesystem type of a mount_task
 * @param task the mount_task to be modified
 * @param type the filesystem type string
 * @return true on success or false on failure
 */
bool mount_task_set_fstype(struct mount_task *task, const char *type)
{
        if (task == NULL || type == NULL) {
                return false;
        }

        if (task->fs_type != NULL) {
                free(task->fs_type);
        }

        task->fs_type = strdup(type);
        return task->fs_type != NULL;
}

/**
 * @fn bool mount_task_set_data(struct mount_task *task, size_t size, const void *data)
 * @brief Sets the data element of a mount_task
 * @param task the mount_task to be modified
 * @param size the size of the data
 * @param data pointer to the data
 * @return true on success or false on failure
 */
bool mount_task_set_data(struct mount_task *task, size_t size, const void *data)
{
        if (task == NULL) {
                return false;
        }

        if (task->data != NULL) {
                free(task->data);
                task->data_size = 0;
        }

        if (size == 0) {
                return true;
        }

        task->data_size = size;
        task->data = malloc(size);
        if (task->data == NULL) {
                return false;
        }

        memcpy(task->data, data, size);
        return true;
}

/**
 * @fn bool mount_task_set_flags(struct mount_task *task, unsigned long flags)
 * @brief Sets the mounting flags of a mount_task
 * @param task the mount_task to be modified
 * @param flags the mounting flags
 * @return true on success or false on failure
 */
bool mount_task_set_flags(struct mount_task *task, unsigned long flags)
{
        if (task == NULL) {
                return false;
        }

        task->flags = flags;
        return true;
}

/**
 * @fn void mount_task_destroy(struct mount_task *task)
 * @brief Destroys a mount_task structure and frees its memory
 * @details This function will free all allocated memory inside the
 *          mount_task structure as well as the structure itself.
 * @param task the mount_task to be destroyed
 */
void mount_task_destroy(struct mount_task *task)
{
        if (task == NULL) {
                return;
        }

        if (task->source != NULL) {
                free(task->source);
        }
        if (task->target != NULL) {
                free(task->target);
        }
        if (task->fs_type != NULL) {
                free(task->fs_type);
        }
        if (task->data != NULL) {
                free((void *) task->data);
        }
        free(task);
}


/**
 * @fn struct mount_task *mount_task_create_ready(const char *source,
 *                                        const char *target,
 *                                        const char *fs_type,
 *                                        unsigned long flags,
 *                                        size_t data_size,
 *                                        const void *data)
 * @brief Creates a mount_task structure with all fields set
 * @param source the source string
 * @param target the target string
 * @param fs_type the filesystem type string
 * @param flags the mounting flags
 * @param data_size the size of the data
 * @param data pointer to the data
 * @return the pointer on success or NULL on failure
 */
struct mount_task *mount_task_create_ready(const char *source,
                                        const char *target,
                                        const char *fs_type,
                                        unsigned long flags,
                                        size_t data_size,
                                        const void *data)
{
        struct mount_task *ret = NULL;
        bool set_src;
        bool set_target;
        bool set_fs;
        bool set_flags;
        bool set_data;

        ret = mount_task_create();
        if (ret == NULL) {
                return NULL;
        }

        set_src = mount_task_set_source(ret, source);
        set_target = mount_task_set_target(ret, target);
        set_fs = mount_task_set_fstype(ret, fs_type);
        set_flags = mount_task_set_flags(ret, flags);
        set_data = mount_task_set_data(ret, data_size, data);

        if (!set_src || !set_target || !set_fs || !set_flags || !set_data) {
                mount_task_destroy(ret);
                return NULL;
        }

        return ret;
}

/**
 * @fn bool do_mounts(struct mount_task_list *m)
 * @brief mount the taks in the mount_task_list m or the global one if NULL
 * @param m the mount task list pointer
 * @return true on success, false if it fails
 */
bool do_mounts(struct mount_task_list *m)
{
        struct mount_task_list *mtl_ptr = m;
        struct mount_task **array_ptr = NULL;
        struct mount_task *ptr = NULL;
        int mt_ret = 0;
        bool ret = false;

        if (mtl_ptr == NULL) {
                mtl_ptr = &mounts;
        }

        array_ptr = mtl_ptr->mount_tasks;

        while (array_ptr != NULL && *array_ptr != NULL) {
                ptr = *(array_ptr);
                fprintf(stdout, "cyrenit: mounting %s on %s with type %s\n",
                        ptr->source, ptr->target, ptr->fs_type);
                mt_ret = mount(ptr->source, ptr->target, ptr->fs_type,
                                ptr->flags, ptr->data);
                if (mt_ret != 0) {
                        perror("Error: ");
                }
                else {
                        ret = true;
                }

                if (array_ptr - mtl_ptr->mount_tasks == (ptrdiff_t) mtl_ptr->count) {
                        break;
                }

                array_ptr++;
        }

        return ret;
}

#endif//__MOUNTS_C