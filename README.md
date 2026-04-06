# OpenGl---Gravity-Sim
Practicing OpenGL, drawing a simulation of light getting affected by different structures like planets, stars, and black holes.

## Progress 
 - Created Star and Ray structs
 - Ray updates every frame, growing on its trajectory
 - Implemented collision mechanics by calculating distance before overlap

## Needs work
 - Newtonian gravity simulation
   - Possible ray tracing? For rendering
   - Expanding to 3D newtonian gravity
 - Black hole implementation
   - Hopefully, render black hole like they did with interstellar, (except much simpler) 


## Version History

- V1
   - Simple star and ray drawing with collision detection from the left
- V2
   - Changed ray functionality, now grows instead of stamping a new one ahead (fixes diagonal and can now refresh the screen)
   - Cleaned up collisions, now works from all angles for different rays
- V3
   - Added custom window sizing
   - Fixed aspect ratio bug with circle drawing  
