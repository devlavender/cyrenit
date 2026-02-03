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

CPPFLAGS := $(CPPFLAGS_COMMON) $(CPPFLAGS_TESTS)
CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_TESTS)
LDFLAGS := $(LDFLAGS_COMMON) $(LDFLAGS_TESTS)
LDLIBS := $(LDLIBS_COMMON) $(LDLIBS_TESTS)

OBJECTS := $(wildcard *.o)
SUBDIRS := lib

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

all: $(OBJECTS) $(SUBDIRS) test-bin

lib:
	$(MAKE) -C lib/ $(MAKE_VARS)

TEST_OBJECTS := $(find . -name "*-test.o")

test-bin: $(OBJECTS) $(SUBDIRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_OBJECTS) -o cyrenit-test-bin $(LDFLAGS) $(LDLIBS)

test: test-bin
	./cyrenit-test-bin $(TEST_FLAGS)