#pragma once

#include "gl_inspect.hpp"

#include "glad/glad.h"
#include <stdexcept>
#include <string>

class WrappedOpenGLError : public std::runtime_error {
public:
    WrappedOpenGLError(std::string &msg) : runtime_error(msg) {
    }
    WrappedOpenGLError(std::string &&msg) : runtime_error(msg) {
    }
    WrappedOpenGLError(char *msg) : runtime_error(msg) {
    }
    WrappedOpenGLError(const char *msg) : runtime_error(msg) {
    }
};

class ShaderCompilationError : public WrappedOpenGLError {
public:
    ShaderCompilationError(std::string &msg, GLenum shader_type)
        : WrappedOpenGLError(shader_type_to_string(shader_type) + " " + msg) {
    }
    ShaderCompilationError(char *msg, GLenum shader_type)
        : WrappedOpenGLError(shader_type_to_string(shader_type) + " " + msg) {
    }
    ShaderCompilationError(const char *msg, GLenum shader_type)
        : WrappedOpenGLError(shader_type_to_string(shader_type) + " " + msg) {
    }
};

class ShaderProgramCompilationError : public WrappedOpenGLError {
public:
    ShaderProgramCompilationError(std::string &msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramCompilationError(char *msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramCompilationError(const char *msg) : WrappedOpenGLError(msg) {
    }
};
