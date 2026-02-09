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

AC_PREREQ([2.69])

AC_INIT([cyrenit], [0.0.1], [code@agatha.dev])
AC_CONFIG_SRCDIR([Makefile])

AC_PROG_CC

AC_ARG_ENABLE([debug],
        [AS_HELP_STRING([--enable-debug],
                [Build with debug symbols and no optimizations])],
        [CONFIG_DEBUG=yes],
        [CONFIG_DEBUG=no])

AC_ARG_ENABLE([tests],
        AS_HELP_STRING([--enable-tests], [Enable unit testing]),
        [CONFIG_TESTS=$enableval],
        [CONFIG_TESTS=no])

AC_ARG_ENABLE([service-helloop],
        [AS_HELP_STRING([--enable-service-helloop],
                [Build with helloop service])],
        [CONFIG_SERVICE_HELLOOP=yes],
        [CONFIG_SERVICE_HELLOOP=no])

AC_ARG_ENABLE([service-orphan-party],
        [AS_HELP_STRING([--enable-service-orphan-party],
                [Build with orphan-party service])],
        [CONFIG_SERVICE_ORPHAN_PARTY=yes],
        [CONFIG_SERVICE_ORPHAN_PARTY=no])

AC_ARG_ENABLE([service-zombie-party],
        [AS_HELP_STRING([--enable-service-zombie-party],
                [Build with zombie-party service])],
        [CONFIG_SERVICE_ZOMBIE_PARTY=yes],
        [CONFIG_SERVICE_ZOMBIE_PARTY=no])

AC_ARG_WITH([log-ring-size],
        [AS_HELP_STRING([--with-log-ring-size=SIZE],
                [Set the size of the in-memory log ring buffer (default: 256K)])],
        [CONFIG_LOG_RING_SIZE="$withval"],
        [CONFIG_LOG_RING_SIZE="256K"])

AC_ARG_WITH([log-ring-maxline],
        [AS_HELP_STRING([--with-log-ring-maxline=SIZE],
                [Set the maximum line size for the in-memory log ring buffer (default: 120)])],
        [CONFIG_LOG_RING_MAXLINE="$withval"],
        [CONFIG_LOG_RING_MAXLINE="120"])

AC_ARG_WITH([qemu],
        [AS_HELP_STRING([--with-qemu=QEMU],
                [Set the path to the QEMU binary for testing (default: qemu-system-x86_64)])],
        [QEMU_CHECK="$withval"],
        [QEMU_CHECK="qemu-system-x86_64"])

AC_SUBST(CONFIG_DEBUG)
AC_SUBST(CONFIG_TESTS)
AC_SUBST(CONFIG_SERVICE_HELLOOP)
AC_SUBST(CONFIG_SERVICE_ORPHAN_PARTY)
AC_SUBST(CONFIG_SERVICE_ZOMBIE_PARTY)
AC_SUBST(CONFIG_LOG_RING_SIZE)
AC_SUBST(CONFIG_LOG_RING_MAXLINE)

AC_SUBST(QEMU)

AS_IF([test "x$CONFIG_DEBUG" = "xyes"],
      [AC_DEFINE([CONFIG_DEBUG], [1], [Build with debug symbols and no optimizations])])

AS_IF([test "x$CONFIG_TESTS" = "xyes"],
        [
                PKG_CHECK_MODULES([CRITERION], [criterion], [
                        AC_DEFINE([CONFIG_TESTS], [1], [Enable unit testing])
                ], [
                        AC_MSG_ERROR([Criterion library not found, required for building tests])
                ])
        ])

AS_IF([test "x$CONFIG_SERVICE_HELLOOP" = "xyes"],
      [AC_DEFINE([CONFIG_SERVICE_HELLOOP], [1], [Build with helloop service])])

AS_IF([test "x$CONFIG_SERVICE_ORPHAN_PARTY" = "xyes"],
      [AC_DEFINE([CONFIG_SERVICE_ORPHAN_PARTY], [1], [Build with orphan-party service])])

AS_IF([test "x$CONFIG_SERVICE_ZOMBIE_PARTY" = "xyes"],
      [AC_DEFINE([CONFIG_SERVICE_ZOMBIE_PARTY], [1], [Build with zombie-party service])])

AS_IF([test "x$CONFIG_LOG_RING_SIZE" != "x"],
        [
                lastchr=${CONFIG_LOG_RING_SIZE#${CONFIG_LOG_RING_SIZE%?}}
                multiplier=1

                AC_MSG_CHECKING([for log ring size multiplier in $CONFIG_LOG_RING_SIZE])

                case "$lastchr" in
                        K|k|M|m|G|g)
                                AC_MSG_RESULT([found $lastchr])
                                strip_last="yes"
                        ;;
                        0|1|2|3|4|5|6|7|8|9)
                                AC_MSG_RESULT([not found])
                                strip_last="no"
                        ;;
                        *)
                                AC_MSG_ERROR([Log ring size multiplier invalid: $lastchr])
                        ;;
                esac

                test "x$strip_last" = "xyes" && CONFIG_LOG_RING_SIZE=${CONFIG_LOG_RING_SIZE%?} &&
                        AC_MSG_NOTICE([Log ring size number token: $CONFIG_LOG_RING_SIZE])

                case "$lastchr" in
                        K|k)
                                multiplier=1024
                        ;;
                        M|m)
                                multiplier=$((1024 * 1024))
                        ;;
                        G|g)
                                multiplier=$((1024 * 1024 * 1024))
                        ;;
                        *)
                                multiplier=1
                        ;;
                esac

                test $multiplier -ne 1 && AC_MSG_NOTICE([Log ring size multiplier: $lastchr][B ($multiplier)])

                AC_MSG_CHECKING([for the final log ring size in bytes])
                CONFIG_LOG_RING_SIZE=$((CONFIG_LOG_RING_SIZE * multiplier))
                AC_MSG_RESULT([$CONFIG_LOG_RING_SIZE])

                AC_DEFINE_UNQUOTED([CONFIG_LOG_RING_SIZE],
                        [$CONFIG_LOG_RING_SIZE],
                        [Size of the in-memory log ring buffer in bytes])
        ]);

AS_IF([test "x$CONFIG_LOG_RING_MAXLINE" != "x"],
        [AC_DEFINE_UNQUOTED([CONFIG_LOG_RING_MAXLINE],
                [$CONFIG_LOG_RING_MAXLINE],
                [Maximum line size for the in-memory log ring buffer])])

AC_PATH_PROG([QEMU_PATH], [$QEMU_CHECK], [no])
AS_IF([test "x$QEMU_PATH" != "xno"],
        [
                AC_DEFINE_UNQUOTED([QEMU], ["$QEMU_PATH"], [Path to the QEMU binary for running cyrenit])
                AC_MSG_NOTICE([QEMU binary $QEMU_CHECK found at $QEMU_PATH, running enabled])
        ],
        [
                AC_DEFINE_UNQUOTED([QEMU], [""], [QEMU binary not found, running will be disabled])
                AC_MSG_WARN([QEMU binary $QEMU_CHECK not found, running will be disabled])
        ])

AC_CONFIG_FILES([config.mk])

AH_TOP([/* AUto-generated by configure -- do not edit */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_DONE
])

AC_CONFIG_HEADERS([config.h])

AH_BOTTOM([
#endif//__CONFIG_H
])

AC_OUTPUT