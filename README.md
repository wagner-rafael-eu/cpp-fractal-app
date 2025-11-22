# C++ Fractal Generator

This program generates and displays a Mandelbrot fractal in a 640x480 window using SFML for graphics.

## Prerequisites

- Visual Studio 2022 with C++ development tools
- CMake 3.10 or higher
- Git (for downloading SFML)

## Building the Project

1. Create a build directory:
   ```
   mkdir build
   cd build
   ```

2. Configure CMake:
   ```
   cmake -G "Visual Studio 17 2022" -A x64 ..
   ```

3. Build the project:
   ```
   cmake --build . --config Release
   ```

4. Run the application:
   ```
   Release\FractalApp.exe
   ```

## Features

- Displays a Mandelbrot fractal
- 640x480 resolution
- Colorful visualization using HSV color mapping
- Real-time display using SFML

## Controls

- Close the window to exit the program