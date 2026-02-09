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

# Gets ./configure output -- mandatory
ifeq ($(MAKE_COMMON_INCLUDED),yes)
else
include common.mk
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

CYRENIT_CPPFLAGS := 
CYRENIT_CFLAGS := 

CYRENIT_LIBS := libcyrenit_logger.a
LIBS := $(CYRENIT_LIBS)
LIB_DIR := lib/

CPPFLAGS := $(CPPFLAGS_COMMON) $(CYRENIT_CPPFLAGS)
CFLAGS := $(CFLAGS_COMMON) $(CYRENIT_CFLAGS)

MAKE_VARS := $(MAKE_VARS_COMMON)

all: cyrenit services lib test-dir

cyrenit: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cyrenit $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

services:
	@echo "Passing make vars: $(MAKE_VARS)"
	$(MAKE) -C $(SERVICES_DIR) $(MAKE_VARS)

lib:
	$(MAKE) -C lib/ $(MAKE_VARS)

test-dir:
	$(MAKE) -C test/ $(MAKE_VARS)

test: test-dir
	$(MAKE) -C test/ $(MAKE_VARS) test

IMAGE_BUILD_DIR := build/initcpio
TARGET_IMAGE := initrd.cpio
CYRENIT_BIN := $(CYRENIT_DEST_DIR)/cyrenit
INIT_BIN := $(IMAGE_BUILD_DIR)$(CYRENIT_DEST_DIR)/init
IMAGE_BINS := bash ls find mount umount df cp mv rm dmesg mkdir \
	touch cat tail ln ps kill ldd pstree grep sed awk free
IMAGE_DATA := /usr/share/terminfo
IMAGE_LIBS := libgcc_s.so.1
LIBDIR_SYMLINKS := /usr/lib /lib64 /usr/lib64
ROOT_DIRS := {sbin,bin,boot,var,lib,etc,proc,sys,dev,mnt,run,usr,tmp}
USR_DIRS := {share,libexec}

tests:
	$(MAKE) -C tests/ $(MAKE_VARS)

initcpio: all scan-libs.sh
	mkdir -p $(IMAGE_BUILD_DIR)/$(ROOT_DIRS)
	mkdir -p $(IMAGE_BUILD_DIR)/usr/$(USR_DIRS)
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
	for data in $(IMAGE_DATA); do \
		(test -d $$data || test -f $$data) && \
			cp -a $$data $(IMAGE_BUILD_DIR)/$$(dirname $$data); \
		continue; \
	done

	bash scan-libs.sh $(IMAGE_BUILD_DIR) $(SERVICES_DIR)

CPIO_FLAGS := --owner root:root --null -ov --format=newc
$(TARGET_IMAGE): initcpio
	( cd $(IMAGE_BUILD_DIR) && find . -print0 | doas cpio $(CPIO_FLAGS) ) > $(TARGET_IMAGE)


KERNEL_CMD_CONSOLE := console=ttyS0
KERNEL_CMD_INIT := init=$(CYRENIT_DEST_DIR)/init rdinit=$(CYRENIT_DEST_DIR)/init
KERNEL_CMD_ROOT := root=

ifeq ($(KERN_CONS),1)

ifndef LOGLEVEL
LOGLEVEL := 7
endif

KERNEL_CMD_DEBUG := debug loglevel=$(LOGLEVEL)
else

ifndef LOGLEVEL
LOGLEVEL := 3
endif

KERNEL_CMD_DEBUG := quiet loglevel=$(LOGLEVEL)
endif

KERNEL_CMDLINE := $(KERNEL_CMD_CONSOLE) $(KERNEL_CMD_INIT) $(KERNEL_CMD_ROOT) $(KERNEL_CMD_DEBUG)

QEMU_CONSOLE_OPTS := -display none -serial stdio
QEMU_OPTS_DEBUG := -s -S
QEMU_OPTS_REG :=
ifeq ($(QEMU_DEBUG),1)
QEMU_OPTS := $(QEMU_OPTS_DEBUG)
else
QEMU_OPTS := $(QEMU_OPTS_REG)
endif

run: $(TARGET_IMAGE)
ifeq ($(QEMU),no)
	@echo "Running is disabled because it's either been disabled at" \
		" configure time or the binary wasn't found in your" \
		" system."
	@echo "To enable running configure again provind a valid qemu" \
		" binary using the --with-qemu option."
else
	@echo "Starting qemu with $(KERNEL) kernel, $(TARGET_IMAGE) initrd, " \
		"console opts: '$(QEMU_CONSOLE_OPTS)' and kernel cmdline: $(KERNEL_CMDLINE)"
	$(QEMU) -kernel $(KERNEL) -initrd $(TARGET_IMAGE) \
		$(QEMU_CONSOLE_OPTS) -append "$(KERNEL_CMDLINE)" \
		$(QEMU_OPTS)
endif

clean:
	rm -f cyrenit *.o
	rm -rf build
	rm -rf $(TARGET_IMAGE)
	$(MAKE) -C $(SERVICES_DIR) $(MAKE_VARS) clean
	$(MAKE) -C $(LIB_DIR) $(MAKE_VARS) clean

dist-clean: clean
	rm -rf config.mk config.h config.h.in configure config.log \
		config.status autom4te.cache configure~ config.h.in~ \
		aclocal.m4 compile_commands.json

.PHONY: all services initcpio clean run
