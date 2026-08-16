#!/bin/bash

set -e

echo "======================================"
echo "        AbibaOS build v1.0"
echo "======================================"
echo

echo "[1/3] Cleaning old build..."

make clean


echo
echo "[2/3] Building kernel..."

make


echo
echo "[3/3] Build successful!"
echo
echo "Output:"
echo "    kernel.elf"
echo
