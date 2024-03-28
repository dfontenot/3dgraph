#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <range/v3/view/indices.hpp>
#include <range/v3/view/zip.hpp>

#include "exceptions.hpp"
#include "glad/glad.h"
#include "shader_program.hpp"
#include "shader.hpp"

using std::cerr;
using std::endl;
using std::for_each;
using std::initializer_list;
using std::shared_ptr;
using std::vector;
using std::string;

ShaderProgram::ShaderProgram(initializer_list<shared_ptr<Shader>> shaders) : program_handle(glCreateProgram()), attached_shaders(shaders) {
    using std::make_unique;
    using namespace ranges;

    for_each(attached_shaders.cbegin(), attached_shaders.cend(), [&](const shared_ptr<Shader>& shader) {
        glAttachShader(program_handle, shader->shader_handle);
    });

    GLuint pos = 0;
    for (auto variable_name : uniform_variable_names) {
        glBindAttribLocation(program_handle, pos++, variable_name);
    }

    glLinkProgram(program_handle);

    GLint linked;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &linked);

    GLsizei to_allocate;
    glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &to_allocate);
    if (to_allocate > 1) {
        auto linker_log = make_unique<GLchar[]>(to_allocate);
        glGetProgramInfoLog(program_handle, to_allocate, nullptr, linker_log.get());

        if (linked == GL_FALSE) {
            throw ShaderProgramCompilationError(linker_log.get());
        }
        else {
            cerr << linker_log.get() << endl;
        }
    }

    assert(linked == GL_TRUE);

    for (const auto& idx_var_name : views::zip(views::indices, uniform_variable_names)) {
        auto variable_name = idx_var_name.second;
        GLint location = glGetUniformLocation(program_handle, variable_name);
        if (location < 0) {
            string msg = "unable to find uniform ";
            throw WrappedOpenGLError(msg + variable_name);
        }

        uniform_locations[variable_name] = location;
        uniform_types[variable_name] = uniform_variable_types[idx_var_name.first];
    }
}

ShaderProgram::~ShaderProgram() {
    auto detach_shader = [&](const shared_ptr<Shader>& shader) {
        glDetachShader(program_handle, shader->shader_handle);
    };

    for_each(attached_shaders.cbegin(), attached_shaders.cend(), detach_shader);
    glDeleteProgram(program_handle);
}

void ShaderProgram::use() {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("cannot use program due to existing error: " + gl_get_error_string(current_error));
    }

    glUseProgram(program_handle);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError("error using the shader program: " + gl_get_error_string(current_error));
    }
}

void ShaderProgram::update_uniforms(GLfloat offset_x, GLfloat offset_y) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("update uniforms due to existing error: " + gl_get_error_string(current_error));
    }

    glUniform1f(uniform_locations[offset_x_uniform_variable_name], offset_x);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError("error setting x_offset uniform: " + gl_get_error_string(current_error));
    }

    glUniform1f(uniform_locations[offset_y_uniform_variable_name], offset_y);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError("error setting y_offset uniform: " + gl_get_error_string(current_error));
    }
}
