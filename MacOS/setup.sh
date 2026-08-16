#!/bin/bash

set -e

echo "======================================"
echo "        AbibaOS macOS setup v1.0"
echo "======================================"
echo

# ---------------------------------------------------------
# Check macOS
# ---------------------------------------------------------

if [[ "$(uname)" != "Darwin" ]]; then
    echo "[ERROR] This script is for macOS only."
    exit 1
fi

echo "[OK] macOS detected"


# ---------------------------------------------------------
# Check Homebrew
# ---------------------------------------------------------

echo
echo "[*] Checking Homebrew..."

if ! command -v brew >/dev/null 2>&1; then

    echo "[MISSING] Homebrew"

    echo
    echo "Please install Homebrew first:"
    echo
    echo "https://brew.sh"
    echo

    exit 1
fi

echo "[OK] Homebrew"


# ---------------------------------------------------------
# Install dependencies
# ---------------------------------------------------------

echo
echo "[*] Installing dependencies..."

brew install nasm
brew install qemu


# ---------------------------------------------------------
# Check compiler
# ---------------------------------------------------------

echo
echo "[*] Checking compiler..."

if command -v gcc >/dev/null 2>&1; then
    echo "[OK] gcc"
else
    echo "[ERROR] gcc was not found."
    echo
    echo "Install Xcode Command Line Tools:"
    echo
    echo "xcode-select --install"
    echo
    exit 1
fi


# ---------------------------------------------------------
# Check Make
# ---------------------------------------------------------

if command -v make >/dev/null 2>&1; then
    echo "[OK] make"
else
    echo "[ERROR] make was not found."
    exit 1
fi


# ---------------------------------------------------------
# Check NASM
# ---------------------------------------------------------

if command -v nasm >/dev/null 2>&1; then
    echo "[OK] nasm"
else
    echo "[ERROR] nasm was not found."
    exit 1
fi


# ---------------------------------------------------------
# Check QEMU
# ---------------------------------------------------------

if command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "[OK] qemu-system-i386"
else
    echo "[ERROR] qemu-system-i386 was not found."
    exit 1
fi


# ---------------------------------------------------------
# Finish
# ---------------------------------------------------------

echo
echo "======================================"
echo "        Setup completed!"
echo "======================================"
echo

echo "Build:"
echo
echo "    make"
echo

echo "Run:"
echo
echo "    qemu-system-i386 -kernel kernel.elf"
echo

echo "Or:"
echo
echo "    ./scripts/run.sh"
echo
