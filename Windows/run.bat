@echo off

if not exist kernel.elf (
    echo [*] kernel.elf not found.
    echo [*] Building...
    make

    if %errorlevel% neq 0 (
        echo.
        echo [ERROR] Build failed.
        pause
        exit /b 1
    )
)

echo.
echo ======================================
echo          Starting AbibaOS
echo ======================================
echo.

qemu-system-i386 -kernel kernel.elf -display gtk
