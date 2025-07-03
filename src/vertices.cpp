#include "vertices.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include <iostream>
#include <memory>

using std::shared_ptr;

std::ostream &operator<<(std::ostream &stream, const Vertices &vertices) {
    stream << " { Vertices: size " << vertices.size << " points per vertex: " << vertices.points_per_vertex << "}";
    return stream;
}

template <> struct std::formatter<Vertices> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const Vertices &obj, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{ Vertices: size {0} points per vertex {1} }", obj.size, obj.points_per_vertex);
    }
};

shared_ptr<Vao> Vertices::get_vao() const noexcept {
    return vao;
}

shared_ptr<Vbo> Vertices::get_vbo() const noexcept {
    return vbo;
}

size_t Vertices::get_vert_count() const noexcept {
    return size / points_per_vertex;
}

