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
CC      := cc
CFLAGS  := -static -O2 -Wall -Wextra -std=c11
LDFLAGS :=

# Kernel image for initcpio
KERNEL  := /boot/vmlinuz-linux

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

SERVICES_DIR := services
SERVICE_BINS := $(shell make -s -C $(SERVICES_DIR) -qp | awk '/^[a-zA-Z0-9].*: .*\.c/ {print $$1}' | sort -u)

SERVICES_DEST_DIR := /etc/cyrenit/services/l0
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

initcpio: all scan-libs.sh
	rm -f $(TARGET_IMAGE)
	mkdir -p $(IMAGE_BUILD_DIR)/{sbin,bin,boot,var,lib,etc,proc,sys,dev,mnt}
	install cyrenit $(IMAGE_BUILD_DIR)$(CYRENIT_DEST_DIR)
	ln -s $(CYRENIT_DEST_DIR)/cyrenit $(IMAGE_BUILD_DIR)$(CYRENIT_DEST_DIR)/init

	mkdir -p $(IMAGE_BUILD_DIR)$(SERVICES_DEST_DIR)
	@for bin in $(SERVICES_DIR)/* ; do \
		test -x $$bin && install $$bin $(IMAGE_BUILD_DIR)$(SERVICES_DEST_DIR); \
		continue; \
	done
	bash scan-libs.sh $(IMAGE_BUILD_DIR) $(SERVICES_DIR)

$(TARGET_IMAGE): initcpio
	( cd $(IMAGE_BUILD_DIR) && find . -print0 | cpio --null -ov --format=newc ) > $(TARGET_IMAGE)


KERNEL_CMD_CONSOLE := console=ttyS0
KERNEL_CMD_INIT := init=$(CYRENIT_DEST_DIR)/cyrenit rdinit=$(CYRENIT_DEST_DIR)/cyrenit
KERNEL_CMD_ROOT := root=
KERNEL_CMD_DEBUG := debug
KERNEL_CMDLINE := $(KERNEL_CMD_CONSOLE) $(KERNEL_CMD_INIT) $(KERNEL_CMD_ROOT) $(KERNEL_CMD_DEBUG)

QEMU_CONSOLE_OPTS := -display none -serial stdio 

run: $(TARGET_IMAGE)
	echo "Starting qemu with $(KERNEL) kernel, $(TARGET_IMAGE) initrd, " \
		"console opts: '$(QEMU_CONSOLE_OPTS)' and kernel cmdline: $(KERNEL_CMDLINE)"
	qemu-system-x86_64 -kernel $(KERNEL) -initrd $(TARGET_IMAGE) \
		$(QEMU_CONSOLE_OPTS) -append "$(KERNEL_CMDLINE)"

clean:
	rm -f cyrenit *.o
	rm -rf build
	$(MAKE) -C $(SERVICES_DIR) clean

.PHONY: all services initcpio clean run
