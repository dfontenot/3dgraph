#pragma once

#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/mat4x4.hpp>

#include "function_params.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#include "tessellation_settings.hpp"

class ShaderProgram {
    // uniforms
    static constexpr const GLchar *offset_x_uniform_variable_name = "u_offset_x";
    static constexpr const GLchar *offset_y_uniform_variable_name = "u_offset_y";
    static constexpr const GLchar *z_mult_uniform_variable_name = "u_z_mult";
    static constexpr const GLchar *model_uniform_variable_name = "u_model";
    static constexpr const GLchar *view_uniform_variable_name = "u_view";
    static constexpr const GLchar *projection_uniform_variable_name = "u_projection";
    static constexpr const GLchar *tessellation_level_variable_name = "u_tess_level";

    // all names
    // the attribution position of the uniform is its position in this array
    static constexpr const GLchar *uniform_variable_names[]{
        offset_x_uniform_variable_name,   offset_y_uniform_variable_name, z_mult_uniform_variable_name,
        model_uniform_variable_name,      view_uniform_variable_name,     projection_uniform_variable_name,
        tessellation_level_variable_name,
    };

    // all positions

    GLuint program_handle;
    std::vector<std::shared_ptr<Shader>> attached_shaders;
    std::unordered_map<const GLchar *, GLint> uniform_locations;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;
    std::shared_ptr<TessellationSettings> tessellation_settings;

    void set_uniform_1f(const GLchar *uniform_variable_name, GLfloat value);
    void set_uniform_1ui(const GLchar *uniform_variable_name, GLuint value);
    void set_uniform_matrix_4fv(const GLchar *uniform_variable_name, std::shared_ptr<glm::mat4> value);

public:
    ShaderProgram() = delete;
    // TODO: more ergonomic ways of constructing this class

    /**
     * prereq: must have opengl initialized before calling
     */
    ShaderProgram(std::initializer_list<std::shared_ptr<Shader>> shaders, std::shared_ptr<glm::mat4> model,
                  std::shared_ptr<glm::mat4> view, std::shared_ptr<glm::mat4> projection,
                  std::shared_ptr<FunctionParams> function_params,
                  std::shared_ptr<TessellationSettings> tessellation_settings);
    ~ShaderProgram();

    void use();
    void release();

    void set_initial_uniforms();
    void update_function_params();
    void update_model();
    void update_view();
    void update_projection();
    void update_tessellation_settings();
};
