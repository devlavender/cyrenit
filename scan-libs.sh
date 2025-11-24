#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
#
# scan-libs.sh <init-root> <services-dir> <service-bins...>
#
# Copies ELF library dependencies into the initramfs tree.
# Very simple implementation: uses `ldd`, resolves paths, copies libs.
# You can refine it later.

set -u

INITROOT="$1"
SERVICESDIR="$2"
shift 2

install_lib() {
        lib="$1"
        dst="$INITROOT/lib"
        mkdir -p "$dst"
        test -x "$lib" && test "$lib" -nt "$dst/$(basename "$lib")" && install "$lib" "$dst/" && echo "Installed"
}

scan_bin(){
        bin="$1"
        echo "Scanning binary $bin"
        test -x "$bin" && ldd "$bin"  2>/dev/null| grep -v 'linux-vdso' | grep 'lib' | while read -r a b c; do
                case "$a $b $c" in
                        *"=>"* )
                                # Format: libfoo.so => /path/to/libfoo.so (0x...)
                                libpath=$(echo "$c" | awk '{print $1}')
                                test -f "$libpath" && install_lib "$libpath" && scan_bin "$libpath"
                                ;;
                        /* )
                                # Direct /lib/libc.so.6 style
                                echo "DIRECT: Should never come here"
                                test -f "$a" && install_lib "$a" && scan_bin "$a"
                                ;;
                esac
        done
        return 0
}

for bin in "$INITROOT"/etc/cyrenit/services/l0/*; do
        scan_bin "$bin"
done

scan_bin "$INITROOT/sbin/cyrenit"