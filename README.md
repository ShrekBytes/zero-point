# Zero Point

**Where Forest Meets Village — a 2D OpenGL environmental simulation**

`Zero Point` renders a split world: a dense forest in the upper-left, a quiet village in the lower-right, and a diagonal river between them. The world transitions between day and night, changing lighting, color palette, and active wildlife.

![Zero Point Showcase](planning/showcase.gif)


## Scene Overview

| Zone | Elements |
|------|----------|
| Sky | Sun/moon, drifting clouds, stars, birds |
| Forest (upper-left) | Trees, layered grass, deer (night), fireflies (night) |
| Village (lower-right) | Houses, windmill, clothesline, fence, cat (day) |
| River (diagonal) | Bresenham banks, animated flow streaks |

## Controls

| Key | Action |
|-----|--------|
| `Space` | Toggle day/night |
| `Esc` / `q` / `Q` | Quit |

Auto cycle: day/night toggles about every **10 seconds**.

## Features

- Day/night rendering with different tones and active entities
- Smooth animation loop (~16 ms timer) for clouds, birds, windmill, cat, deer, and fireflies
- Classic graphics algorithms:
  - Bresenham line drawing (river banks)
  - DDA line drawing (fence)
  - Midpoint circle (sun/moon)
- 2D transformation use cases: translation, rotation, and scaling

## Build and Run

### Prerequisites

- CMake 3.10+
- C++20 compiler
- OpenGL
- GLUT/FreeGLUT

### Linux (FreeGLUT)

Install dependencies (Debian/Ubuntu example):

```sh
sudo apt install build-essential cmake freeglut3-dev
```

Build and run:

```sh
cmake -B build
cmake --build build
./build/ZeroPoint
```

### macOS

Install dependencies (Homebrew example):

```sh
brew install cmake freeglut
```

Then build and run with the same commands above.

### Windows

Use Visual Studio (or another CMake-capable toolchain) with OpenGL + GLUT available in your environment, then:

```sh
cmake -B build
cmake --build build --config Release
```

Run the produced `ZeroPoint` executable from the `build` output directory.
