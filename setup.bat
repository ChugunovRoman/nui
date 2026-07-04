@echo off
REM ──────────────────────────────────────────────────────────────
REM  NUI: First-time setup
REM  Initializes submodules and checks out correct tags.
REM  After this, just open nui.sln and build.
REM ──────────────────────────────────────────────────────────────
setlocal

set SCRIPT_DIR=%~dp0

echo ============================================
echo   NUI - First-time Setup
echo ============================================
echo.

REM ── Step 1: Init submodules ───────────────────────────────────
echo [1/3] Initializing git submodules...
cd /d "%SCRIPT_DIR%"
git submodule update --init --recursive
if errorlevel 1 (
    echo ERROR: git submodule init failed.
    exit /b 1
)

REM ── Step 2: Checkout release tags ─────────────────────────────
echo.
echo [2/3] Checking out release tags...
cd /d "%SCRIPT_DIR%Externals\SDL2"
git fetch --depth 1 origin tag release-2.30.12 2>nul
git checkout release-2.30.12 2>nul

cd /d "%SCRIPT_DIR%Externals\SDL_ttf"
git fetch --depth 1 origin tag release-2.22.0 2>nul
git checkout release-2.22.0 2>nul

cd /d "%SCRIPT_DIR%Externals\pugixml"
git fetch --depth 1 origin tag v1.14 2>nul
git checkout v1.14 2>nul

cd /d "%SCRIPT_DIR%"

REM ── Step 3: Download stb_image.h ──────────────────────────────
echo.
echo [3/3] Checking stb_image.h...
if not exist "%SCRIPT_DIR%src\renderer\stb_image.h" (
    powershell -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile '%SCRIPT_DIR%src\renderer\stb_image.h'"
    echo       Downloaded.
) else (
    echo       Already exists.
)

echo.
echo ============================================
echo   Setup complete!
echo.
echo   Open nui.sln in Visual Studio
echo   Select Release ^| x64 and build.
echo.
echo   First build will run CMake to configure
echo   dependencies. Subsequent builds are fast.
echo ============================================
pause
