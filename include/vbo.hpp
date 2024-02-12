#pragma once

#include "glad/glad.h"

struct Vbo {
    GLuint val;

    constexpr operator GLuint() const { return val; }
    Vbo() {
        glGenBuffers(num_create, &val);
    }

    ~Vbo() {
        glDeleteBuffers(num_create, &val);
    }

    void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, val);
    }

    void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

private:
    static constexpr GLsizei num_create = 1;
};
