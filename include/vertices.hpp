#pragma once
#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include "glad/glad.h"
#include <array>
#include <memory>

template <std::size_t N> class Vertices {
    static constexpr GLint points_per_vertex = 3;
    static constexpr GLuint vertex_attrib_location = 0; // where the vertex data is stored
    static constexpr GLboolean is_normalized = GL_FALSE;
    static constexpr GLsizei stride = 0;
    static constexpr GLsizei num_create = 1;
    static constexpr GLvoid *first_component_offset = nullptr;

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
            throw WrappedOpenGLError("cannot send vertex data: " + gl_get_error_string(current_error));
        }

        glEnableVertexAttribArray(vertex_attrib_location);
        glVertexAttribPointer(vertex_attrib_location, points_per_vertex, GL_FLOAT, is_normalized, stride,
                              first_component_offset);

        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot set vertex data attribs: " + gl_get_error_string(current_error));
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
        return N / points_per_vertex;
    }
};
