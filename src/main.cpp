#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include "glad/glad.h"
#include <iostream>
#include <range/v3/all.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>
#include <string_view>

using std::array;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::filesystem::current_path;
using std::get;
using std::next;
using std::ostream;
using std::ostream_iterator;
using std::size_t;
using std::string;
using std::string_view;
using ranges::views::iota;
using ranges::views::cartesian_product;
using ranges::for_each;

constexpr size_t window_h = 800;
constexpr size_t window_w = 1200;
constexpr int tesselation_amount = 20;

// source: https://stackoverflow.com/a/19152438/854854
template <class T, size_t N>
ostream& operator<<(ostream& o, const array<T, N>& arr) {
    copy(arr.cbegin(), arr.cend(), ostream_iterator<T>(o, " "));
    return o;
}

// source: https://stackoverflow.com/a/2602060/854854
string read_file(const string& source_fn) {
    using std::ifstream;
    using std::istreambuf_iterator;

    ifstream file { source_fn };
    string str;

    file.seekg(0, std::ios::end);
    str.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    str.assign((istreambuf_iterator<char> { file }), istreambuf_iterator<char>{});

    return str;
}

auto make_lattice() {
    constexpr size_t total_size = tesselation_amount * tesselation_amount * 2;
    constexpr GLfloat scaling = 1.0 / static_cast<GLfloat>(tesselation_amount);
    array<GLfloat, total_size> lattice;

    const auto tesselation = iota(0, tesselation_amount);
    const auto product = cartesian_product(tesselation, tesselation);
    auto point_location = lattice.begin();
    for_each(product, [&point_location](auto pt) {
        *point_location = get<0>(pt) * scaling;
        point_location = next(point_location);
        *point_location = get<1>(pt) * scaling;
        point_location = next(point_location);
    });

    return lattice;
}

class Vertices {
    static constexpr GLint points_per_vertex = 2;
    static constexpr GLuint vertex_attrib_location = 0;
    static constexpr GLboolean is_normalized = GL_FALSE;
    static constexpr GLsizei stride = 0;

public:
    GLuint vao;
    GLuint vbo;

    template <size_t N>
    void init(array<GLfloat, N> data) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, N * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_attrib_location,
                              points_per_vertex,
                              GL_FLOAT,
                              is_normalized,
                              stride,
                              0);
        glEnableVertexAttribArray(vertex_attrib_location);
    }
};

class Shader {
    static constexpr GLsizei number_of_sources = 1;
    static constexpr GLint* source_lengths = 0; // can be set to 0 since source ends with a null terminator

    string source_fn;
    string last_error_;
    int compiled;
public:
    GLuint shader_handle;
    GLenum shader_type;

    Shader() = delete;
    Shader(const char* source_fn, GLenum shader_type) :
        shader_type(shader_type),
        compiled(GL_FALSE),
        source_fn(string(source_fn)) {}

    void compile() {
        auto shader_dir = current_path() / "shaders";

        auto shader_source = read_file(shader_dir / source_fn);
        auto shader_handle_data = shader_source.data();
        last_error_.clear();

        shader_handle = glCreateShader(shader_type);
        glShaderSource(shader_handle,
                       number_of_sources,
                       static_cast<GLchar**>(&shader_handle_data),
                       source_lengths);

        glCompileShader(shader_handle);
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);
        if (! did_compile()) {
            int to_allocate;
            glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &to_allocate);
            last_error_.reserve(to_allocate);
            glGetShaderInfoLog(shader_handle, to_allocate, &to_allocate, last_error_.data());
        }
    }

    bool did_compile() const {
        return compiled == GL_TRUE;
    }

    string_view last_error() const {
        return string_view { last_error_ };
    }
};

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

    glViewport(0, 0, window_w, window_h);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

    Vertices verts {};
    verts.init(make_lattice());

    Shader vertex_shader("vertex.glsl", GL_VERTEX_SHADER);
    if (! vertex_shader.did_compile()) {
        cerr << "vertex shader compilation failed:" << endl << vertex_shader.last_error() << endl;
    }

    Shader fragment_shader("fragment.glsl", GL_FRAGMENT_SHADER);
    if (! fragment_shader.did_compile()) {
        cerr << "fragment shader compilation failed:" << endl << fragment_shader.last_error() << endl;
    }

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
