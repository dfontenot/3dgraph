#pragma once

#include <vector>
#include <cstddef>
#include "glad/glad.h"

/**
 * @ brief creates a lattice of points (no triangles)
 * @ return a flat GLfloat array
 */
std::vector<GLfloat> make_lattice(std::size_t tessellation_amount);
