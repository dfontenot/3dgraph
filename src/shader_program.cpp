#include <algorithm>
#include <cassert>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "exceptions.hpp"
#include "function_params.hpp"
#include "gl_inspect.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#include "shader_program.hpp"
#include "tessellation_settings.hpp"

using glm::mat4;
using glm::value_ptr;

using std::cerr;
using std::endl;
using std::for_each;
using std::format;
using std::initializer_list;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::vector;

void ShaderProgram::link_shaders() {
    using std::make_unique;

    // preconditions
    assert(program_handle != 0);

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(
            format("precondition failed to init shader program: {}", gl_get_error_string(current_error)));
    }

    logger->debug("will link {} shaders", attached_shaders.size());

    for_each(attached_shaders.cbegin(), attached_shaders.cend(),
             [&](const shared_ptr<Shader> &shader) { glAttachShader(program_handle, shader->shader_handle); });

    glLinkProgram(program_handle);

    GLint linked = -1;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &linked);

    GLsizei to_allocate = -1;
    glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &to_allocate);
    if (to_allocate > 1) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
        auto linker_log = make_unique<GLchar[]>(to_allocate);
        glGetProgramInfoLog(program_handle, to_allocate, nullptr, linker_log.get());

        if (linked == GL_FALSE) {
            throw ShaderProgramLinkerError(linker_log.get());
        }
        else {
            logger->error(linker_log.get());
        }
    }

    assert(linked == GL_TRUE);

    // progam has to be in use first https://stackoverflow.com/a/36416867
    glUseProgram(program_handle);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("program issue: {}", gl_get_error_string(current_error)));
    }

    for (auto variable_name : uniform_variable_names) {
        // clean up? very impl specific, might benefit from more general mechanism
        if (variable_name == tessellation_level_variable_name &&
            !tessellation_settings->is_hardware_tessellation_supported()) {
            continue;
        }

        GLint location = glGetUniformLocation(program_handle, variable_name);
        if (location < 0) {
            throw WrappedOpenGLError(format("unable to find uniform {}", variable_name));
        }

        uniform_locations[variable_name] = location;
    }

    glUseProgram(0);
}

ShaderProgram::ShaderProgram(vector<shared_ptr<Shader>> &&shaders, shared_ptr<glm::mat4> const &model,
                             shared_ptr<glm::mat4> const &view, shared_ptr<glm::mat4> const &projection,
                             shared_ptr<FunctionParams> const &function_params,
                             shared_ptr<TessellationSettings> const &tessellation_settings)
    : program_handle(glCreateProgram()), in_use(false), attached_shaders(std::move(shaders)), model(model), view(view),
      projection(projection), function_params(function_params), tessellation_settings(tessellation_settings),
      logger(spdlog::stderr_color_mt("shader_program")), err(spdlog::stderr_color_mt("shader_program_err")) {
    link_shaders();
}

ShaderProgram::~ShaderProgram() {
    auto detach_shader = [&](const shared_ptr<Shader> &shader) {
        glDetachShader(program_handle, shader->shader_handle);
    };

    for_each(attached_shaders.cbegin(), attached_shaders.cend(), detach_shader);
    logger->trace("deleting shader program");
    glDeleteProgram(program_handle);
}

void ShaderProgram::use() {
    if (in_use) {
        return;
    }

    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(
            format("cannot use program due to existing error: {}", gl_get_error_string(current_error)));
    }

    glUseProgram(program_handle);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("error using the shader program: {}", gl_get_error_string(current_error)));
    }

    in_use = true;
}

void ShaderProgram::release() {
    if (!in_use) {
        return;
    }

    glUseProgram(0);
    in_use = false;
}

bool ShaderProgram::is_in_use() const {
    return in_use;
}

void ShaderProgram::update_function_params() {
    set_uniform_1f(offset_x_uniform_variable_name, function_params->x_offset);
    set_uniform_1f(offset_y_uniform_variable_name, function_params->y_offset);
    set_uniform_1f(z_mult_uniform_variable_name, function_params->z_mult);
}

void ShaderProgram::set_uniform_1f(const GLchar *uniform_variable_name, GLfloat value) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(
            format("couldn't update uniforms due to existing error: {}", gl_get_error_string(current_error)));
    }

    glUniform1f(uniform_locations[uniform_variable_name], value);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("error setting uniform {0} {1} at location {2}", uniform_variable_name,
                                        gl_get_error_string(current_error), uniform_locations[uniform_variable_name]));
    }
}

void ShaderProgram::set_uniform_1ui(const GLchar *uniform_variable_name, GLuint value) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(
            format("couldn't update uniforms due to existing error: {}", gl_get_error_string(current_error)));
    }

    glUniform1ui(uniform_locations[uniform_variable_name], value);

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("error setting uniform {0} {1} at location {2}", uniform_variable_name,
                                        gl_get_error_string(current_error), uniform_locations[uniform_variable_name]));
    }
}

void ShaderProgram::update_model() {
    set_uniform_matrix_4fv(model_uniform_variable_name, model);
}

void ShaderProgram::update_view() {
    set_uniform_matrix_4fv(view_uniform_variable_name, view);
}

void ShaderProgram::update_projection() {
    set_uniform_matrix_4fv(projection_uniform_variable_name, projection);
}

void ShaderProgram::set_uniform_matrix_4fv(const GLchar *uniform_variable_name, shared_ptr<mat4> const &value) {
    auto current_error = glGetError();

    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError("cannot update uniforms due to existing error: " + gl_get_error_string(current_error));
    }

    glUniformMatrix4fv(uniform_locations[uniform_variable_name], 1, GL_FALSE, value_ptr(*value));

    if ((current_error = glGetError()) != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("error setting uniform {0} {1} at location {2}", uniform_variable_name,
                                        gl_get_error_string(current_error), uniform_locations[uniform_variable_name]));
    }
}

void ShaderProgram::update_tessellation_settings() {
    if (tessellation_settings->is_hardware_tessellation_supported()) {
        set_uniform_1ui(tessellation_level_variable_name, tessellation_settings->get_level());
    }
}

void ShaderProgram::set_initial_uniforms() {
    update_function_params();
    update_model();
    update_view();
    update_projection();

#ifndef OPENGL_ES
    update_tessellation_settings();
#endif
}
