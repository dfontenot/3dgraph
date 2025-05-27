#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"

struct Ibo {
    GLuint val;

    constexpr operator GLuint() const {
        return val;
    }

    Ibo() {
        glGenBuffers(num_create, &val);
    }

    ~Ibo() {
        glDeleteBuffers(num_create, &val);
    }

    void bind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot to bind IBO due to existing error " + std::to_string(val) + ": " +
                                     gl_get_error_string(err));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, val);
        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("failed to bind IBO " + std::to_string(val) + ": " + gl_get_error_string(err));
        }
    }

    /**
     * unbind after vao is unbound
     */
    void unbind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot to unbind IBO due to existing error " + std::to_string(val) + ": " +
                                     gl_get_error_string(err));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("failed to unbind IBO " + std::to_string(val) + ": " + gl_get_error_string(err));
        }
    }

private:
    static constexpr GLsizei num_create = 1;
};
