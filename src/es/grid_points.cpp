#include "grid_points.hpp"
#include "cpu_tessellation.hpp"

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "ibo.hpp"

#include <cstddef>
#include <memory>
#include <numeric>

using std::size_t;
using std::shared_ptr;

GridPoints::GridPoints(size_t tessellation_amount)
    : ibo(make_shared<Ibo>()), triangles_points(make_lattice(tessellation_amount)),
      tessellation_amount(tessellation_amount) {
    points_order.reserve(tessellation_amount * tessellation_amount * 4);

    // assign to points_order

    ibo->bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, N * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("cannot setup ibo: " + gl_get_error_string(current_error));
    }

    ibo->unbind();
}

shared_ptr<Ibo> GridPoints::get_ibo() const noexcept {
    return ibo;
}

std::size_t get_tessellation_amount() const noexcept {
    return tessellation_amount;
}
