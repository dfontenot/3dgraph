#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#include "shader_program.hpp"

using glm::mat4;
using glm::value_ptr;

using std::cerr;
using std::endl;
using std::for_each;
using std::initializer_list;
using std::shared_ptr;
using std::string;
using std::stringstream;

ShaderProgram::ShaderProgram(initializer_list<shared_ptr<Shader>> shaders)
    : program_handle(glCreateProgram()), attached_shaders(shaders) {
    using std::make_unique;

    assert(program_handle != 0);

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("precondition failed to init shader program: " + gl_get_error_string(current_error));
    }

    for_each(attached_shaders.cbegin(), attached_shaders.cend(),
             [&](const shared_ptr<Shader> &shader) { glAttachShader(program_handle, shader->shader_handle); });

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

    // progam has to be in use first https://stackoverflow.com/a/36416867
    glUseProgram(program_handle);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError("program issue: " + gl_get_error_string(current_error));
    }

    for (auto variable_name : uniform_variable_names) {
        GLint location = glGetUniformLocation(program_handle, variable_name);
        if (location < 0) {
            string msg = "unable to find uniform ";
            throw WrappedOpenGLError(msg + variable_name);
        }

        uniform_locations[variable_name] = location;
    }

    glUseProgram(0);
}

ShaderProgram::~ShaderProgram() {
    auto detach_shader = [&](const shared_ptr<Shader> &shader) {
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

void ShaderProgram::release() {
    glUseProgram(0);
}

void ShaderProgram::set_offset_x(GLfloat offset_x) {
    set_uniform_1f(offset_x_uniform_variable_name, offset_x);
}

void ShaderProgram::set_offset_y(GLfloat offset_y) {
    set_uniform_1f(offset_y_uniform_variable_name, offset_y);
}

void ShaderProgram::set_z_mult(GLfloat offset_z) {
    set_uniform_1f(z_mult_uniform_variable_name, offset_z);
}

void ShaderProgram::set_uniform_1f(const GLchar *uniform_variable_name, GLfloat value) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("update uniforms due to existing error: " + gl_get_error_string(current_error));
    }

    glUniform1f(uniform_locations[uniform_variable_name], value);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        stringstream ss;
        ss << "error setting uniform " << uniform_variable_name << " " << gl_get_error_string(current_error)
           << " at location " << uniform_locations[uniform_variable_name] << endl;
        throw WrappedOpenGLError(ss.str());
    }
}

void ShaderProgram::set_model(const glm::mat4 &model) {
    set_uniform_matrix_4fv(model_uniform_variable_name, model);
}

void ShaderProgram::set_view(const glm::mat4 &view) {
    set_uniform_matrix_4fv(view_uniform_variable_name, view);
}

void ShaderProgram::set_projection(const glm::mat4 &projection) {
    set_uniform_matrix_4fv(projection_uniform_variable_name, projection);
}

void ShaderProgram::set_uniform_matrix_4fv(const GLchar *uniform_variable_name, const mat4 &value) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("cannot update uniforms due to existing error: " + gl_get_error_string(current_error));
    }

    glUniformMatrix4fv(uniform_locations[uniform_variable_name], 1, GL_FALSE, value_ptr(value));

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        stringstream ss;
        ss << "error setting uniform matrix " << uniform_variable_name << " " << gl_get_error_string(current_error)
           << " at location " << uniform_locations[uniform_variable_name] << endl;
        throw WrappedOpenGLError(ss.str());
    }
}
