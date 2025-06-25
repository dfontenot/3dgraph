#pragma once

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"
#include "vao.hpp"
#include "vbo.hpp"

#include <iostream>
#include <concepts>
#include <format>
#include <memory>
#include <utility>

template <typename T>
concept HasDataPointerAccess = requires(T obj) {
    { obj.data() } -> std::same_as<GLfloat *>;
    { obj.size() } -> std::same_as<std::size_t>;
};

class Vertices {
    static constexpr const GLuint vertex_attrib_location = 0; // where the vertex data is stored
    static constexpr const GLboolean is_normalized = GL_FALSE;
    static constexpr const GLsizei stride = 0;
    static constexpr const GLsizei num_create = 1;
    static constexpr const GLvoid *first_component_offset = nullptr;

    /** raw data size, not count of verts */
    std::size_t size;
    std::size_t points_per_vertex;
    std::shared_ptr<Vao> vao;
    std::shared_ptr<Vbo> vbo;

    friend std::ostream &operator<<(std::ostream &stream, const Vertices &key);
    friend std::formatter<Vertices>;

public:
    Vertices() = delete;

    template <HasDataPointerAccess Data>
    Vertices(Data &&data, std::size_t points_per_vertex)
        : vao(std::make_shared<Vao>()), vbo(std::make_shared<Vbo>()), points_per_vertex(points_per_vertex),
          size(std::forward<Data>(data).size()) {
        vao->bind();
        vbo->bind();

        glBufferData(GL_ARRAY_BUFFER, std::forward<Data>(data).size() * sizeof(GLfloat),
                     std::forward<Data>(data).data(), GL_STATIC_DRAW);
        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError(std::format("cannot send vertex data: {}", gl_get_error_string(current_error)));
        }

        glEnableVertexAttribArray(vertex_attrib_location);
        glVertexAttribPointer(vertex_attrib_location, static_cast<GLint>(points_per_vertex), GL_FLOAT, is_normalized,
                              stride, first_component_offset);

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

    [[nodiscard]] std::shared_ptr<Vao> get_vao() const noexcept;
    [[nodiscard]] std::shared_ptr<Vbo> get_vbo() const noexcept;
    [[nodiscard]] std::size_t get_vert_count() const noexcept;
};
