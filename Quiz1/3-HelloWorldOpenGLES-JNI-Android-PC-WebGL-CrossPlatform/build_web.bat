@echo off
REM =============================================================================
REM build_web.bat - Build GLPIFrameworkIntro-Modern for WebGL (Emscripten/Windows)
REM =============================================================================

SET EMSDK_PATH=C:\emsdk
CALL "%EMSDK_PATH%\emsdk_env.bat"

IF NOT EXIST web MKDIR web

REM --- GLFW backend (default) -----------------------------------------------
emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Square.cpp ^
     Scene/Triangle.cpp ^
     -IScene ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s MIN_WEBGL_VERSION=2 ^
     -s MAX_WEBGL_VERSION=2 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     -s ALLOW_MEMORY_GROWTH=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

REM --- SDL2 backend (opt-in) ------------------------------------------------
REM To use SDL2 instead, comment out the GLFW block above and uncomment below:
REM emcc -std=c++17 ^
REM      Scene/main.cpp ^
REM      Scene/Renderer.cpp ^
REM      Scene/Square.cpp ^
REM      Scene/Triangle.cpp ^
REM      -IScene ^
REM      -s USE_SDL=2 ^
REM      -s USE_WEBGL2=1 ^
REM      -s MIN_WEBGL_VERSION=2 ^
REM      -s MAX_WEBGL_VERSION=2 ^
REM      -s FULL_ES3=1 ^
REM      -s WASM=1 ^
REM      -s ALLOW_MEMORY_GROWTH=1 ^
REM      --preload-file android/app/src/main/assets/shader@assets/shader ^
REM      -o web/index.html

IF %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded.
echo Run:  cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/
pause


