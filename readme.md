# 3dgraph

Graph a 3d function using OpenGL or OpenGL ES. 

## OpenGL requirements
Requires OpenGL 4.1 for hardware tessellation, or OpenGL ES 3.0 (software tessellation) for the es build.

## Building
Need a C++23 capable compiler (at least GCC15, or at least clang20), cmake, and [conan package manager](https://conan.io)

Can treat the build helper script like cmake, all args passed to it will be forwarded to cmake
* `./run-build.sh`

## Controls
* Up / down : Control the divisor of the 3D function
* Left / right: "Pan" the 3D function (render different parts of the surface). Hold shift to pan on Y axis
* WASD: Orbit the mesh. Hold shift to slow down the orbit.
* E: Toggle wireframe only view (OpenGL 4.1 only)
* Scroll wheel: Change the tessellation level of the mesh
* Plus / minus: Zoom in and out
* Q: Quit
