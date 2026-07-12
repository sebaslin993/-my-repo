
@echo off
setlocal

where emcc >nul 2>&1
if errorlevel 1 (
    echo ERROR: Emscripten is not active. Run C:\emsdk\emsdk_env.bat first.
    exit /b 1
)

if not exist external\glm\glm\glm.hpp (
    echo Fetching GLM 1.0.1 ...
    if not exist external mkdir external
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external\glm
    if errorlevel 1 exit /b 1
)

if not exist web mkdir web

emcc -std=c++17 ^
    Scene/main.cpp ^
    Scene/Renderer.cpp ^
    Scene/Fan.cpp ^
    Scene/Transform.cpp ^
    -IScene ^
    -Iexternal/glm ^
    -DUSE_GLFW ^
    -s USE_GLFW=3 ^
    -s USE_WEBGL2=1 ^
    -s FULL_ES3=1 ^
    -s WASM=1 ^
    --preload-file android/app/src/main/assets/shader@assets/shader ^
    -o web/index.html

if errorlevel 1 (
    echo Build FAILED.
    exit /b 1
)

echo.
echo Web build succeeded: web\index.html
echo Run: cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/
endlocal


