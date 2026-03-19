# Zero Point 🌲🏡
**Where Forest Meets Village — A 2D OpenGL Environmental Simulation**

A Computer Graphics Lab project built with OpenGL/FreeGLUT. The scene is split into two worlds — a forest and a village — divided by a diagonal river. The environment shifts between day and night, each revealing different elements and animals.

###

## Scene Layout

- **Sky (top)** — sun/moon, stars, clouds, birds
- **Forest (upper-left)** — trees, tall grass, deer at night, fireflies
- **Village (lower-right)** — house, windmill, fence, cat during day
- **River (diagonal)** — natural boundary between the two worlds

###

## Features

- Day/Night cycle with automatic timer toggle
- Keyboard shortcut to manually switch states
- Animated cat (day), deer (night), windmill, birds, fireflies
- Graphics algorithms: Bresenham Line, DDA Line, Midpoint Circle
- 2D transformations: translation, rotation, scaling

###

## Controls

| Key | Action |
|-----|--------|
| `d` | Switch to Day |
| `n` | Switch to Night |

###

## Built With

- C++
- OpenGL
- FreeGLUT
