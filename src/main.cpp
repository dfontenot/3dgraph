#include "event_loop.hpp"
#include "function_params.hpp"
#include "consts.hpp"
#include "glad/glad.h" // have to load glad first
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <algorithm>
#include <array>
#include <cpptrace/from_current.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include "create_array.hpp"
#include "shader.hpp"
#include "shader_program.hpp"
#include "vertices.hpp"

using glm::mat4;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::translate;
using glm::vec3;

using std::array;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::make_shared;
using std::ostream;
using std::ostream_iterator;
using std::size_t;
using std::string;
using std::stringstream;

// source: https://stackoverflow.com/a/19152438/854854
template <class T, size_t N> ostream &operator<<(ostream &o, const array<T, N> &arr) {
    copy(arr.cbegin(), arr.cend(), ostream_iterator<T>(o, " "));
    return o;
}

int main(int argc, char *argv[]) {
    atexit(SDL_Quit);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        cerr << "sdl init failed: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    auto window = SDL_CreateWindow("opengl render test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w,
                                   window_h, SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        cerr << "could not create window: " << SDL_GetError() << endl;
        return 1;
    }

    auto context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        cerr << "could not create opengl context: " << SDL_GetError() << endl;
        return 1;
    }

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    cout << "vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "version: " << glGetString(GL_VERSION) << endl;
    cout << "shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || minor < 1) {
        cerr << "invalid opengl version" << endl;
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // vsync
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glEnable(GL_PROGRAM_POINT_SIZE);

    glViewport(0, 0, window_w, window_h);

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.5);

    CPPTRACE_TRY {
        auto model = make_shared<mat4>(rotate(mat4(1.0f), radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)));
        auto view = make_shared<mat4>(translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f)));
        auto projection =
            make_shared<mat4>(perspective(radians(50.0f), (float)window_w / (float)window_h, 0.01f, 10.00f));
        auto function_params = make_shared<FunctionParams>();

        auto vertex_shader = make_shader("vertex.glsl", GL_VERTEX_SHADER);
        auto tsc_shader = make_shader("tsc.glsl", GL_TESS_CONTROL_SHADER);
        auto tes_shader = make_shader("tes.glsl", GL_TESS_EVALUATION_SHADER);
        auto fragment_shader = make_shader("fragment.glsl", GL_FRAGMENT_SHADER);

        ShaderProgram program{{vertex_shader, tsc_shader, tes_shader, fragment_shader}, model, view, projection, function_params};

        Vertices verts{create_array_t<GLfloat>(0.5, -0.5, 0.0, 0.5, 0.5, 0.0, -0.5, 0.5, 0.0, -0.5, -0.5, 0.0)};

        program.use();
        program.update_function_params();
        program.update_model();
        program.update_view();
        program.update_projection();
        program.release();

        EventLoop event_loop { model, view, projection, function_params };
        while (true) {

            verts.get_vao()->bind();
            program.use();

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glPatchParameteri(GL_PATCH_VERTICES, 4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_PATCHES, 0, 4);

            SDL_GL_SwapWindow(window);

            auto tick_result = event_loop.tick();
            if (tick_result.should_exit) {
                return 0;
            }

            if (event_loop.function_params_modified()) {
                program.update_function_params();
            }

            if (event_loop.view_modified()) {
                program.update_view();
            }

            if (event_loop.model_modified()) {
                program.update_model();
            }

            verts.get_vao()->unbind();
            program.release();

            if (max_sleep_per_tick > tick_result.elapsed_ticks) {
                SDL_Delay(max_sleep_per_tick - tick_result.elapsed_ticks);
            }
        }

        return 0;
    }
    CPPTRACE_CATCH(std::exception & e) {
        cerr << e.what() << endl;
        cpptrace::from_current_exception().print();
        return 1;
    }
}
