#include "glad/glad.h" // have to load glad first

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>

#include "spdlog/cfg/env.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cpptrace/from_current.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "consts.hpp"
#include "es/cpu_tessellation.hpp"
#include "es/grid_points.hpp"
#include "event_loop.hpp"
#include "function_params.hpp"
#include "grid.hpp"
#include "max_deque.hpp"
#include "opengl_debug_callback.hpp"
#include "shader.hpp"
#include "shader_program.hpp"
#include "tessellation_settings.hpp"
#include "vertices.hpp"

using glm::mat4;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::translate;
using glm::vec3;

using std::array;
using std::getenv;
using std::initializer_list;
using std::make_shared;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::stringstream;
using std::to_array;
using std::vector;
using std::filesystem::path;

static constexpr const GLint default_tessellation_level = 9;

#ifdef OPENGL_ES
static constexpr const bool is_opengl_es = true;
#else
static constexpr const bool is_opengl_es = false;
#endif

#if OPENGL_DEBUG
static constexpr const bool has_opengl_debug = true;
#else
static constexpr const bool has_opengl_debug = false;
#endif

void set_log_level() {
    const auto spdlog_env_var = getenv("SPDLOG_LEVEL");
    const auto log_level_env_var = getenv("LOG_LEVEL");

    if (spdlog_env_var == nullptr) {
        if (log_level_env_var == nullptr) {
            // fallback log level
#ifdef DEBUG_BUILD
#if DEBUG_BUILD == 1
            spdlog::set_level(spdlog::level::debug);
#else
            spdlog::set_level(spdlog::level::warn);
#endif
#else
            spdlog::set_level(spdlog::level::info);
#endif
        }
        else {
            spdlog::cfg::load_env_levels("LOG_LEVEL");
        }
    }
    else {
        spdlog::cfg::load_env_levels();
    }
}

int main(int argc, char *argv[]) {

    auto const stdout = spdlog::stdout_color_mt("main");
    auto const stderr = spdlog::stderr_color_mt("main_err");

    set_log_level();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        stderr->error("sdl init failed: {}", SDL_GetError());
        return 1;
    }

    if (std::atexit(SDL_Quit) != 0) {
        stderr->error("failed to register SDL_Quit exit handler");
        return 1;
    }

    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if constexpr (is_opengl_es) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    }
    else {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }

    auto const window = SDL_CreateWindow("opengl render test", window_w, window_h, SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        stderr->error("could not create window: {}", SDL_GetError());
        return 1;
    }

    auto const context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        stderr->error("could not create opengl context: {}", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, context);
    const int sdl_version = SDL_GetVersion();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    stdout->info("vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    stdout->info("renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    stdout->info("version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    stdout->info("shading language version: {}",
                 reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    stdout->info("SDL version: {0}.{1}.{2} (linked {3}.{4}.{5})", SDL_VERSIONNUM_MAJOR(SDL_VERSION),
                 SDL_VERSIONNUM_MINOR(SDL_VERSION), SDL_VERSIONNUM_MICRO(SDL_VERSION),
                 SDL_VERSIONNUM_MAJOR(sdl_version), SDL_VERSIONNUM_MINOR(sdl_version),
                 SDL_VERSIONNUM_MICRO(sdl_version));
#if defined(__clang__)
    stdout->info("compiler: clang {}.{}.{}", __clang_major__, __clang_minor__, __clang_patchlevel__);
#else
    stdout->info("compiler: gcc {}.{}.{}", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

    GLint major = -1;
    GLint minor = -1;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if ((is_opengl_es && major < 3) || (!is_opengl_es && (major < 4 || minor < 1))) {
        stderr->error("invalid opengl version");
        return 1;
    }

    if constexpr (has_opengl_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        init_opengl_debug();
    }

    SDL_GL_SetSwapInterval(1); // vsync
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glEnable(GL_PROGRAM_POINT_SIZE);

    glViewport(0, 0, window_w, window_h);

    if (!is_opengl_es) {
        glEnable(GL_LINE_SMOOTH);
    }
    glLineWidth(1.0f);

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        stderr->error("OpenGL setup failed");
        return 1;
    }

    CPPTRACE_TRY {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        auto model = make_shared<mat4>(rotate(mat4(1.0f), radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)));
        auto view = make_shared<mat4>(translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f)));
        auto projection =
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
            make_shared<mat4>(perspective(radians(50.0f), (float)window_w / (float)window_h, 0.01f, 10.00f));
        auto function_params = make_shared<FunctionParams>();
        auto tessellation_settings = make_shared<TessellationSettings>();

        // TODO: clean this up
        vector<shared_ptr<Shader>> the_shaders;
        if constexpr (is_opengl_es) {
            const path es_shader_base_path = "shaders/es";
            the_shaders.push_back(make_shared<Shader>(es_shader_base_path / "vertex.glsl", GL_VERTEX_SHADER));
            the_shaders.push_back(make_shared<Shader>(es_shader_base_path / "fragment.glsl", GL_FRAGMENT_SHADER));
        }
        else {
            the_shaders.push_back(make_shared<Shader>("vertex.glsl", GL_VERTEX_SHADER));
            the_shaders.push_back(make_shared<Shader>("tsc.glsl", GL_TESS_CONTROL_SHADER));
            the_shaders.push_back(make_shared<Shader>("tes.glsl", GL_TESS_EVALUATION_SHADER));
            the_shaders.push_back(make_shared<Shader>("fragment.glsl", GL_FRAGMENT_SHADER));
        }

        auto const program = make_shared<ShaderProgram>(std::move(the_shaders), model, view, projection,
                                                        function_params, tessellation_settings);

        // TODO: new abstraction to handle VAO only for opengl 4.1 and VAO + IBO for opengl ES
#ifdef OPENGL_ES
        GridPoints verts{default_tessellation_level};
#else
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        Vertices verts{to_array<GLfloat>({0.5, -0.5, 0.0, 0.5, 0.5, 0.0, -0.5, 0.5, 0.0, -0.5, -0.5, 0.0}), (size_t)3};
#endif

        Grid grid{std::move(verts), program};

        program->use();
        program->set_initial_uniforms();
        program->release();

        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        MaxDeque<uint64_t> render_timings(10);
        EventLoop event_loop{model, view, projection, function_params, tessellation_settings};
        while (true) {
            auto const tick_result = event_loop.process_frame(render_timings.get_avg());
            if (tick_result.should_exit()) {
                return 0;
            }

            if (tick_result.frame_skip()) {
                continue;
            }

            if (tick_result.any_uniforms_modified()) {
                program->use();

                if (tick_result.function_params_modified()) {
                    program->update_function_params();
                }

                if (tick_result.view_modified()) {
                    program->update_view();
                }

                if (tick_result.model_modified()) {
                    program->update_model();
                }

                if (tick_result.tessellation_settings_modified()) {
                    program->update_tessellation_settings();
                }

                program->release();
            }

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            auto const start_render_tick = SDL_GetTicksNS();
            grid.render(tick_result);
            program->release();

            SDL_GL_SwapWindow(window);

            render_timings.add(SDL_GetTicksNS() - start_render_tick);

            if (max_sleep_ms_per_tick > tick_result.elapsed_ticks_ms) {
                SDL_Delay(max_sleep_ms_per_tick - tick_result.elapsed_ticks_ms);
            }
        }

        return 0;
    }
    CPPTRACE_CATCH(std::exception & e) {
        stderr->info(e.what());
        cpptrace::from_current_exception().print();
        return 1;
    }
}
