#pragma once

#include "ibo.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include <cstddef>
#include <format>
#include <iostream>
#include <memory>
#include <vector>

class GridPoints {
    static constexpr const GLuint vertex_attrib_location = 0; // where the vertex data is stored
    static constexpr const GLboolean is_normalized = GL_FALSE;
    static constexpr const GLsizei stride = 0;
    static constexpr const GLvoid *first_component_offset = nullptr;
    static constexpr const GLint points_per_vertex = 2;

    std::shared_ptr<Vao> vao;
    std::shared_ptr<Vbo> vbo;
    std::shared_ptr<Ibo> ibo;
    std::vector<GLfloat> triangles_points;
    std::vector<GLuint> indices;
    std::size_t tessellation_amount;

    friend std::ostream &operator<<(std::ostream &stream, const GridPoints &key);
    friend std::formatter<GridPoints>;

public:
    GridPoints(std::size_t tessellation_amount);

    [[nodiscard]] std::shared_ptr<Ibo> get_ibo() const noexcept;
    [[nodiscard]] std::shared_ptr<Vao> get_vao() const noexcept;
    [[nodiscard]] std::shared_ptr<Vao> get_vbo() const noexcept;
    [[nodiscard]] std::size_t get_tessellation_amount() const noexcept;
    [[nodiscard]] std::size_t get_indices_count() const noexcept;
};
