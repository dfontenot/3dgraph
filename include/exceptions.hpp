#pragma once

#include "gl_inspect.hpp"
#include "glad/glad.h"

#include <format>
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

class ShaderError : public WrappedOpenGLError {
public:
    ShaderError(std::string &msg, GLenum shader_type)
        : WrappedOpenGLError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }
    ShaderError(std::string &&msg, GLenum shader_type)
        : WrappedOpenGLError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }
    ShaderError(char *msg, GLenum shader_type)
        : WrappedOpenGLError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }
    ShaderError(const char *msg, GLenum shader_type)
        : WrappedOpenGLError(std::format("{0} {1}", shader_type_to_string(shader_type), msg)) {
    }
};

class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(std::string &msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
    ShaderCompilationError(std::string &&msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
    ShaderCompilationError(char *msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
    ShaderCompilationError(const char *msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
};

class ShaderProgramError : public WrappedOpenGLError {
public:
    ShaderProgramError(std::string &msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramError(std::string &&msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramError(char *msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramError(const char *msg) : WrappedOpenGLError(msg) {
    }
};

class ShaderProgramLinkerError : public ShaderProgramError {
public:
    ShaderProgramLinkerError(std::string &msg) : ShaderProgramError(msg) {
    }
    ShaderProgramLinkerError(std::string &&msg) : ShaderProgramError(msg) {
    }
    ShaderProgramLinkerError(char *msg) : ShaderProgramError(msg) {
    }
    ShaderProgramLinkerError(const char *msg) : ShaderProgramError(msg) {
    }
};

