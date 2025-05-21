#include "glad/glad.h" // have to load glad first
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <SDL_video.h>
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
#include <optional>
#include <sstream>
#include <string>

#include "create_array.hpp"
#include "glad/glad.h"
#include "mouse_loc.hpp"
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
using std::optional;
using std::ostream;
using std::ostream_iterator;
using std::size_t;
using std::string;
using std::string_view;
using std::stringstream;

constexpr Uint32 target_fps = 30;
constexpr Uint32 max_sleep_per_tick = 1000 / target_fps;
constexpr size_t window_h = 800;
constexpr size_t window_w = 1200;

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

    CPPTRACE_TRY {
        auto vertex_shader = make_shader("vertex.glsl", GL_VERTEX_SHADER);
        auto tsc_shader = make_shader("tsc.glsl", GL_TESS_CONTROL_SHADER);
        auto tes_shader = make_shader("tes.glsl", GL_TESS_EVALUATION_SHADER);
        auto fragment_shader = make_shader("fragment.glsl", GL_FRAGMENT_SHADER);

        ShaderProgram program = {vertex_shader, tsc_shader, tes_shader, fragment_shader};

        Vertices verts{create_array_t<GLfloat>(0.5, -0.5, 0.0, 0.5, 0.5, 0.0, -0.5, 0.5, 0.0, -0.5, -0.5, 0.0)};

        mat4 model = rotate(mat4(1.0f), radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
        //mat4 model = mat4(1.0f);
        const mat4 view = translate(mat4(1.0f), vec3(0.0f, 0.0f, -3.0f));
        const mat4 projection = perspective(radians(50.0f), (float)window_w / (float)window_h, 0.01f, 10.0f);

        program.use();
        program.set_offset_x(0.0);
        program.set_offset_y(0.0);
        program.set_offset_z(0.0);
        program.set_model(model);
        program.set_view(view);
        program.set_projection(projection);
        program.release();

        // allows panning the 3d function but does not move the model matrix
        // itself (panning happens in place)
        GLfloat offset_x = 0.0f;
        GLfloat offset_y = 0.0f;
        GLfloat offset_z = 0.0f;

        while (true) {
            bool modified_offset_x = false;
            bool modified_offset_y = false;
            bool modified_offset_z = false;
            bool rotation_modified = false;
            bool is_mouse_rotating_surface = false;
            optional<MouseLoc> start_click_loc;
            MouseLoc current_loc;

            auto start_ticks = SDL_GetTicks();
            verts.get_vao()->bind();
            program.use();

            SDL_Event evt;
            while (SDL_PollEvent(&evt)) {
                if (evt.type == SDL_QUIT) {
                    return 0;
                }
                else if (evt.type == SDL_KEYDOWN) {
                    if (evt.key.keysym.sym == SDLK_q) {
                        return 0;
                    }

                    if (!is_mouse_rotating_surface) {
                        if (evt.key.keysym.sym == SDLK_a) {
                            rotation_modified = true;
                            model = rotate(model, radians(-1.0f), vec3(1.0f, 0.0f, 0.0f));
                        }

                        else if (evt.key.keysym.sym == SDLK_d) {
                            rotation_modified = true;
                            model = rotate(model, radians(1.0f), vec3(1.0f, 0.0f, 0.0f));
                        }
                    }

                    // TODO: mouse events to make this more intuitive
                    if (evt.key.keysym.sym == SDLK_LEFT) {
                        if (evt.key.keysym.mod & KMOD_SHIFT) {
                            offset_y -= 0.1;
                            modified_offset_y = true;
                        }
                        else {
                            offset_x -= 0.1;
                            modified_offset_x = true;
                        }
                    }

                    if (evt.key.keysym.sym == SDLK_RIGHT) {
                        if (evt.key.keysym.mod & KMOD_SHIFT) {
                            offset_y += 0.1;
                            modified_offset_y = true;
                        }
                        else {
                            offset_x += 0.1;
                            modified_offset_x = true;
                        }
                    }

                    if (evt.key.keysym.sym == SDLK_UP) {
                        offset_z += 0.1;
                        modified_offset_z = true;
                    }

                    if (evt.key.keysym.sym == SDLK_DOWN) {
                        offset_z -= 0.1;
                        modified_offset_z = true;
                    }
                }
                else if (evt.type == SDL_MOUSEBUTTONDOWN) {
                    is_mouse_rotating_surface = true;
                    MouseLoc new_loc;
                    start_click_loc = new_loc;
                }
                else if (evt.type == SDL_MOUSEBUTTONUP) {
                    is_mouse_rotating_surface = false;
                    start_click_loc.reset();
                }

                if (is_mouse_rotating_surface && start_click_loc) {
                    current_loc.update_loc();

                    double dist = current_loc.distance(start_click_loc.value());
                    if (dist >= 5) { // arbitrary
                                     // TODO spin the model based off of how far the mouse
                                     // has been moved since clicking
                    }
                }
            }

            glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            glPatchParameteri(GL_PATCH_VERTICES, 4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_PATCHES, 0, 4);

            // debug the points only (need to disable tessellation)
            // glDrawElements(GL_POINTS, static_cast<GLsizei>(verts.get_vert_count()), GL_UNSIGNED_INT, nullptr);
            SDL_GL_SwapWindow(window);

            verts.get_vao()->unbind();
            program.release();

            auto end_ticks = SDL_GetTicks();
            auto sleep_for_ticks = end_ticks - start_ticks;

            if (max_sleep_per_tick > sleep_for_ticks) {
                SDL_Delay(max_sleep_per_tick - sleep_for_ticks);
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
