#pragma once

#include <array>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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

    /** all uniform names that appear in any shaders
     * the attribution position of the uniform is its position in this array
     */
    static constexpr std::array const uniform_variable_names{
        offset_x_uniform_variable_name,  offset_y_uniform_variable_name, z_mult_uniform_variable_name,
        model_uniform_variable_name,     view_uniform_variable_name,     projection_uniform_variable_name,
        tessellation_level_variable_name};

    GLuint program_handle;
    bool in_use;
    std::vector<std::shared_ptr<Shader>> attached_shaders;

    std::unordered_map<const GLchar *, GLint> uniform_locations;
    std::shared_ptr<glm::mat4> model;
    std::shared_ptr<glm::mat4> view;
    std::shared_ptr<glm::mat4> projection;
    std::shared_ptr<FunctionParams> function_params;
    std::shared_ptr<TessellationSettings> tessellation_settings;

    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> err;

    void set_uniform_1f(const GLchar *uniform_variable_name, GLfloat value);
    void set_uniform_1ui(const GLchar *uniform_variable_name, GLuint value);
    void set_uniform_matrix_4fv(const GLchar *uniform_variable_name, std::shared_ptr<glm::mat4> const &value);

    void link_shaders();

public:
    ShaderProgram() = delete;
    ShaderProgram(ShaderProgram const &) = delete; // TODO relax this
    ShaderProgram(ShaderProgram &&) = default;
    ShaderProgram &operator=(const ShaderProgram &rhs) = delete; // TODO relax this
    ShaderProgram &operator=(ShaderProgram &&rhs) noexcept = default;

    /**
     * prereq: must have opengl initialized before calling
     */
    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::shared_ptr<Shader>>
    ShaderProgram(R &&shaders, std::shared_ptr<glm::mat4> const &model, std::shared_ptr<glm::mat4> const &view,
                  std::shared_ptr<glm::mat4> const &projection, std::shared_ptr<FunctionParams> const &function_params,
                  std::shared_ptr<TessellationSettings> const &tessellation_settings)
        : program_handle(glCreateProgram()), in_use(false), attached_shaders(std::forward<R>(shaders)), model(model),
          view(view), projection(projection), function_params(function_params),
          tessellation_settings(tessellation_settings), logger(spdlog::stderr_color_mt("shader_program")),
          err(spdlog::stderr_color_mt("shader_program_err")) {
        link_shaders();
    }

    ~ShaderProgram();

    [[nodiscard]] bool is_in_use() const;
    void use();
    void release();

    void set_initial_uniforms();
    void update_function_params();
    void update_model();
    void update_view();
    void update_projection();
    void update_tessellation_settings();
};
