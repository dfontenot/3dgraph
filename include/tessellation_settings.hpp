#pragma once
#include "gl_inspect.hpp"
#include "glad/glad.h"

class TessellationSettings {
    GLuint tessellation_level;
    bool hardware_tessellation_supported;

public:
    /**
     * prereq: must have opengl initialized before calling
     */
    TessellationSettings();
    TessellationSettings(GLuint tessellation_level);

    /**
     * @return true if the level is possible
     */
    bool set_level(GLuint new_level);

    /**
     * @return true if the new level is possible
     */
    bool increment_level();

    /**
     * @return true if the new level is possible
     */
    bool decrement_level();

    /**
     * @return the current tessellation level
     */
    GLuint get_level() const;

    /**
     * @return if hardware tessellation is supported
     */
    bool is_hardware_tessellation_supported() const;
};
