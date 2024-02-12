#include "gl_inspect.hpp"
#include "exceptions.hpp"
#include "vertices.hpp"
#include "grid_points.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include "glad/glad.h"
#include <iostream>
#include <memory>
#include <range/v3/all.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <sstream>
#include <numeric>

using std::array;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::filesystem::current_path;
using std::get;
using std::make_unique;
using std::next;
using std::ostream;
using std::ostream_iterator;
using std::size_t;
using std::string;
using std::string_view;
using std::runtime_error;
using std::stringstream;
using ranges::views::iota;
using ranges::views::cartesian_product;

constexpr size_t window_h = 800;
constexpr size_t window_w = 1200;
constexpr int tesselation_amount = 10;

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

/**
* @ brief creates a lattice of points (no triangles)
* @ return a flat GLfloat array
*/
auto make_lattice() {
    using ranges::for_each;

    constexpr size_t total_size = tesselation_amount * tesselation_amount * 3; // 3 dims per vertex
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
        *point_location = 0.0f; // let shader compute height
        point_location = next(point_location);
    });

    return lattice;
}

class Shader {
    static constexpr GLsizei number_of_sources = 1;
    static constexpr GLint* source_lengths = 0; // can be set to 0 since source ends with a null terminator

    string source_fn;
    GLint compiled;
public:
    GLuint shader_handle;
    GLenum shader_type;

    Shader() = delete;
    Shader(const char* source_fn, GLenum shader_type) :
        shader_type(shader_type),
        compiled(GL_FALSE),
        source_fn(string(source_fn)) {}

    bool compile() {
        auto shader_dir = current_path() / "shaders";

        auto shader_source = read_file(shader_dir / source_fn);
        auto shader_handle_data = shader_source.data();

        shader_handle = glCreateShader(shader_type);
        glShaderSource(shader_handle,
                       number_of_sources,
                       static_cast<GLchar**>(&shader_handle_data),
                       source_lengths);

        glCompileShader(shader_handle);
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);
        if (! did_compile()) {
            GLsizei to_allocate;
            glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &to_allocate);
            auto compilation_err_str = make_unique<GLchar[]>(to_allocate);
            glGetShaderInfoLog(shader_handle, to_allocate, nullptr, compilation_err_str.get());
            throw ShaderCompilationError(compilation_err_str.get(), shader_type);
        }
        else {
            return true;
        }
    }

    /**
    * @brief has the shader been successfully compiled yet
    * @return bool
    */
    bool did_compile() const {
        return compiled == GL_TRUE;
    }
};


class ShaderProgramCompilationError : public WrappedOpenGLError {
public:
    ShaderProgramCompilationError(string& msg) : WrappedOpenGLError(msg) {}
    ShaderProgramCompilationError(char* msg) : WrappedOpenGLError(msg) {}
    ShaderProgramCompilationError(const char* msg) : WrappedOpenGLError(msg) {}
};

class ShaderProgram {
    static constexpr GLuint position_index = 0;
    static constexpr const GLchar* position_variable_name = "position"; // must line up w/ shader source code
    static constexpr const GLchar* offset_x_uniform_variable_name = "offset_x";
    static constexpr const GLchar* offset_y_uniform_variable_name = "offset_y";
    
    GLint linked;
public:
    GLuint program_handle;
    bool program_enabled;

    ShaderProgram() : linked(GL_FALSE), program_handle(glCreateProgram()), program_enabled(false) {}

    ShaderProgram& attach_shader(Shader& shader) {
        auto current_error = glGetError();

        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot link " + shader_type_to_string(shader.shader_type) + " shader due to existing error: " + gl_get_error_string(current_error));
        }

        glAttachShader(program_handle, shader.shader_handle);

        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("error occurred during " + shader_type_to_string(shader.shader_type) + " shader linkage: " + gl_get_error_string(current_error));
        }

        return *this;
    }

    ShaderProgram& link() {
        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot link shader program due to existing error: " + gl_get_error_string(current_error));
        }

        glBindAttribLocation(program_handle, position_index, position_variable_name);
        glLinkProgram(program_handle);
        glGetProgramiv(program_handle, GL_LINK_STATUS, &linked);

        if (! did_link()) {
            GLsizei to_allocate;
            glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &to_allocate);
            auto link_err_str = make_unique<GLchar[]>(to_allocate);
            glGetProgramInfoLog(program_handle, to_allocate, nullptr, link_err_str.get());
            throw ShaderProgramCompilationError(link_err_str.get());
        }

        // TODO: call glDetachShader() on each shader then call glDeleteShader() on each shader

        return *this;
    }

    ShaderProgram& use() {
        if (! did_link()) {
            throw WrappedOpenGLError("need to link the program first");
        }

        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot enable shader program due to existing error: " + gl_get_error_string(current_error));
        }

        glUseProgram(program_handle);
        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("failed to enable the program: " + gl_get_error_string(current_error));
        }

        program_enabled = true;

        // initialize uniform variables
        update_uniforms(0.0, 0.0);

        return *this;
    }

    void update_uniforms(float offset_x, GLfloat offset_y) {
        if (! program_enabled) {
            throw WrappedOpenGLError("need to enable the program first");
        }

        auto current_error = glGetError();
        if (current_error != GL_NO_ERROR) {
            throw WrappedOpenGLError("cannot set uniforms due to existing error: " + gl_get_error_string(current_error));
        }

        GLint offset_x_location = glGetUniformLocation(program_handle, offset_x_uniform_variable_name);
        if (offset_x_location < 0) {
            throw WrappedOpenGLError("unable to set offset_x");
        }

        glUniform1f(offset_x_location, offset_x);
        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("error setting x_offset uniform: " + gl_get_error_string(current_error));
        }

        GLint offset_y_location = glGetUniformLocation(program_handle, offset_y_uniform_variable_name);
        if (offset_y_location < 0) {
            throw WrappedOpenGLError("unable to set offset_y");
        }

        glUniform1f(offset_y_location, offset_y);
        if ((current_error = glGetError()) != GL_NO_ERROR) {
            throw WrappedOpenGLError("error setting y_offset uniform: " + gl_get_error_string(current_error));
        }
    }

    bool did_link() const {
        return linked == GL_TRUE;
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
    glEnable(GL_PROGRAM_POINT_SIZE);

    glViewport(0, 0, window_w, window_h);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    try {

        auto points = make_lattice();
        Vertices verts(points);
        GridPoints<points.size()> grid_points {};

        Shader vertex_shader("vertex.glsl", GL_VERTEX_SHADER);
        vertex_shader.compile();

        Shader fragment_shader("fragment.glsl", GL_FRAGMENT_SHADER);
        fragment_shader.compile();

        ShaderProgram program {};
        program.attach_shader(vertex_shader).attach_shader(fragment_shader).link().use();

        // allows panning the 3d function
        GLfloat offset_x;
        GLfloat offset_y;

        while (true) {

            verts.get_vao()->bind();
            grid_points.get_ibo()->bind();
            // glBindVertexArray(*verts.get_vao());
            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *grid_points.get_ibo());

            glDrawElements(GL_POINTS, static_cast<GLsizei>(verts.get_vert_count()), GL_UNSIGNED_INT, nullptr);
            //glBindBuffer(GL_ARRAY_BUFFER, verts.vbo);
            //glDrawArrays(GL_POINTS, 0, verts.num_verts);
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

                SDL_Delay(200);
                program.update_uniforms(offset_x, offset_y);
                glBindVertexArray(0); // TODO: clean up direct opengl call
            }
        }

        return 0;
    }
    catch (std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
