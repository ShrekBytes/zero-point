# Zero Point

**Where Forest Meets Village — A 2D OpenGL Environmental Simulation**

A scene split into two worlds — a dense forest and a quiet village — divided by a diagonal river. The environment automatically cycles between day and night, each revealing different elements and animals.

## Scene

| Zone | Contents |
|------|----------|
| Sky | Sun / moon, drifting clouds, stars, birds |
| Forest (upper-left) | Trees, tall grass, deer at night, fireflies |
| Village (lower-right) | Houses, windmill, clothesline, fence, cat |
| River (diagonal) | Animated flow streaks, muddy banks |

## Controls

| Key | Action |
|-----|--------|
| `Space` | Toggle day / night |
| `ESC` / `q` | Quit |

The scene also auto-toggles every ~8 seconds.

## Features

- Day/night cycle with distinct lighting, colors, and active elements
- Animated cat (day), deer (night), birds (day), windmill, fireflies (night), and clouds
- Line drawing algorithms: **Bresenham** (river edges), **DDA** (fence), **Midpoint Circle** (sun/moon)
- 2D transformations: translation, rotation, and scaling

## Build

Requires **OpenGL** and **FreeGLUT**.

```sh
cmake -B build && cmake --build build
./build/ZeroPoint
```

> On Linux, install dependencies with:
> `sudo apt install freeglut3-dev` or equivalent for your distro.
