#include "es/grid_points.hpp"
#include "es/cpu_tessellation.hpp"

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "ibo.hpp"

#include <cstddef>
#include <format>
#include <memory>

using std::format;
using std::make_shared;
using std::shared_ptr;
using std::size_t;

GridPoints::GridPoints(size_t tessellation_amount)
    : ibo(make_shared<Ibo>()), triangles_points(make_lattice(tessellation_amount)),
      indices(lattice_points_list(tessellation_amount)), tessellation_amount(tessellation_amount) {

    ibo->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
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
