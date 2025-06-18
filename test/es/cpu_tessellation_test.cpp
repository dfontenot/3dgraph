#include "es/cpu_tessellation.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(CPUTessellation, InspectLattice) {
    auto const any_tessellation_amount = 4;
    auto const lattice = make_lattice(any_tessellation_amount);
    EXPECT_EQ(3 * any_tessellation_amount * any_tessellation_amount, lattice.size());

    // bottom left point
    EXPECT_FLOAT_EQ(-0.5f, lattice[0]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[1]);

    // top left point
    auto x_idx = 3 * (any_tessellation_amount - 1);
    EXPECT_FLOAT_EQ(-0.5f, lattice[x_idx]);
    EXPECT_FLOAT_EQ(0.5f, lattice[x_idx + 1]);

    // bottom right point
    x_idx = 3 * (any_tessellation_amount) * (any_tessellation_amount - 1);
    EXPECT_FLOAT_EQ(0.5f, lattice[x_idx]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[x_idx + 1]);

    // top right point
    EXPECT_FLOAT_EQ(0.5f, *(lattice.cend() - 3));
    EXPECT_FLOAT_EQ(0.5f, *(lattice.cend() - 2));
}

TEST(CPUTessellation, IBOPoints) {
    auto const any_tessellation_amount = 4;
    auto const points_per_square = 8;
    const auto expected_subdivision_count =
        static_cast<size_t>(pow(static_cast<double>(any_tessellation_amount - 1), 2.0));

    auto const lattice_points_for_ibo = lattice_points_list(any_tessellation_amount);
    EXPECT_EQ(lattice_points_for_ibo.size(), expected_subdivision_count * points_per_square);
}
