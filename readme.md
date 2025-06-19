# 3dgraph

Graph a 3d function using OpenGL. Requires OpenGL 4.1 for hardware tessellation.

## Building
Need a C++23 capable compiler (at least GCC14)

Can treat the build helper script like cmake, all args passed to it will be forwarded to cmake
1. `./run-build.sh`

## Controls
* Up / down : Control the divisor of the 3D function
* Left / right (optionally with shift): "Pan" the 3D function (render different parts of the surface)
* WASD: Orbit the mesh
* Scroll wheel: Change the tessellation level of the mesh
* Plus / minus: Zoom in and out
* Q: Quit
