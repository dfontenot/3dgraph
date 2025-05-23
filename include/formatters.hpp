#pragma once

#include <fmt/format.h>
#include <glm/gtc/quaternion.hpp>

template <> struct fmt::formatter<glm::quat> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const glm::quat &obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "quat {0} {1} {2} {3}", obj.w, obj.x, obj.y, obj.z);
    }
};
