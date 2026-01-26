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

# Global definitions and settings for the cyrenit build system

MAKE_COMMON_INCLUDED := yes

ifeq ($(origin TOPDIR), undefined)
CYRENIT_BUILD_TREE_ROOT := $(shell pwd)
TOPDIR := $(CYRENIT_BUILD_TREE_ROOT)
else
CYRENIT_BUILD_TREE_ROOT := $(TOPDIR)
TOPDIR := $(TOPDIR)
endif

GLOBAL_DEFINES := -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
GLOBAL_INCLUDES := -I$(CYRENIT_BUILD_TREE_ROOT) -I$(CYRENIT_BUILD_TREE_ROOT)/lib

CPPFLAGS_COMMON_GLOBAL := $(GLOBAL_INCLUDES) $(GLOBAL_DEFINES)
CFLAGS_COMMON_GLOBAL := -std=c11 -Wall -Wextra
LDFLAGS_COMMON_GLOBAL := 
LDLIBS_COMMON_GLOBAL :=

MAKE_VARS_COMMON_GLOBAL := TOPDIR="$(TOPDIR)"

CPPFLAGS_COMMON_DEBUG := -DCONFIG_DEBUG
CFLAGS_COMMON_DEBUG := -g3 -O0 -fno-omit-frame-pointer
LDFLAGS_COMMON_DEBUG :=
LDLIBS_COMMON_DEBUG :=

MAKE_VARS_COMMON_DEBUG :=

# Gets ./configure output -- mandatory
include $(TOPDIR)/config.mk

ifeq ($(CONFIG_DEBUG),1)
CPPFLAGS_COMMON := $(CPPFLAGS_COMMON_GLOBAL) $(CPPFLAGS_COMMON_DEBUG)
CFLAGS_COMMON := $(CFLAGS_COMMON_GLOBAL) $(CFLAGS_COMMON_DEBUG)
LDFLAGS_COMMON := $(LDFLAGS_COMMON_GLOBAL) $(LDFLAGS_COMMON_DEBUG)
LDLIBS_COMMON := $(LDLIBS_COMMON_GLOBAL) $(LDLIBS_COMMON_DEBUG)

MAKE_VARS_COMMON := $(MAKE_VARS_COMMON_GLOBAL) $(MAKE_VARS_COMMON_DEBUG)
else
CPPFLAGS_COMMON := $(CPPFLAGS_COMMON_GLOBAL)
CFLAGS_COMMON := $(CFLAGS_COMMON_GLOBAL)
LDFLAGS_COMMON := $(LDFLAGS_COMMON_GLOBAL)
LDLIBS_COMMON := $(LDLIBS_COMMON_GLOBAL)

MAKE_VARS_COMMON := $(MAKE_VARS_COMMON_GLOBAL)
endif

ifeq ($(CONFIG_DEBUG),1)
CPPFLAGS_COMMON += -DCONFIG_DEBUG
endif

ifneq ($(CONFIG_LOG_RING_SIZE),)
CPPFLAGS_COMMON += -DCONFIG_LOG_RING_SIZE=$(CONFIG_LOG_RING_SIZE)
endif

ifneq ($(CONFIG_LOG_RING_MAXLINE),)
CPPFLAGS_COMMON += -DCONFIG_LOG_RING_MAXLINE=$(CONFIG_LOG_RING_MAXLINE)
endif