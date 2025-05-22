#pragma once

#include "glad/glad.h"

constexpr GLfloat z_mult_default = 0.01f;

struct FunctionParams {
    // allows panning the 3d function but does not move the model matrix
    // itself (panning happens in place)
    GLfloat x_offset;
    GLfloat y_offset;

    // allows the user to modify the intensity of the function
    GLfloat z_mult;

    FunctionParams() : x_offset(0.0f), y_offset(0.0f), z_mult(z_mult_default) {
    }

    FunctionParams(GLfloat x_offset, GLfloat y_offset, GLfloat z_mult)
        : x_offset(x_offset), y_offset(y_offset), z_mult(z_mult) {
    }
};
