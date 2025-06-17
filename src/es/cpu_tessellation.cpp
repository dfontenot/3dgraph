#include "es/cpu_tessellation.hpp"
#include "glad/glad.h"
#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

using std::vector;
using std::ranges::iota_view;
using std::ranges::views::cartesian_product;

vector<GLfloat> make_lattice(size_t tessellation_amount) {
    using std::get;
    using std::ranges::for_each;

    const size_t total_size = tessellation_amount * tessellation_amount * 3; // 3 dims per vertex
    const GLfloat scaling = 1.0f / static_cast<GLfloat>(tessellation_amount);
    vector<GLfloat> lattice;
    lattice.reserve(total_size);

    const iota_view tessellation{(size_t)0, tessellation_amount};
    // clang-format off
    auto result = cartesian_product(tessellation, tessellation)
        | std::views::transform([scaling](auto pt) { 
            const vector<GLfloat> point{get<0>(pt) * scaling, get<1>(pt) * scaling, 0.0f};
            return point;
        })
        | std::views::join;
    // clang-format on

    std::ranges::copy(result.cbegin(), result.cend(), std::back_inserter(lattice));
    return lattice;
}
