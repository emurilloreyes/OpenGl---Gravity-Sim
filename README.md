# OpenGL Gravity Sim

Practice project: light rays near a Schwarzschild black hole in 3D. Physics integrates **equatorial null geodesics** (RK4 in φ); rendering uses OpenGL 3.3 core with an orbit camera.

## Build and run

Requires MSYS2 UCRT64 with `g++`, GLFW, and GLAD. `glfw3.dll` must be on the PATH or next to the executable. From the project root:

```bash
g++ -std=c++17 -Iinclude -Llib src/main.cpp src/mesh.cpp src/geodesic.cpp src/glad.c -lglfw3dll -o simulation.exe
./simulation.exe <width> <height>
```

Example:

```bash
./simulation.exe 800 600
```

The VS Code default build task in `[.vscode/tasks.json](.vscode/tasks.json)` uses the same command.

## Controls

- **Left drag** — Orbit camera around the black hole
- **Scroll** — Zoom in/out

## What you see

- **Red sphere** at the origin — visual black hole (`rs` in geometric units; see `[include/constants.hpp](include/constants.hpp)`).
- **White line strips** — photon paths that grow frame by frame, bent by gravity. Each ray is a fan entry: approach azimuth × impact parameter `b` × optional orbital-plane tilt about X.
- Paths stop when a photon is **captured** (`r ≤ rs`) or **escapes** (`r ≥ rMax`).

Tune ray count and integration in `constants.hpp` (`rayDirectionCount`, `rayBMin`/`rayBMax`/`rayBStep`, `rayTiltCount`, `dPhi`, `geodesicStepsPerFrame`).

## Architecture

Each frame:

1. `**Photon::step`** (`[src/geodesic.cpp](src/geodesic.cpp)`) — advance `(u = 1/r, u', φ)` with RK4; append world-space `(x,y,z)` to `path`.
2. `**Mesh::updateVertices**` (`[src/mesh.cpp](src/mesh.cpp)`) — copy `path` into a GPU line-strip buffer.
3. `**Mesh::draw**` — vertex shader applies shared `mvp` (camera view × projection); depth test handles occlusion.

`PhotonVisual` in `[src/main.cpp](src/main.cpp)` pairs one `Photon` (physics + CPU `path`) with one `Mesh` (GPU geometry). `draw` does not set positions; it renders whatever was uploaded after `step`.

```
constants.hpp  →  spawn parameters (b, angles, rs, dPhi, …)
geodesic.cpp   →  Photon path in world space
main.cpp       →  window, camera, loop, PhotonVisual fan
mesh.cpp       →  shaders, sphere, VAO/VBO, glDrawArrays
camera.hpp     →  OrbitCamera → view/projection matrices
math.hpp       →  Vec3, Mat4, lookAt, perspective
```

## Project layout


| Path                                             | Role                                                                |
| ------------------------------------------------ | ------------------------------------------------------------------- |
| `[src/main.cpp](src/main.cpp)`                   | GLFW/GLAD init, input, ray fan setup, frame loop                    |
| `[src/geodesic.cpp](src/geodesic.cpp)`           | Schwarzschild photon ODE, RK4, `Photon` / `traceEquatorialGeodesic` |
| `[src/mesh.cpp](src/mesh.cpp)`                   | Shaders, `createSphere`, `Mesh` (VAO/VBO), draw                     |
| `[src/glad.c](src/glad.c)`                       | OpenGL 3.3 core loader (generated)                                  |
| `[include/constants.hpp](include/constants.hpp)` | BH mass/radius, integration steps, ray fan                          |
| `[include/geodesic.hpp](include/geodesic.hpp)`   | `Photon`, geodesic state API                                        |
| `[include/mesh.hpp](include/mesh.hpp)`           | `Mesh`, shader helpers                                              |
| `[include/camera.hpp](include/camera.hpp)`       | `OrbitCamera`                                                       |
| `[include/math.hpp](include/math.hpp)`           | Small linear algebra                                                |
| `include/GLFW/`, `include/glad/`                 | Third-party headers                                                 |
| `lib/`, `glfw3.dll`                              | GLFW link/runtime (Windows)                                         |


## Physics (current)

- Unit system: geometric (effectively **G = c = 1**); `rs = 2M`.
- Equatorial null geodesic: **u'' + u = (3/2) rs u²** with **u = 1/r**, integrated in **φ** via RK4.
- Initial conditions at `rStart`: `u = 1/rStart`, `u' = 1/b`, `φ = approachAngle + π`.
- `planeTiltX` rotates the equatorial track into 3D (approximate multi-plane fan, not full off-equatorial GR).
- Helpers: `bCritical()`, `weakFieldDeflection(b)` for reference; SI constants are documented but not used in the ODE.

## Rendering (current)

- OpenGL **3.3 core**, depth buffer, perspective projection.
- Black hole: triangle sphere mesh (`GL_TRIANGLES`).
- Rays: dynamic `GL_LINE_STRIP` per photon (default line width; not tube geometry).
- Single **MVP** per frame (`model` = identity); per-ray differences are only vertex positions.

## Limitations / next steps

- Equatorial geodesics only; tilt is a display rotation, not full 3D null geodesics.
- Physics step count per frame (`geodesicStepsPerFrame`) ties simulation speed to frame rate.
- Newtonian massive-particle orbits not implemented.
- `traceEquatorialGeodesic` and `createLineStrip` exist for one-shot / legacy use; the live app uses incremental `Photon::step`.

## Version history

- **V1** — Simple star and ray drawing with collision detection from the left
- **V2** — Rays grow along their path; collisions from all approach angles
- **V3** — Custom window sizing; aspect ratio fix for circle drawing
- **V4** — Weak-field light bending; arc-length integration; `tests/ray_invariance.cpp` (see git history; not in tree)
- **V5** — 3D shell: sphere BH, straight 3D rays, orbit camera
- **V6 (current)** — Schwarzschild equatorial null geodesics (RK4), multi-ray fan (`b`, azimuth, tilt), `Photon` + dynamic line-strip meshes

