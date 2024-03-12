#pragma once

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "glad/glad.h"
#include "shader.hpp"

class ShaderProgram {
    //uniforms
    static constexpr const GLchar* offset_x_uniform_variable_name = "offset_x";
    static constexpr const GLchar* offset_y_uniform_variable_name = "offset_y";

    // all names
    // the attribution position of the uniform is its position in this array
    static constexpr const GLchar* uniform_variable_names[]{offset_x_uniform_variable_name, offset_y_uniform_variable_name};

    // all positions

    GLuint program_handle;
    std::vector<std::shared_ptr<Shader>> attached_shaders;
    std::unordered_map<const GLchar*, GLint> uniform_locations;

public:
    ShaderProgram() = delete;
    // TODO: more ergonomic ways of constructing this class
    ShaderProgram(std::initializer_list<std::shared_ptr<Shader>> shaders);
    ~ShaderProgram();

    void use();
    void update_uniforms(float offset_x, GLfloat offset_y);
};
