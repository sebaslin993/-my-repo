# Assignment 2 - 3D Fan

A cross-platform OpenGL ES 3.0 table fan built from one indexed cube. The same shared C++ scene runs on Windows Desktop, Android, and WebGL.

Live projects:

- [Assignment 1 - Primitives](https://sebaslin993.github.io/CSD2101/Assignment-1-Primitives/)
- [Assignment 2 - 3D Fan](https://sebaslin993.github.io/CSD2101/Assignment-2-3DFan/)
- [Assignment 3 - HUD](https://sebaslin993.github.io/CSD2101/Assignment-3-HUD/)

The fan consists of seven draws of one VAO:

- Brown base
- Light-gray pole
- Dark-gray hub
- Red, blue, orange, and green blades

The model uses the provided `Transform` matrix stack for hierarchical transformations. Clicking or tapping toggles the fan, while dragging temporarily increases its rotation speed according to pointer velocity.

## Controls

| Input | Result |
|---|---|
| Click or tap | Toggle fan ON/OFF |
| Hold and drag | Increase blade speed according to drag velocity |
| Release | Return to the base rotation speed |
| Escape (Desktop) | Close the application |

Messages such as `Fan ON`, `Fan OFF`, and measured drag velocity are written to the platform log.

## Repository structure

```text
Assignment-2-3DFan/
|-- .github/workflows/       GitHub Actions and Pages deployment
|-- android/                 Android Studio project
|-- cmake/                   Desktop dependency configuration
|-- Scene/                   Shared C++ rendering and interaction code
|-- CMakeLists.txt           Windows Desktop build
|-- script_build_and_run.bat Desktop convenience script
|-- build_web.bat            Windows Emscripten build script
|-- build_web.sh             Linux/macOS Emscripten build script
|-- README.md
`-- .gitignore
```

## Desktop - Windows

### Prerequisites

- Windows 10 or 11
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.15 or newer available in `PATH`
- Git
- Internet access during the first configuration so CMake can fetch GLFW, GLEW, and GLM

### Build and run

Open Command Prompt in the repository root and run:

```bat
script_build_and_run.bat
```

The script configures a Visual Studio 2022 x64 Debug build, compiles it, copies the shader assets, and launches the program.

Desktop executable:

```text
build_desktop/Debug/TouchEvents.exe
```

The generated `build_desktop/` directory is ignored by Git and must not be committed.

## Android

### Prerequisites

- Android Studio
- Android SDK configured by Android Studio
- Android NDK
- CMake 3.22.1
- A physical Android device or emulator supporting OpenGL ES 3.0
- Internet access during the first build so Gradle and CMake can fetch dependencies

### Build and run

1. Start Android Studio.
2. Select **Open**.
3. Open the `android/` directory, not the repository root.
4. Allow Gradle sync and CMake configuration to finish.
5. Select an OpenGL ES 3.0-capable phone or emulator.
6. Run the `app` configuration.

Android APK output is generated under:

```text
android/app/build/outputs/apk/
```

### Android logging

Open Android Studio's **Logcat** tool window and filter for:

```text
Fan3D
```

Alternatively, use ADB:

```bat
adb logcat -s Fan3D
```

Android build output such as `.gradle/`, `app/build/`, and `app/.cxx/` must not be committed.

## WebGL - Emscripten

### Prerequisites

- Current Emscripten SDK
- Python 3
- Git
- A browser with WebGL 2 enabled

The commands below assume Emscripten is installed at `C:\emsdk`. Adjust the path if it is installed elsewhere.

### Build on Windows

Open Command Prompt and activate Emscripten:

```bat
cd C:\emsdk
emsdk_env.bat
```

Then build from the repository root:

```bat
cd C:\path\to\Assignment-2-3DFan
build_web.bat
```

### Build on Linux or macOS

Activate Emscripten and run:

```sh
source /path/to/emsdk/emsdk_env.sh
cd /path/to/Assignment-2-3DFan
./build_web.sh
```

### Run locally

The build generates:

```text
web/index.html
web/index.js
web/index.wasm
web/index.data
```

Serve these files through HTTP:

```bat
cd web
python -m http.server 8080
```

Open:

```text
http://localhost:8080/
```

Do not open `index.html` directly from File Explorer. WebAssembly and preloaded shader assets must be served through HTTP.

Browser messages are available in Developer Tools under the **Console** tab. If WebGL context creation fails, verify that WebGL 2 and browser graphics acceleration are enabled.

The generated `web/` and `external/` directories are ignored by Git and must not be committed.

## Implementation summary

- One VBO containing position and grayscale-shade sub-regions created with `glBufferSubData`
- One 36-index IBO
- One VAO capturing attribute locations 0/1 and the IBO binding
- Seven `glDrawElements` calls using the same cube geometry
- Per-part colour through the `PARTCOLOR` uniform
- Perspective projection rebuilt when the viewport changes
- Hierarchical push/pop transforms for the base, pole, hub, and blades
- Four blades animated by a shared `spinAngle`
- Tap detection using accumulated pointer movement
- Drag velocity measured with `std::chrono`
- Drag boost clamped to 20 degrees per frame and reset on release

## Submission hygiene

Do not commit generated or machine-specific directories:

```text
.idea/
build_desktop/
external/
web/
android/.gradle/
android/app/build/
android/app/.cxx/
```

Before committing, run:

```bat
git status
```

Only source code, shaders, build scripts, documentation, and workflow definitions should be tracked.
