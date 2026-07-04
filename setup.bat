@echo off
REM ──────────────────────────────────────────────────────────────
REM  NUI: Visual Studio Setup Script
REM  Downloads vcpkg, installs dependencies, downloads stb_image.
REM  Run this once before opening the .sln in Visual Studio.
REM ──────────────────────────────────────────────────────────────
setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0

echo ============================================
echo   NUI - Visual Studio Setup
echo ============================================
echo.

REM ── Step 1: Clone or update vcpkg ─────────────────────────────
if exist "%SCRIPT_DIR%vcpkg\vcpkg.exe" (
    echo [1/3] vcpkg already installed, updating...
    cd /d "%SCRIPT_DIR%vcpkg"
    git pull >nul 2>&1
    call bootstrap-vcpkg.bat -disableMetrics
) else (
    echo [1/3] Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git "%SCRIPT_DIR%vcpkg"
    if errorlevel 1 (
        echo ERROR: Failed to clone vcpkg. Is git installed?
        exit /b 1
    )
    cd /d "%SCRIPT_DIR%vcpkg"
    call bootstrap-vcpkg.bat -disableMetrics
    if errorlevel 1 (
        echo ERROR: Failed to bootstrap vcpkg.
        exit /b 1
    )
)

REM ── Step 2: Install dependencies ──────────────────────────────
echo.
echo [2/3] Installing dependencies (SDL2, SDL2-ttf, pugixml)...
echo       This may take several minutes on first run.
echo.
"%SCRIPT_DIR%vcpkg\vcpkg.exe" install --triplet x64-windows-static --triplet x86-windows-static --x-manifest-root="%SCRIPT_DIR%" --x-install-root="%SCRIPT_DIR%vcpkg\installed"
if errorlevel 1 (
    echo ERROR: vcpkg install failed.
    exit /b 1
)

REM ── Step 3: Download stb_image.h ──────────────────────────────
echo.
echo [3/3] Downloading stb_image.h...
if not exist "%SCRIPT_DIR%src\renderer\stb_image.h" (
    powershell -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile '%SCRIPT_DIR%src\renderer\stb_image.h'"
    if errorlevel 1 (
        echo WARNING: Failed to download stb_image.h. Download manually from:
        echo   https://github.com/nothings/stb/blob/master/stb_image.h
    ) else (
        echo       stb_image.h downloaded successfully.
    )
) else (
    echo       stb_image.h already exists.
)

echo.
echo ============================================
echo   Setup complete!
echo.
echo   Open nui.sln in Visual Studio
echo   and build (Release x64 recommended).
echo.
echo   Make sure VCPKG_ROOT is NOT set globally
echo   or the local vcpkg will be used via
echo   MSBuild integration in the .vcxproj files.
echo ============================================
pause
