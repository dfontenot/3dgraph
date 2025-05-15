#include <array>
#include <range/v3/all.hpp>

#include "glad/glad.h"

using ranges::views::cartesian_product;
using ranges::views::iota;
using std::array;
using std::get;
using std::next;

constexpr int tesselation_amount = 10;

/**
 * @ brief creates a lattice of points (no triangles)
 * @ return a flat GLfloat array
 */
auto make_lattice() {
    using ranges::for_each;

    constexpr size_t total_size = tesselation_amount * tesselation_amount * 3; // 3 dims per vertex
    constexpr GLfloat scaling = 1.0 / static_cast<GLfloat>(tesselation_amount);
    array<GLfloat, total_size> lattice;

    const auto tesselation = iota(0, tesselation_amount);
    const auto product = cartesian_product(tesselation, tesselation);
    auto point_location = lattice.begin();
    for_each(product, [&point_location](auto pt) {
        *point_location = get<0>(pt) * scaling;
        point_location = next(point_location);
        *point_location = get<1>(pt) * scaling;
        point_location = next(point_location);
        *point_location = 0.0f; // let shader compute height
        point_location = next(point_location);
    });

    return lattice;
}
