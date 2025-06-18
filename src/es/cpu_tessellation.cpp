#include "es/cpu_tessellation.hpp"
#include "glad/glad.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <ranges>
#include <vector>

using std::vector;
using std::ranges::iota_view;
using std::ranges::views::cartesian_product;

/**
* @brief makes a lattice mesh in 3 dimensions (plane)
* order of points will be bottom left corner to top left corner, then towards right side
* points in order x0, y0, z0, x1, y1, z1, ...
*/
vector<GLfloat> make_lattice(size_t tessellation_amount) {
    using std::get;
    using std::ranges::for_each;

    assert(tessellation_amount > 0);

    const size_t total_size = tessellation_amount * tessellation_amount * 3; // 3 dims per vertex
    const GLfloat scaling = 1.0f / static_cast<GLfloat>(tessellation_amount - 1);
    vector<GLfloat> lattice;
    lattice.reserve(total_size);

    const iota_view tessellation{(size_t)0, tessellation_amount};
    // clang-format off
    auto result = cartesian_product(tessellation, tessellation)
        | std::views::transform([scaling](auto pt) { 
            auto const x = static_cast<GLfloat>(get<0>(pt)) * scaling - 0.5f;
            auto const y = static_cast<GLfloat>(get<1>(pt)) * scaling - 0.5f;
            const vector<GLfloat> point{x, y, 0.0f}; // let shader calculate 3d function
            return point;
        })
        | std::views::join;
    // clang-format on

    std::ranges::copy(result.cbegin(), result.cend(), std::back_inserter(lattice));
    return lattice;
}
