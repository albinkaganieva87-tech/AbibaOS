#!/bin/bash

set -e

echo "======================================"
echo "        AbibaOS setup v1.0"
echo "======================================"
echo

echo "[*] Checking operating system..."

OS="$(uname -s)"

case "$OS" in
    Linux*)
        echo "[OK] Linux detected"
        ;;

    Darwin*)
        echo "[OK] macOS detected"
        ;;

    *)
        echo "[ERROR] Unsupported operating system"
        exit 1
        ;;
esac


echo
echo "[*] Checking required tools..."

check_command()
{
    if command -v "$1" >/dev/null 2>&1; then
        echo "[OK] $1"
    else
        echo "[MISSING] $1"
        return 1
    fi
}


MISSING=0

check_command gcc || MISSING=1
check_command ld || MISSING=1
check_command nasm || MISSING=1
check_command make || MISSING=1
check_command qemu-system-i386 || MISSING=1


if [ "$MISSING" -ne 0 ]; then
    echo
    echo "[ERROR] Some dependencies are missing."
    echo

    if [ "$OS" = "Linux" ]; then
        echo "On Debian/Ubuntu you can install them with:"
        echo
        echo "sudo apt install build-essential gcc-multilib nasm qemu-system-x86"
    fi

    if [ "$OS" = "Darwin" ]; then
        echo "Install Homebrew and run:"
        echo
        echo "brew install nasm qemu"
    fi

    exit 1
fi


echo
echo "[*] Creating build directory..."

mkdir -p build


echo
echo "[*] Setup complete!"
echo
echo "You can now build AbibaOS with:"
echo
echo "    make"
echo
echo "And run it with:"
echo
echo "    qemu-system-i386 -kernel kernel.elf"
echo
