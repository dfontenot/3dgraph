#pragma once
#include "gl_inspect.hpp"
#include "glad/glad.h"

class TessellationSettings {
    GLint tessellation_level;
    bool hardward_tessellation_supported;

public:
    TessellationSettings();
    TessellationSettings(GLint tessellation_level);

    /**
     * @return true if the level is possible
     */
    bool set_level(GLint new_level);
    constexpr operator GLuint() const;
};
