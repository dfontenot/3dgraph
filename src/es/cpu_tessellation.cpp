#include "es/cpu_tessellation.hpp"
#include "glad/glad.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <iterator>
#include <ranges>
#include <vector>

using std::size_t;
using std::vector;
using std::ranges::iota_view;
using std::ranges::views::cartesian_product;

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
vector<GLfloat> make_lattice(size_t tessellation_amount) {
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
vector<GLuint> lattice_points_list(size_t tessellation_amount) {
    assert(tessellation_amount >= 0);

    if (tessellation_amount == 0) {
        vector<GLuint> indices{0};
        return indices;
    }

    const auto count = static_cast<size_t>(pow(static_cast<double>(tessellation_amount), 2.0));
    vector<GLuint> indices_list;
    indices_list.reserve(count);

    // two adjacent CCW triangles
    const vector<GLuint> two_triangles_pattern{0, 4, 5, 0, 5, 1};

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
