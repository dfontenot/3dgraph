#pragma once

#include <vector>
#include "glad/glad.h"

/**
 * @ brief creates a lattice of points (no triangles)
 * @ return a flat GLfloat array
 */
std::vector<GLfloat> make_lattice(GLuint tessellation_amount);

std::vector<GLuint> lattice_points_list(GLuint tessellation_amount);
