#include "glad/glad.h"

#include "es/cpu_tessellation.hpp"
#include "es/grid_points.hpp"

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "ibo.hpp"
#include "vao.hpp"

#include <cstddef>
#include <format>
#include <memory>

using std::format;
using std::make_shared;
using std::shared_ptr;
using std::size_t;

std::ostream &operator<<(std::ostream &stream, const GridPoints &grid_points) {
    stream << " { GridPoints: triangle_count " << (grid_points.triangles_points.size() / 3) << " tessellation amount "
           << grid_points.tessellation_amount << "}";
    return stream;
}

template <> struct std::formatter<GridPoints> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const GridPoints &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{ GridPoints: triangle_count {0} tessellation amount {1} }",
                              obj.triangles_points.size() / 3, obj.tessellation_amount);
    }
};

GridPoints::GridPoints(size_t tessellation_amount)
    : vao(make_shared<Vao>()), vbo(make_shared<Vbo>()), ibo(make_shared<Ibo>()),
      triangles_points(make_lattice(tessellation_amount)), indices(lattice_points_list(tessellation_amount)),
      tessellation_amount(tessellation_amount) {

    // TODO: clean up the copy-paste between this and Vertices ctor
    vao->bind();
    vbo->bind();

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(triangles_points.size() * sizeof(GLfloat)),
                 triangles_points.data(), GL_STATIC_DRAW);
    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("cannot send vertex data: {}", gl_get_error_string(current_error)));
    }

    glEnableVertexAttribArray(vertex_attrib_location);
    glVertexAttribPointer(vertex_attrib_location, points_per_vertex, GL_FLOAT, is_normalized, stride,
                          first_component_offset);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("cannot set vertex data attribs: {}", gl_get_error_string(current_error)));
    }

    vbo->unbind();
    vao->unbind();

    ibo->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)), indices.data(),
                 GL_STATIC_DRAW);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("cannot setup ibo: {}", gl_get_error_string(current_error)));
    }

    ibo->unbind();
}

shared_ptr<Ibo> GridPoints::get_ibo() const noexcept {
    return ibo;
}

std::size_t GridPoints::get_tessellation_amount() const noexcept {
    return tessellation_amount;
}

std::shared_ptr<Vao> GridPoints::get_vao() const noexcept {
    return vao;
}

std::size_t GridPoints::get_indices_count() const noexcept {
    return indices.size();
}
