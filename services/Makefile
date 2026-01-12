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

include $(TOPDIR)/common.mk

SRCS := $(wildcard *.c)
ENABLED_BINS := 

ifeq ($(CONFIG_SERVICE_HELLOOP),yes)
ENABLED_BINS += helloop
endif

ifeq ($(CONFIG_SERVICE_ORPHAN_PARTY),yes)
ENABLED_BINS += orphan-party
endif

ifeq ($(CONFIG_SERVICE_ZOMBIE_PARTY),yes)
ENABLED_BINS += zombie-party
endif

BINS := $(ENABLED_BINS)
CFLAGS := $(CFLAGS_COMMON)
CPPFLAGS := $(CPPFLAGS_COMMON)

all: $(BINS)
	echo "Building services $(BINS)"

%: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(BINS)

.PHONY: all clean