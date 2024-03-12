#include "gl_inspect.hpp"
#include "exceptions.hpp"
#include "vertices.hpp"
#include "shader.hpp"
#include "shader_program.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include "glad/glad.h"
#include <iostream>
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <sstream>
#include <numeric>
#include <vector>
#include <iterator>

using std::array;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::filesystem::current_path;
using std::make_unique;
using std::ostream;
using std::ostream_iterator;
using std::size_t;
using std::string;
using std::string_view;
using std::shared_ptr;
using std::runtime_error;
using std::stringstream;
using std::vector;

constexpr Uint32 target_fps = 30;
constexpr Uint32 max_sleep_per_tick = 1000 / target_fps;
constexpr size_t window_h = 800;
constexpr size_t window_w = 1200;

// source: https://stackoverflow.com/a/19152438/854854
template <class T, size_t N>
ostream& operator<<(ostream& o, const array<T, N>& arr) {
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

    auto window = SDL_CreateWindow("opengl render test",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   window_w,
                                   window_h,
                                   SDL_WINDOW_OPENGL);

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

    SDL_GL_SetSwapInterval(1); // vsync
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glViewport(0, 0, window_w, window_h);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    try {

        auto points = make_lattice();
        Vertices verts(points);
        GridPoints<points.size()> grid_points {};
        verts.unbind();

        auto vertex_shader = make_shader("vertex.glsl", GL_VERTEX_SHADER);

        auto fragment_shader = make_shader("fragment.glsl", GL_FRAGMENT_SHADER);

        ShaderProgram program = { vertex_shader, fragment_shader };
        //program.attach_shader(vertex_shader).attach_shader(fragment_shader).link().use();

        // allows panning the 3d function
        GLfloat offset_x;
        GLfloat offset_y;

        while (true) {

            auto start_ticks = SDL_GetTicks();
            verts.get_vao()->bind();

            glDrawElements(GL_POINTS, static_cast<GLsizei>(verts.get_vert_count()), GL_UNSIGNED_INT, nullptr);
            SDL_GL_SwapWindow(window);

            SDL_Event evt;
            while (SDL_PollEvent(&evt)) {

                if (evt.type == SDL_QUIT) {
                    return 0;
                }

                if (evt.type == SDL_KEYDOWN) {
                    if (evt.key.keysym.sym == SDLK_q) {
                        return 0;
                    }

                    if (evt.key.keysym.sym == SDLK_LEFT) {
                        offset_x -= 0.1;
                    }

                    if (evt.key.keysym.sym == SDLK_RIGHT) {
                        offset_x += 0.1;
                    }

                    if (evt.key.keysym.sym == SDLK_UP) {
                        offset_y += 0.1;
                    }

                    if (evt.key.keysym.sym == SDLK_DOWN) {
                        offset_y -= 0.1;
                    }
                }
            }

            program.update_uniforms(offset_x, offset_y);
            verts.get_vao()->unbind();

            auto end_ticks = SDL_GetTicks();
            auto sleep_for_ticks = end_ticks - start_ticks;

            if (max_sleep_per_tick > sleep_for_ticks) {
                SDL_Delay(max_sleep_per_tick - sleep_for_ticks);
            }
        }

        return 0;
    }
    catch (std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
