#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "ibo.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <numeric>

class PlaneIbo
{
    static constexpr const int num_verts_in_quad = 4;

    std::shared_ptr<Ibo> ibo;

public:
    PlaneIbo() : ibo(std::make_shared<Ibo>())
    {
        std::array<GLuint, num_verts_in_quad> indices;
        std::iota(indices.begin(), indices.end(), 0);

        ibo->bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_verts_in_quad * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot setup ibo: " + gl_get_error_string(current_error));
        }
    }

    std::shared_ptr<Ibo> get_ibo() const noexcept
    {
        return ibo;
    }
};
