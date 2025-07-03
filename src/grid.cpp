#include "grid.hpp"
#include "es/grid_points.hpp"
#include "vertices.hpp"

#include "glad/glad.h"
#include <SDL3/SDL.h>

#include <variant>

using std::holds_alternative;

uint64_t Grid::render() const {
    using std::get;

    // TODO: convert to std::visit
    auto const start_nsec = SDL_GetTicksNS();
    if (holds_alternative<Vertices>(verts)) {
        auto const &verts_ = get<Vertices>(verts);
        auto vao = verts_.get_vao();

        vao->bind();
        program->use();

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_PATCHES, 0, 4);

        vao->unbind();
        program->release();
    }
    else {
        auto const &verts_ = get<GridPoints>(verts);
        auto vao = verts_.get_vao();
        auto ibo = verts_.get_ibo();

        vao->bind();
        ibo->bind();
        program->use();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(verts_.get_indices_count()), GL_UNSIGNED_INT, nullptr);

        ibo->unbind();
        vao->unbind();
        program->release();
    }

    return SDL_GetTicksNS() - start_nsec;
}
