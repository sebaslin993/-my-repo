# AndroidProgrammingQuiz-CrossPlatformCI

Cross-platform OpenGL ES project for Desktop, Android, and Web.

Live WebGL version: [Assignment 1 - Primitives](https://sebaslin993.github.io/CSD2101/Assignment-1-Primitives/)

## Structure

- `Scene/` contains the shared renderer code used by all platforms.
- `android/` contains the Android Studio / Gradle project.
- `CMakeLists.txt` and `script_build_and_run.bat` build and run the Desktop target.
- `build_web.bat` and `build_web.sh` build the WebAssembly/WebGL target.

## Desktop

```bat
script_build_and_run.bat
```

## Android

Open the `android/` folder in Android Studio, run Clean Project, then build and run on an emulator or device.

## Web

```bat
build_web.bat
cd web
python -m http.server 8080
```

Open `http://localhost:8080/`.

## Submission Notes

Do not submit generated folders such as `build_desktop/`, `web/`, `.gradle/`, `.idea/`, `app/build/`, or `app/.cxx/`. Submit screenshots separately for Desktop, Android, and Web after rebuilding and running each target.
