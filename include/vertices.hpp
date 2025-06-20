#pragma once
#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include "glad/glad.h"
#include <array>
#include <format>
#include <memory>

template <std::size_t N, std::size_t POINTS_PER_VERTEX = 3> class Vertices {
    static_assert(N > 0);
    static_assert(POINTS_PER_VERTEX > 0);

    static constexpr const GLuint vertex_attrib_location = 0; // where the vertex data is stored
    static constexpr const GLboolean is_normalized = GL_FALSE;
    static constexpr const GLsizei stride = 0;
    static constexpr const GLsizei num_create = 1;
    static constexpr const GLvoid *first_component_offset = nullptr;

    std::shared_ptr<Vao> vao;
    std::shared_ptr<Vbo> vbo;

public:
    Vertices() = delete;
    Vertices(const std::array<GLfloat, N> &data) : vao(std::make_shared<Vao>()), vbo(std::make_shared<Vbo>()) {
        vao->bind();
        vbo->bind();

        glBufferData(GL_ARRAY_BUFFER, N * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError(std::format("cannot send vertex data: {}", gl_get_error_string(current_error)));
        }

        glEnableVertexAttribArray(vertex_attrib_location);
        glVertexAttribPointer(vertex_attrib_location, POINTS_PER_VERTEX, GL_FLOAT, is_normalized, stride,
                              first_component_offset);

        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError(
                std::format("cannot set vertex data attribs: {}", gl_get_error_string(current_error)));
        }

        vbo->unbind();
        vao->unbind();
    }

    Vertices(const Vertices &) noexcept = delete;
    Vertices(Vertices &&) noexcept = default;
    Vertices &operator=(const Vertices &) noexcept = delete;
    Vertices &operator=(Vertices &&) noexcept = default;

    std::shared_ptr<Vao> get_vao() const noexcept {
        return vao;
    }

    std::shared_ptr<Vbo> get_vbo() const noexcept {
        return vbo;
    }

    constexpr std::size_t get_vert_count() const noexcept {
        return N / POINTS_PER_VERTEX;
    }
};
