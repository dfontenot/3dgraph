#pragma once

#include "glad/glad.h"
#include "spdlog/logger.h"

#include <filesystem>
#include <format>
#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

class ShaderProgram;

class Shader {
    static constexpr const GLsizei number_of_sources = 1;   // only supporting 1 source per shader type
    static constexpr const GLint *source_lengths = nullptr; // can be set to 0 since source ends with a null terminator

    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> err;
    GLuint shader_handle;
    GLenum shader_type;

    friend std::ostream &operator<<(std::ostream &stream, const Shader &shader);
    friend std::formatter<Shader>;

public:
    Shader() = delete;
    Shader(Shader const &) = delete; // TODO relax this
    Shader(Shader &&) = default;

    Shader(const char *source_fn, GLenum shader_type);
    Shader(const std::string &source_fn, GLenum shader_type);
    Shader(const std::filesystem::path &source_path, GLenum shader_type);
    ~Shader();

    [[nodiscard]] GLenum get_shader_type() const noexcept;

    friend class ShaderProgram;
};
