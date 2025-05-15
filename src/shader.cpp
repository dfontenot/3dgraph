#include "shader.hpp"
#include "exceptions.hpp"
#include "gl_inspect.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using std::cerr;
using std::endl;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::filesystem::current_path;

namespace {
// source: https://stackoverflow.com/a/2602060/854854
string read_file(const string &source_fn) {
    using std::ifstream;
    using std::istreambuf_iterator;

    ifstream file{source_fn};
    string str;

    file.seekg(0, std::ios::end);
    str.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    str.assign((istreambuf_iterator<char>{file}), istreambuf_iterator<char>{});

    return str;
}
} // namespace

Shader::Shader(const char *source_fn, GLenum shader_type)
    : shader_type(shader_type), shader_handle(glCreateShader(shader_type)) {
    auto shader_dir = current_path() / "shaders";

    auto shader_source = read_file(shader_dir / source_fn);
    auto shader_handle_data = shader_source.data();

    glShaderSource(shader_handle, number_of_sources, static_cast<GLchar **>(&shader_handle_data), source_lengths);

    glCompileShader(shader_handle);

    GLint compiled;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);

    GLsizei log_bytes_to_allocate;
    glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_bytes_to_allocate);
    if (log_bytes_to_allocate > 1) {
        auto compilation_err_str = make_unique<GLchar[]>(log_bytes_to_allocate);
        glGetShaderInfoLog(shader_handle, log_bytes_to_allocate, nullptr, compilation_err_str.get());

        if (compiled == GL_TRUE) {
            cerr << "encountered error when compiling " << shader_type_to_string(shader_type)
                 << " shader: " << compilation_err_str.get() << endl;
        }
        else {
            throw ShaderCompilationError(compilation_err_str.get(), shader_type);
        }
    }

    assert(compiled == GL_TRUE);
}

Shader::~Shader() {
    glDeleteShader(shader_handle);
}

shared_ptr<Shader> make_shader(const char *source_fn, GLenum shader_type) {
    using std::make_shared;
    return make_shared<Shader>(Shader(source_fn, shader_type));
}
