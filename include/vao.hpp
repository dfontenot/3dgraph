#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"

struct Vao {
    GLuint val;

    constexpr operator GLuint() const {
        return val;
    }
    Vao() {
        glGenVertexArrays(num_create, &val);
    }

    ~Vao() {
        glDeleteVertexArrays(num_create, &val);
    }

    void bind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot bind VAO due to existing error: " + gl_get_error_string(err));
        }

        glBindVertexArray(val);

        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("failed to bind VAO " + std::to_string(val) + ": " + gl_get_error_string(err));
        }
    }

    void unbind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot unbind VAO due to existing error: " + gl_get_error_string(err));
        }

        glBindVertexArray(0);

        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("failed to unbind VAO " + std::to_string(val) + ": " + gl_get_error_string(err));
        }
    }

private:
    static constexpr GLsizei num_create = 1;
};
