#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"

#include <format>

struct Ibo {
    constexpr operator GLuint() const {
        return val;
    }

    Ibo() {
        glGenBuffers(num_create, &val);
    }

    Ibo(Ibo &&) = default;
    Ibo(Ibo const &) = delete;
    Ibo &operator=(const Ibo &) noexcept = delete;
    Ibo &operator=(Ibo &&) noexcept = default;

    ~Ibo() {
        glDeleteBuffers(num_create, &val);
    }

    /**
     * binds the IBO, throws on error
     */
    void bind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError(
                std::format("cannot to bind IBO due to existing error {0}: {1}", val, gl_get_error_string(err)));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, val);
        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError(std::format("failed to bind IBO {0}: {1}", val, gl_get_error_string(err)));
        }
    }

    /**
     * unbind after vao is unbound
     */
    void unbind() {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            throw WrappedOpenGLError(
                std::format("failed to unbind IBO due to existing error {0}: {1}", val, gl_get_error_string(err)));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        if ((err = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError(std::format("failed to unbind IBO {0}: {1}", val, gl_get_error_string(err)));
        }
    }

private:
    static constexpr const GLsizei num_create = 1;
    GLuint val{};
};
