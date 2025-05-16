#pragma once

#include "glad/glad.h"

#include <memory>

class ShaderProgram;

class Shader {
    static constexpr GLsizei number_of_sources = 1; // only supporting 1 source per shader type
    static constexpr GLint *source_lengths = 0; // can be set to 0 since source ends with a null terminator

    const GLuint shader_handle;

public:
    const GLenum shader_type;

    Shader() = delete;
    Shader(const char *source_fn, GLenum shader_type);
    ~Shader();

    friend class ShaderProgram;
};

std::shared_ptr<Shader> make_shader(const char *source_fn, GLenum shader_type);
