#pragma once

#include "gl_inspect.hpp"
#include "glad/glad.h"

#include <format>
#include <stdexcept>
#include <string>

/** top-level opengl related error */
class WrappedOpenGLError : public std::runtime_error {
public:
    WrappedOpenGLError(std::string const &msg) : std::runtime_error(msg) {
    }
    WrappedOpenGLError(const char *msg) : std::runtime_error(msg) {
    }
};

/** error with an opengl shader */
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

/** failed to compile opengl shader */
class ShaderCompilationError : public ShaderError {
public:
    ShaderCompilationError(std::string &msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
    ShaderCompilationError(const char *msg, GLenum shader_type) : ShaderError(msg, shader_type) {
    }
};

/** the opengl shader program encountered an error */
class ShaderProgramError : public WrappedOpenGLError {
public:
    ShaderProgramError(std::string &msg) : WrappedOpenGLError(msg) {
    }
    ShaderProgramError(const char *msg) : WrappedOpenGLError(msg) {
    }
};

/** could not link the opengl shader program */
class ShaderProgramLinkerError : public ShaderProgramError {
public:
    ShaderProgramLinkerError(std::string &msg) : ShaderProgramError(msg) {
    }
    ShaderProgramLinkerError(const char *msg) : ShaderProgramError(msg) {
    }
};

/** top-level problem with sdl input / output */
class InputError : public std::runtime_error {
public:
    InputError(std::string const &msg) : std::runtime_error(msg) {
    }
    InputError(const char *msg) : std::runtime_error(msg) {
    }
};
