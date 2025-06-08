#pragma once
#include "gl_inspect.hpp"
#include "glad/glad.h"

class TessellationSettings {
    GLint tessellation_level;
    bool hardware_tessellation_supported;

public:
    TessellationSettings();
    TessellationSettings(GLint tessellation_level);

    /**
     * @return true if the level is possible
     */
    bool set_level(GLint new_level);

    /**
     * @return the current tessellation level
     */
    GLint get_level() const;

    /**
     * @return if hardware tessellation is supported
     */
    bool is_hardware_tessellation_supported() const;
};
