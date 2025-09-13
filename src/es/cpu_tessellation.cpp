#include "es/cpu_tessellation.hpp"

#include "glad/glad.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <vector>

#if defined(__clang__)
#include <range/v3/all.hpp>
#endif

using std::domain_error;
using std::numeric_limits;
using std::size_t;
using std::vector;
using std::ranges::iota_view;

#if defined(__clang__)
using ranges::views::cartesian_product;
#else
using std::ranges::views::cartesian_product;
#endif

/** grid, 2 dimensions only */
constexpr const auto vertex_dims = 2;

/**
 * @brief makes a lattice mesh in 2 dimensions (plane)
 * order of points will be bottom left corner to top left corner, then towards right side
 * points in order x0, y0, x1, y1, ...
 *
 * tessellation level 0 = 1 point
 * tessellation level 1 = 1 square
 */
vector<GLfloat> make_lattice(GLuint tessellation_amount) {
    using std::get;
    using std::ranges::for_each;

    assert(tessellation_amount >= 0);

    // just here for correctness, the ranges code won't work on this
    if (tessellation_amount == 0) {
        vector<GLfloat> lattice{0.0f, 0.0f};
        return lattice;
    }

    const size_t tessellation_amount_ = tessellation_amount + 1;
    const size_t total_size = tessellation_amount_ * tessellation_amount_ * vertex_dims;
    if (total_size > numeric_limits<GLuint>::max()) {
        throw domain_error("tessellation amount is too large to be indexed by GLuint");
    }

    const GLfloat scaling = 1.0f / static_cast<GLfloat>(tessellation_amount);

    vector<GLfloat> lattice;
    lattice.reserve(total_size);

    const iota_view tessellation{(size_t)0, tessellation_amount_};
    // clang-format off
    auto result = cartesian_product(tessellation, tessellation)
        | std::views::transform([scaling](auto pt) { 
            auto const x = static_cast<GLfloat>(get<0>(pt)) * scaling - 0.5f;
            auto const y = static_cast<GLfloat>(get<1>(pt)) * scaling - 0.5f;
            vector<GLfloat> point{x, y}; // let shader calculate 3d function
            return point;
        })
        | std::views::join;
    // clang-format on

    std::ranges::copy(result.cbegin(), result.cend(), std::back_inserter(lattice));
    return lattice;
}

/**
 * assumes points will be 3 sequential floats following
 * layout from make_lattice
 */
vector<GLuint> lattice_points_list(GLuint tessellation_amount) {
    assert(tessellation_amount >= 0);

    if (tessellation_amount == 0) {
        vector<GLuint> indices{0};
        return indices;
    }

    const auto count = static_cast<size_t>(pow(static_cast<double>(tessellation_amount), 2.0));

    if (count > numeric_limits<GLuint>::max()) {
        throw domain_error("tessellation amount is too large to be indexed by GLuint");
    }

    vector<GLuint> indices_list;
    indices_list.reserve(count);

    // two adjacent CCW triangles
    const vector<GLuint> two_triangles_pattern{0, tessellation_amount + 1, tessellation_amount + 2,
                                               0, tessellation_amount + 2, 1};

    const iota_view times{(size_t)0, count};

    // clang-format off
    auto result = times | std::views::transform([&two_triangles_pattern](size_t idx) {
        return two_triangles_pattern 
            | std::views::transform([idx](auto idx_) { return idx + idx_; })
            | std::ranges::to<vector>();
    }) | std::views::join;
    // clang-format on

    std::ranges::copy(result.cbegin(), result.cend(), std::back_inserter(indices_list));
    return indices_list;
}
