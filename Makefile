# SPDX-License-Identifier: GPL-3.0-or-later
	
# cyrenit - Minimal init system for experimental initramfs environments
# Copyright (C) 2025  Ágatha Isabelle Moreira Guedes <code@agatha.dev>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# A copy of the license is also provided in the file named LICENSE
# distributed with the source code.

# Compiler and flags
DEFINES := -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
CC      := cc
CFLAGS_REG  := -O2 
CFLAGS_DEBUG  := -g3 -O0 -fno-omit-frame-pointer -Wall -Wextra
CFLAGS_COMMON := -static -std=c11 -Wall -Wextra $(DEFINES)
LDFLAGS_REG :=
LDFLAGS_DEBUG :=

ifeq ($(DEBUG),1)
CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
LDFLAGS := $(LDFLAGS_DEBUG)
else
CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_REG)
LDFLAGS := $(LDFLAGS_REG)
endif

# Kernel image for initcpio
KERNEL  := /boot/vmlinuz-linux

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

SERVICES_DIR := services
SERVICE_BINS := $(shell make -s -C $(SERVICES_DIR) -qp | awk '/^[a-zA-Z0-9].*: .*\.c/ {print $$1}' | sort -u)

CONFIG_DIR := /etc/cyrenit
SERVICES_DEST_DIR := $(CONFIG_DIR)/services/l0
CYRENIT_DEST_DIR := /sbin

all: cyrenit services

cyrenit: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cyrenit $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

services:
	$(MAKE) -C $(SERVICES_DIR)

IMAGE_BUILD_DIR := build/initcpio
TARGET_IMAGE := initrd.cpio
CYRENIT_BIN := $(CYRENIT_DEST_DIR)/cyrenit
INIT_BIN := $(IMAGE_BUILD_DIR)$(CYRENIT_DEST_DIR)/init
IMAGE_BINS := bash ls find mount umount df cp mv rm dmesg mkdir \
	touch cat tail ln ps kill
IMAGE_LIBS := libgcc_s.so.1
LIBDIR_SYMLINKS := /usr/lib /lib64 /usr/lib64
ROOT_DIRS := {sbin,bin,boot,var,lib,etc,proc,sys,dev,mnt,run,usr,tmp}

initcpio: all scan-libs.sh
	mkdir -p $(IMAGE_BUILD_DIR)/$(ROOT_DIRS)
	for dir in $(LIBDIR_SYMLINKS); do \
		ln -s /lib $(IMAGE_BUILD_DIR)/$$dir ; \
	done
	install cyrenit $(IMAGE_BUILD_DIR)$(CYRENIT_DEST_DIR)
	test -L $(INIT_BIN) || ln -s $(CYRENIT_BIN) $(INIT_BIN)

	mkdir -p $(IMAGE_BUILD_DIR)$(CONFIG_DIR)
	mkdir -p $(IMAGE_BUILD_DIR)$(SERVICES_DEST_DIR)
	chmod -R 700 $(IMAGE_BUILD_DIR)$(CONFIG_DIR)
	chmod -R 700 $(IMAGE_BUILD_DIR)$(SERVICES_DEST_DIR)
	for bin in $(SERVICES_DIR)/* ; do \
		test -x $$bin && install $$bin $(IMAGE_BUILD_DIR)$(SERVICES_DEST_DIR); \
		continue; \
	done
	for bin in $(IMAGE_BINS); do \
		which $$bin >/dev/null 2>&1 && install `which $$bin` $(IMAGE_BUILD_DIR)/bin/ ; \
		continue; \
	done
	for lib in $(IMAGE_LIBS); do \
		test -a /lib/$$lib && install /lib/$$lib $(IMAGE_BUILD_DIR)/lib/; \
		continue; \
	done

	bash scan-libs.sh $(IMAGE_BUILD_DIR) $(SERVICES_DIR)

CPIO_FLAGS := --owner root:root --null -ov --format=newc
$(TARGET_IMAGE): initcpio
	( cd $(IMAGE_BUILD_DIR) && find . -print0 | doas cpio $(CPIO_FLAGS) ) > $(TARGET_IMAGE)


KERNEL_CMD_CONSOLE := console=ttyS0
KERNEL_CMD_INIT := init=$(CYRENIT_DEST_DIR)/init rdinit=$(CYRENIT_DEST_DIR)/init
KERNEL_CMD_ROOT := root=
KERNEL_CMD_DEBUG := debug
KERNEL_CMDLINE := $(KERNEL_CMD_CONSOLE) $(KERNEL_CMD_INIT) $(KERNEL_CMD_ROOT) $(KERNEL_CMD_DEBUG)

QEMU_CONSOLE_OPTS := -display none -serial stdio
QEMU_OPTS_DEBUG := -s -S
QEMU_OPTS_REG :=
ifeq ($(DEBUG),1)
QEMU_OPTS := $(QEMU_OPTS_DEBUG)
else
QEMU_OPTS := $(QEMU_OPTS_REG)
endif

run: $(TARGET_IMAGE)
	echo "Starting qemu with $(KERNEL) kernel, $(TARGET_IMAGE) initrd, " \
		"console opts: '$(QEMU_CONSOLE_OPTS)' and kernel cmdline: $(KERNEL_CMDLINE)"
	qemu-system-x86_64 -kernel $(KERNEL) -initrd $(TARGET_IMAGE) \
		$(QEMU_CONSOLE_OPTS) -append "$(KERNEL_CMDLINE)" \
		$(QEMU_OPTS)

clean:
	rm -f cyrenit *.o
	rm -rf build
	$(MAKE) -C $(SERVICES_DIR) clean

.PHONY: all services initcpio clean run
