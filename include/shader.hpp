#pragma once

#include "glad/glad.h"

#include <filesystem>
#include <string>

class ShaderProgram;

class Shader {
    static constexpr const GLsizei number_of_sources = 1; // only supporting 1 source per shader type
    static constexpr const GLint *source_lengths = nullptr;     // can be set to 0 since source ends with a null terminator

    const GLuint shader_handle;

public:
    const GLenum shader_type;

    Shader() = delete;
    Shader(const char *source_fn, GLenum shader_type);
    Shader(const std::string &source_fn, GLenum shader_type);
    Shader(const std::filesystem::path &source_path, GLenum shader_type);
    ~Shader();

    friend class ShaderProgram;
};

