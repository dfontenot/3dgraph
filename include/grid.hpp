#pragma once

#include "es/grid_points.hpp"
#include "shader_program.hpp"
#include "vertices.hpp"

#include <cstdint>
#include <memory>
#include <variant>

class Grid {
    std::variant<Vertices, GridPoints> verts;
    std::shared_ptr<ShaderProgram> program;

public:
    Grid() = delete;
    Grid(const Grid &) = delete;
    Grid(Grid &&) noexcept = default;
    Grid &operator=(const Grid &) noexcept = delete;
    Grid &operator=(Grid &&) noexcept = delete;

    Grid(Vertices &&verts, std::shared_ptr<ShaderProgram> const &shader_program) noexcept
        : verts(std::move(verts)), program(shader_program) {
    }

    Grid(GridPoints &&grid_points, std::shared_ptr<ShaderProgram> const &shader_program) noexcept
        : verts(std::move(grid_points)), program(shader_program) {
    }

    // NOLINTNEXTLINE(modernize-use-nodiscard)
    uint64_t render() const;
};
