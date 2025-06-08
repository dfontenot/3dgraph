#pragma once

#include "glad/glad.h"
#include <string>
#include <optional>

/**
 * @brief similar to glu
 * @return string representation of the shader type (one word)
 */
std::string shader_type_to_string(GLenum shader_type);

/**
 * @brief similar to gluErrorString
 * @return string rep of the error
 */
std::string gl_get_error_string(GLenum err);

/**
 * @brief similar to gluErrorString
 * @return string rep of the error
 */
std::string gl_get_error_string();

/**
 * @return get max tessellation level, or nullopt on unsupported
 */
std::optional<GLuint> get_max_tessellation_level();
