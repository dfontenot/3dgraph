#include <gtest/gtest.h>
#include "es/cpu_tessellation.hpp"

TEST(CPUTessellation, InspectLattice) {
    auto const any_tessellation_amount = 4;
    auto const lattice = make_lattice(any_tessellation_amount);
    EXPECT_EQ(3 * any_tessellation_amount * any_tessellation_amount, lattice.size());
}
