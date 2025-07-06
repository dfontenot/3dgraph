#include "es/cpu_tessellation.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

using std::vector;

/** grid, 2 dimensions only */
constexpr const auto vertex_dims = 2;

TEST(CPUTessellation, TessellationLevelZero) {
    // will result in a single point
    auto const lattice = make_lattice(0);
    EXPECT_EQ(vertex_dims, lattice.size());

    EXPECT_FLOAT_EQ(0.0f, lattice[0]);
    EXPECT_FLOAT_EQ(0.0f, lattice[1]);
}

TEST(CPUTessellation, TessellationLevelOne) {
    // will result in a square
    auto const lattice = make_lattice(1);
    EXPECT_EQ(vertex_dims * 4, lattice.size());

    // bottom left
    EXPECT_FLOAT_EQ(-0.5f, lattice[0]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[1]);

    // top left
    EXPECT_FLOAT_EQ(-0.5f, lattice[2]);
    EXPECT_FLOAT_EQ(0.5f, lattice[3]);

    // bottom right
    EXPECT_FLOAT_EQ(0.5f, lattice[4]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[5]);

    // top right
    EXPECT_FLOAT_EQ(0.5f, lattice[6]);
    EXPECT_FLOAT_EQ(0.5f, lattice[7]);
}

TEST(CPUTessellation, TessellationLevelTwo) {
    // will result in a square subdivided once
    auto const lattice = make_lattice(2);
    EXPECT_EQ(vertex_dims * 9, lattice.size());

    // bottom left
    EXPECT_FLOAT_EQ(-0.5f, lattice[0]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[1]);

    // middle left
    EXPECT_FLOAT_EQ(-0.5f, lattice[2]);
    EXPECT_FLOAT_EQ(0.0f, lattice[3]);

    // top left
    EXPECT_FLOAT_EQ(-0.5f, lattice[4]);
    EXPECT_FLOAT_EQ(0.5f, lattice[5]);

    // bottom middle
    EXPECT_FLOAT_EQ(0.0f, lattice[6]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[7]);

    // middle middle
    EXPECT_FLOAT_EQ(0.0f, lattice[8]);
    EXPECT_FLOAT_EQ(0.0f, lattice[9]);

    // top middle
    EXPECT_FLOAT_EQ(0.0f, lattice[10]);
    EXPECT_FLOAT_EQ(0.5f, lattice[11]);

    // bottom right
    EXPECT_FLOAT_EQ(0.5f, lattice[12]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[13]);

    // middle right
    EXPECT_FLOAT_EQ(0.5f, lattice[14]);
    EXPECT_FLOAT_EQ(0.0f, lattice[15]);

    // top right
    EXPECT_FLOAT_EQ(0.5f, lattice[16]);
    EXPECT_FLOAT_EQ(0.5f, lattice[17]);
}

TEST(CPUTessellation, TessellationLevelThree) {
    // +---+---+---+
    // |   |   |   |
    // +---+---+---+
    // |   |   |   |
    // +---+---+---+
    // |   |   |   |
    // +---+---+---+
    auto const num_divisions = 3;
    auto const lattice = make_lattice(num_divisions);
    EXPECT_EQ(vertex_dims * (num_divisions + 1) * (num_divisions + 1), lattice.size());

    GLfloat const increment = 1.0f / static_cast<GLfloat>(num_divisions);
    // ordering of tessellated points: start on bottom left go up 1 col at a time

    // left-most col
    EXPECT_FLOAT_EQ(-0.5f, lattice[0]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[1]);

    EXPECT_FLOAT_EQ(-0.5f, lattice[2]);
    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[3]);

    EXPECT_FLOAT_EQ(-0.5f, lattice[4]);
    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[5]);

    EXPECT_FLOAT_EQ(-0.5f, lattice[6]);
    EXPECT_FLOAT_EQ(0.5f, lattice[7]);

    // left-inner col
    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[8]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[9]);

    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[10]);
    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[11]);

    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[12]);
    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[13]);

    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[14]);
    EXPECT_FLOAT_EQ(0.5f, lattice[15]);

    // right-inner col
    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[16]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[17]);

    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[18]);
    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[19]);

    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[20]);
    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[21]);

    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[22]);
    EXPECT_FLOAT_EQ(0.5f, lattice[23]);

    // right-most col
    EXPECT_FLOAT_EQ(0.5f, lattice[24]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[25]);

    EXPECT_FLOAT_EQ(0.5f, lattice[26]);
    EXPECT_FLOAT_EQ(-0.5f + increment, lattice[27]);

    EXPECT_FLOAT_EQ(0.5f, lattice[28]);
    EXPECT_FLOAT_EQ(-0.5f + increment * 2.0f, lattice[29]);

    EXPECT_FLOAT_EQ(0.5f, lattice[30]);
    EXPECT_FLOAT_EQ(0.5f, lattice[31]);
}

TEST(CPUTessellation, InspectLatticeCorners) {
    auto const any_tessellation_amount = 4;
    auto const lattice = make_lattice(any_tessellation_amount);
    EXPECT_EQ(vertex_dims * (any_tessellation_amount + 1) * (any_tessellation_amount + 1), lattice.size());

    // bottom left point
    EXPECT_FLOAT_EQ(-0.5f, lattice[0]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[1]);

    // top left point
    auto x_idx = vertex_dims * (any_tessellation_amount);
    EXPECT_FLOAT_EQ(-0.5f, lattice[x_idx]);
    EXPECT_FLOAT_EQ(0.5f, lattice[x_idx + 1]);

    // bottom right point
    x_idx = vertex_dims * (any_tessellation_amount + 1) * (any_tessellation_amount);
    EXPECT_FLOAT_EQ(0.5f, lattice[x_idx]);
    EXPECT_FLOAT_EQ(-0.5f, lattice[x_idx + 1]);

    // top right point
    EXPECT_FLOAT_EQ(0.5f, *(lattice.cend() - 2));
    EXPECT_FLOAT_EQ(0.5f, *(lattice.cend() - 1));
}

TEST(CPUTessellation, LatticePointsZero) {
    const vector<GLuint> expected_ibo{0};
    auto const lattice_points_for_ibo = lattice_points_list(0);
    EXPECT_EQ(expected_ibo, lattice_points_for_ibo);
}

TEST(CPUTessellation, LatticePointsOne) {
    const vector<GLuint> expected_ibo{0, 2, 3, 0, 3, 1};
    auto const lattice_points_for_ibo = lattice_points_list(1);
    EXPECT_EQ(expected_ibo, lattice_points_for_ibo);
}

TEST(CPUTessellation, LatticePoints) {
    auto const any_tessellation_amount = 4;
    auto const points_per_square = 6;
    /** the number of squares in the lattice */
    auto const expected_subdivision_count = static_cast<size_t>(pow(static_cast<double>(any_tessellation_amount), 2.0));

    auto const lattice_points_for_ibo = lattice_points_list(any_tessellation_amount);
    EXPECT_EQ(lattice_points_for_ibo.size(), expected_subdivision_count * points_per_square);

    /** how many individual vertices will there be in the lattice */
    auto const expected_num_points = static_cast<GLuint>(pow(static_cast<double>(any_tessellation_amount + 1), 2.0));
    EXPECT_TRUE(std::ranges::all_of(lattice_points_for_ibo,
                                    [expected_num_points](auto idx) { return idx < expected_num_points; }));
}
