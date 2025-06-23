#pragma once

#include "gl_inspect.hpp"
#include "glad/glad.h"

#include <format>
#include <stdexcept>
#include <string>

class WrappedOpenGLError : public std::runtime_error {
public:
    WrappedOpenGLError(std::string const &msg) : runtime_error(msg) {
    }
    WrappedOpenGLError(const char *msg) : runtime_error(msg) {
    }
};

class ShaderError : public WrappedOpenGLError {
public:
    ShaderError(const std::string &msg) : WrappedOpenGLError(msg) {
    }

    ShaderError(const char *msg) : WrappedOpenGLError(msg) {
    }

    ShaderError(std::string const &msg, GLenum shader_type)
        : ShaderError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }

    ShaderError(const char *msg, GLenum shader_type)
        : ShaderError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }
};

class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(std::string &msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
    ShaderCompilationError(const char *msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
};

class ShaderProgramError : public WrappedOpenGLError {
public:
    ShaderProgramError(std::string &msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramError(const char *msg) : WrappedOpenGLError(msg) {
    }
};

class ShaderProgramLinkerError : public ShaderProgramError {
public:
    ShaderProgramLinkerError(std::string &msg) : ShaderProgramError(msg) {
    }
    ShaderProgramLinkerError(const char *msg) : ShaderProgramError(msg) {
    }
};
