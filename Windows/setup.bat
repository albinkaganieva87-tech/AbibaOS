@echo off

echo ======================================
echo          AbibaOS setup v1.0
echo ======================================
echo.

where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] GCC was not found.
    echo.
    echo Install MinGW-w64 or use WSL.
    pause
    exit /b 1
)

where nasm >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] NASM was not found.
    echo.
    echo Install NASM and add it to PATH.
    pause
    exit /b 1
)

where make >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] Make was not found.
    pause
    exit /b 1
)

where qemu-system-i386 >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] QEMU was not found.
    pause
    exit /b 1
)

echo.
echo [OK] All required tools found.
echo.
echo AbibaOS is ready to build.
echo.

pause
