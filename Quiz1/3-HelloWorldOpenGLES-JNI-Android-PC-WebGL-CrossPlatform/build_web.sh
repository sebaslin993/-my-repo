#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build GLPIFrameworkIntro-Modern for WebGL using Emscripten
# =============================================================================

set -e

mkdir -p web

emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Square.cpp \
    Scene/Triangle.cpp \
    -IScene \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s MIN_WEBGL_VERSION=2 \
    -s MAX_WEBGL_VERSION=2 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

# SDL2 fallback, if needed:
# emcc -std=c++17 \
#     Scene/main.cpp \
#     Scene/Renderer.cpp \
#     Scene/Square.cpp \
#     Scene/Triangle.cpp \
#     -IScene \
#     -s USE_SDL=2 \
#     -s USE_WEBGL2=1 \
#     -s MIN_WEBGL_VERSION=2 \
#     -s MAX_WEBGL_VERSION=2 \
#     -s FULL_ES3=1 \
#     -s WASM=1 \
#     -s ALLOW_MEMORY_GROWTH=1 \
#     --preload-file android/app/src/main/assets/shader@assets/shader \
#     -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"

