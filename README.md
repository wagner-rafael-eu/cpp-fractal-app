# cpp-fractal-app

Interactive C++ fractal viewer built with SFML. Supports multiple fractal types (Mandelbrot, Sierpiński, Koch, Menger, Dragon), interactive zooming/centering, keyboard controls, and persistent view settings.

**Key features**
- Multiple fractal modes: Mandelbrot (1), Sierpiński (2), Koch (3), Menger (4), Dragon (5)
- Smooth, cursor-centered mouse-wheel zoom and continuous keyboard zoom (+ / -)
- Left-click to recenter; `R` to reset view
- Overlay showing zoom level and center coordinates
- Persistent settings saved to `C:/_AI/002/fractal_settings.txt`

## Prerequisites

- Visual Studio 2022 (or later) with C++ development tools
- CMake 3.10 or higher
- SFML (the project bundles a build under `build/_deps/sfml-*`)

## Build (Windows)

1. Create a build directory and configure with CMake:

```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
```

2. Build the Release target:

```powershell
cmake --build . --config Release --target FractalApp
```

3. The executable will be copied to `C:\_AI\002\FractalApp.exe` by the CMake post-build step. Alternatively run the built EXE in `build/Release`.

## Run

Run the app and use the on-screen overlay and controls to interact.

## Controls

- 1..5 : Switch fractal modes
- Mouse wheel : Zoom (centered on cursor)
- Left mouse button : Recenter
- `+` / `-` : Continuous zoom while held
- `R` : Reset to initial view

## Notes

- Debug images and logs are written to `C:/_AI/002` (e.g. `debug_fractal_*.png`, `clicks.log`).
- The repo currently contains build artifacts; consider cleaning `build/` from the repository (a `.gitignore` is present).

## License

This repository contains example code. Add your preferred license file if you intend to publish it publicly.