#!/usr/bin/env sh
set -eu

if ! command -v emcc >/dev/null 2>&1; then
    echo "ERROR: Emscripten is not active. Source emsdk_env.sh first."
    exit 1
fi

if [ ! -f external/glm/glm/glm.hpp ]; then
    echo "Fetching GLM 1.0.1 ..."
    mkdir -p external
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
fi

mkdir -p web
emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Scene3D.cpp \
    Scene/SceneHUD.cpp \
    Scene/Fan.cpp \
    Scene/Ground.cpp \
    Scene/Transform.cpp \
    -IScene \
    -Iexternal/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    --preload-file Models@Models \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

echo "Web build succeeded: web/index.html"
echo "Run: cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
