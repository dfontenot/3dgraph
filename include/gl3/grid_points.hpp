#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "ibo.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <numeric>

template <std::size_t N> class GridPoints {
    std::shared_ptr<Ibo> ibo;

public:
    GridPoints() : ibo(std::make_shared<Ibo>()) {
        std::array<GLuint, N> indices;
        std::iota(indices.begin(), indices.end(), 0);

        ibo->bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, N * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot setup ibo: " + gl_get_error_string(current_error));
        }
    }

    std::shared_ptr<Ibo> get_ibo() const noexcept {
        return ibo;
    }
};
