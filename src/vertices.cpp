#include "vertices.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include <memory>

using std::shared_ptr;

shared_ptr<Vao> Vertices::get_vao() const noexcept {
    return vao;
}

shared_ptr<Vbo> Vertices::get_vbo() const noexcept {
    return vbo;
}

size_t Vertices::get_vert_count() const noexcept {
    return size / points_per_vertex;
}

