#include "consts.hpp"
#include "event_loop.hpp"
#include "function_params.hpp"
#include "glad/glad.h" // have to load glad first
#include "opengl_debug_callback.hpp"
#include "tessellation_settings.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cpptrace/from_current.hpp>
#include <cstdint>
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

#include "create_array.hpp"
#include "max_deque.hpp"
#include "shader.hpp"
#include "shader_program.hpp"
#include "vertices.hpp"

using glm::mat4;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::translate;
using glm::vec3;

using std::make_shared;
using std::size_t;
using std::string;
using std::stringstream;

static constexpr GLint default_tessellation_level = 9;

int main(int argc, char *argv[]) {
    atexit(SDL_Quit);

    spdlog::set_level(spdlog::level::debug);
    auto const stdout = spdlog::stdout_color_mt("main");
    auto const stderr = spdlog::stderr_color_mt("main_err");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        stderr->error("sdl init failed: {}", SDL_GetError());
        return 1;
    }

    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#ifdef OPENGL_ES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

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

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    stdout->info("vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    stdout->info("renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    stdout->info("version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    stdout->info("shading language version: {}",
                 reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || minor < 1) {
        stderr->error("invalid opengl version");
        return 1;
    }

#if OPENGL_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    init_opengl_debug();
#endif

    SDL_GL_SetSwapInterval(1); // vsync
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glEnable(GL_PROGRAM_POINT_SIZE);

    glViewport(0, 0, window_w, window_h);

    glEnable(GL_LINE_SMOOTH);

#if __APPLE__
    // macOS only supports a line width of 1.0
    // see: https://www.reddit.com/r/opengl/comments/at1az3/comment/egy4keo
    glLineWidth(1.0f);
#else
    glLineWidth(1.5f);
#endif

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        stderr->error("OpenGL setup failed");
        return 1;
    }

    CPPTRACE_TRY {
        auto model = make_shared<mat4>(rotate(mat4(1.0f), radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)));
        auto view = make_shared<mat4>(translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f)));
        auto projection =
            make_shared<mat4>(perspective(radians(50.0f), (float)window_w / (float)window_h, 0.01f, 10.00f));
        auto function_params = make_shared<FunctionParams>();
        auto tessellation_settings = make_shared<TessellationSettings>();

        auto vertex_shader = make_shader("vertex.glsl", GL_VERTEX_SHADER);
        auto tsc_shader = make_shader("tsc.glsl", GL_TESS_CONTROL_SHADER);
        auto tes_shader = make_shader("tes.glsl", GL_TESS_EVALUATION_SHADER);
        auto fragment_shader = make_shader("fragment.glsl", GL_FRAGMENT_SHADER);

        ShaderProgram program{{vertex_shader, tsc_shader, tes_shader, fragment_shader},
                              model,
                              view,
                              projection,
                              function_params,
                              tessellation_settings};

        Vertices verts{create_array_t<GLfloat>(0.5, -0.5, 0.0, 0.5, 0.5, 0.0, -0.5, 0.5, 0.0, -0.5, -0.5, 0.0)};

        program.use();
        program.set_initial_uniforms();
        program.release();

        MaxDeque<uint64_t> render_timings(10);
        EventLoop event_loop{model, view, projection, function_params, tessellation_settings};
        while (true) {
            auto const tick_result = event_loop.process_frame(render_timings.get_avg());
            if (tick_result.should_exit) {
                return 0;
            }

            if (tick_result.frame_skip) {
                continue;
            }

            auto const start_render_tick = SDL_GetTicksNS();
            verts.get_vao()->bind();
            program.use();

            if (event_loop.function_params_modified()) {
                program.update_function_params();
            }

            if (event_loop.view_modified()) {
                program.update_view();
            }

            if (event_loop.model_modified()) {
                program.update_model();
            }

            if (event_loop.tessellation_settings_modified()) {
                program.update_tessellation_settings();
            }

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glPatchParameteri(GL_PATCH_VERTICES, 4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_PATCHES, 0, 4);

            verts.get_vao()->unbind();
            program.release();

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
