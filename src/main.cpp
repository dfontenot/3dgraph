#include <array>
#include <iostream>
#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define WINDOW_H 800
#define WINDOW_W 1200
#define TESSELATION_AMOUNT 20

using std::cerr;
using std::cout;
using std::endl;
using std::array;

// auto make_lattice() {
//     constexpr std::size_t total_size = TESSELATION_AMOUNT * TESSELATION_AMOUNT * 2;
//     array<GLfloat, total_size> lattice;
//
//     const auto tesselation = std::views::iota(0, TESSELATION_AMOUNT);
//     const auto product = std::views::cartesian_product(tesselation, tesselation);
// }

int main(int argc, char *argv[]) {

    atexit(SDL_Quit);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        cerr << "sdl init failed: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    auto window = SDL_CreateWindow("opengl render test",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   WINDOW_W,
                                   WINDOW_H,
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

    SDL_GL_SetSwapInterval(1); // vsync
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

    while (true) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {

            if (evt.type == SDL_QUIT) {
                return 0;
            }

            if (evt.type == SDL_KEYDOWN) {
                if (evt.key.keysym.sym == SDLK_q) {
                    return 0;
                }
            }
        }
    }

    return 0;
}
