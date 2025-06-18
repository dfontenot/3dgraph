#pragma once

#include "ibo.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class GridPoints {
    std::shared_ptr<Ibo> ibo;
    std::vector<GLfloat> triangles_points;
    std::vector<GLuint> indices;
    std::size_t tessellation_amount;

public:
    GridPoints(std::size_t tessellation_amount);

    std::shared_ptr<Ibo> get_ibo() const noexcept;
    std::size_t get_tessellation_amount() const noexcept;
};
