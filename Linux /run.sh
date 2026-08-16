#!/bin/bash

set -e

if [ ! -f kernel.elf ]; then
    echo "[!] kernel.elf not found."
    echo "[*] Building AbibaOS..."
    make
fi

echo
echo "======================================"
echo "          Starting AbibaOS"
echo "======================================"
echo

qemu-system-i386 \
    -kernel kernel.elf \
    -display gtk
