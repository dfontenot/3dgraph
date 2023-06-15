#include <iostream>
#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define WINDOW_H 600
#define WINDOW_W 800

using std::cerr;
using std::cout;
using std::endl;

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

    return 0;
}
