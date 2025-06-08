#include "tessellation_settings.hpp"
#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"
#include <optional>

using std::optional;

static constexpr const GLint default_tessellation_level = 9;

// minimum value for max hardware tessellation level is 64
static constexpr const GLint max_software_tessellation_level = 128;

namespace {
static const optional<GLuint> max_tessellation_level = get_max_tessellation_level();
bool tessellation_level_ok(GLuint requested_level) {
    if ((max_tessellation_level.has_value() && requested_level > *max_tessellation_level) ||
        (!max_tessellation_level.has_value() && requested_level > max_software_tessellation_level)) {
        return false;
    }

    return true;
}

GLint tessellation_level_or_throw(GLint requested_level) {
    if (!tessellation_level_ok(requested_level)) {
        WrappedOpenGLError("invalid tessellation level");
    }

    return requested_level;
}
} // namespace

TessellationSettings::TessellationSettings()
    : tessellation_level(default_tessellation_level),
      hardware_tessellation_supported(max_tessellation_level.has_value()) {
}

TessellationSettings::TessellationSettings(GLint tessellation_level)
    : tessellation_level(tessellation_level_or_throw(tessellation_level)),
      hardware_tessellation_supported(max_tessellation_level.has_value()) {
}

bool TessellationSettings::set_level(GLint new_level) {
    if (!tessellation_level_ok(new_level)) {
        return false;
    }

    tessellation_level = new_level;
    return true;
}

GLuint TessellationSettings::get_level() const {
    return tessellation_level;
}

bool TessellationSettings::is_hardware_tessellation_supported() const {
    return hardware_tessellation_supported;
}
