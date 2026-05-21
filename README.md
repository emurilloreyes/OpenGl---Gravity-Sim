# OpenGL Gravity Sim

Practicing OpenGL with a 2D simulation of light rays bending near a black hole. Rays are traced as line strips that grow each frame; collision with the hole stops further updates.

## Build and run

Requires MSYS2 UCRT64 (or similar) with `g++`, GLFW, and GLAD. From the project root:

```bash
g++ -std=c++17 -Iinclude -Llib src/main.cpp src/glad.c -lglfw3dll -o simulation.exe
./simulation.exe <width> <height>
```

Example:

```bash
./simulation.exe 800 600
```

The VS Code default build task in [`.vscode/tasks.json`](.vscode/tasks.json) uses the same command.

## How the simulation works

- **Black hole** — Drawn as a red circle in NDC; mass and Schwarzschild-style radius are configurable in [`src/main.cpp`](src/main.cpp).
- **Rays** — A fan of rays enters from the left. Each ray keeps a unit **direction** and advances with `speed * dt` each step.
- **Bending** — Weak-field style deflection: curvature scales as `κ = 4GM / (c²r²)` (rad/m). Each step rotates the direction toward the hole by `Δθ = κ × ds`, where `ds` is the physical arc length for that step (`speed × dt` converted to meters).
- **Time-step invariance** — `kPhysicsDt` and `kRaySpeed` in `main.cpp` control step size and display speed. Changing `dt` while holding simulated time fixed should not change the path shape; changing speed only changes how fast the path is drawn if `speed × dt` stays the same.

This is a qualitative 2D model, not a full geodesic or Newtonian `F = GMm/r²` solver.

## Progress

- `BlackHole` and `Ray` structs with OpenGL VAO/VBO drawing
- Ray paths stored and redrawn as `GL_LINE_STRIP` (correct curved trajectories)
- Collision detection against the hole radius
- Custom window size via command-line arguments
- Aspect-correct distance and circle rendering
- Time-step invariant integration (`Ray::step`, direction + `rotateToward`)

## Needs work

- Newtonian gravity for massive particles (separate from light bending)
- More accurate GR (geodesics, proper impact parameter / Einstein angle)
- 3D extension
- Richer black hole rendering (lensing, accretion disk, etc.)
- Decouple render frame rate from physics (fixed `dt` substeps per frame)

## Version history

- **V1** — Simple star and ray drawing with collision detection from the left
- **V2** — Rays grow along their path instead of stamping segments; collisions work from all approach angles
- **V3** — Custom window sizing; aspect ratio fix for circle drawing
- **V4** — Weak-field light bending with direction + arc-length integration; `dt` / speed no longer change trajectory shape arbitrarily; added `tests/ray_invariance.cpp`
