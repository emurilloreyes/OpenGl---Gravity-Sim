# OpenGL Gravity Sim

Practicing OpenGL with a simulation of light near a black hole. **Phase A (current):** 3D perspective rendering with a sphere black hole and straight ray line strips; orbit the camera with mouse. Physics (weak-field / geodesics) is paused until Phase B.

## Build and run

Requires MSYS2 UCRT64 (or similar) with `g++`, GLFW, and GLAD. From the project root:

```bash
g++ -std=c++17 -Iinclude -Llib src/main.cpp src/mesh.cpp src/glad.c -lglfw3dll -o simulation.exe
./simulation.exe <width> <height>
```

Example:

```bash
./simulation.exe 800 600
```

The VS Code default build task in [`.vscode/tasks.json`](.vscode/tasks.json) uses the same command.

## Controls (Phase A)

- **Left drag** — Orbit camera around the black hole
- **Scroll** — Zoom in/out

## How the simulation works

- **Black hole** — Red sphere at the origin ([`src/main.cpp`](src/main.cpp), mesh from [`src/mesh.cpp`](src/mesh.cpp)).
- **Rays** — Straight white line strips on a 3D grid (plane `x = -6`, direction `+x`). Bending physics returns in Phase B (geodesics).
- **Rendering** — Perspective projection + depth test; MVP shader in [`src/mesh.cpp`](src/mesh.cpp); camera in [`include/camera.hpp`](include/camera.hpp).

## Progress

- 3D perspective rendering, depth buffer, MVP shaders
- Sphere black hole mesh + 3D ray line strips
- Orbit camera (mouse drag + scroll)
- Earlier 2D weak-field physics (V4) — see git history / `tests/ray_invariance.cpp`

## Needs work

- Phase B: Schwarzschild null geodesics
- Newtonian gravity for massive particles (separate from light bending)
- Richer black hole rendering (lensing, accretion disk, etc.)
- Decouple render frame rate from physics (fixed `dt` substeps per frame)

## Version history

- **V1** — Simple star and ray drawing with collision detection from the left
- **V2** — Rays grow along their path instead of stamping segments; collisions work from all approach angles
- **V3** — Custom window sizing; aspect ratio fix for circle drawing
- **V4** — Weak-field light bending with direction + arc-length integration; `dt` / speed no longer change trajectory shape arbitrarily; added `tests/ray_invariance.cpp`
- **V5 (Phase A)** — 3D rendering shell: sphere BH, straight 3D rays, orbit camera
