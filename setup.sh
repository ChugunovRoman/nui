#!/bin/bash
# ──────────────────────────────────────────────────────────────
#  NUI: First-time setup (Linux / macOS)
#  Initializes submodules and checks out correct tags.
#  After this, just run: cmake -B build && cmake --build build
# ──────────────────────────────────────────────────────────────
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "============================================"
echo "  NUI - First-time Setup"
echo "============================================"
echo

# ── Step 1: Init submodules ─────────────────────────────────────
echo "[1/3] Initializing git submodules..."
cd "$SCRIPT_DIR"
git submodule update --init --recursive

# ── Step 2: Checkout release tags ───────────────────────────────
echo
echo "[2/3] Checking out release tags..."

cd "$SCRIPT_DIR/Externals/SDL2"
git fetch --depth 1 origin tag release-2.30.12 2>/dev/null || true
git checkout release-2.30.12 2>/dev/null || true

cd "$SCRIPT_DIR/Externals/SDL_ttf"
git fetch --depth 1 origin tag release-2.22.0 2>/dev/null || true
git checkout release-2.22.0 2>/dev/null || true

cd "$SCRIPT_DIR/Externals/pugixml"
git fetch --depth 1 origin tag v1.14 2>/dev/null || true
git checkout v1.14 2>/dev/null || true

cd "$SCRIPT_DIR"

# ── Step 3: Download stb_image.h ────────────────────────────────
echo
echo "[3/3] Checking stb_image.h..."
if [ ! -f "$SCRIPT_DIR/src/renderer/stb_image.h" ]; then
    curl -sL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" \
        -o "$SCRIPT_DIR/src/renderer/stb_image.h"
    echo "      Downloaded."
else
    echo "      Already exists."
fi

echo
echo "============================================"
echo "  Setup complete!"
echo
echo "  Build with:"
echo "    cmake -B build -DCMAKE_BUILD_TYPE=Release"
echo "    cmake --build build -j\$(nproc)"
echo
echo "  Run:"
echo "    ./build/bin/nui-example"
echo "============================================"
