#include "shader.hpp"
#include "exceptions.hpp"
#include "gl_inspect.hpp"

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using std::format;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::filesystem::current_path;
using std::filesystem::path;

#ifdef OPENGL_ES
static constexpr const bool is_opengl_es = true;
#else
static constexpr const bool is_opengl_es = false;
#endif

namespace {

// source: https://stackoverflow.com/a/2602060/854854
/**
 * precondition: the file must exist and be readable
 */
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

void do_shader_compilation(GLuint shader_handle, GLenum shader_type, GLsizei number_of_sources,
                           const GLint *source_lengths, const path &source_path,
                           const shared_ptr<spdlog::logger> &logger, const shared_ptr<spdlog::logger> &err) {
    // preconditions

    // TODO: relax this restriction with ES 3.2 or GL_EXT_tessellation_shader
    assert(shader_handle != 0);
    if (is_opengl_es && (shader_type == GL_TESS_CONTROL_SHADER || shader_type == GL_TESS_EVALUATION_SHADER)) {
        throw WrappedOpenGLError(
            format("cannot construct shader of type {} in OpenGL ES", shader_type_to_string(shader_type)));
    }

    auto current_error = glGetError();
    if (current_error != GL_NO_ERROR) {
        throw WrappedOpenGLError(format("precondition failed in shader ctor: {}", gl_get_error_string(current_error)));
    }

    logger->debug("reading shader file {}", source_path.string());
    if (!std::filesystem::exists(source_path)) {
        throw ShaderError(format("no such file {}", source_path.string()), shader_type);
    }
    else if (std::filesystem::is_directory(source_path)) {
        throw ShaderError(format("{} is a directory", source_path.string()), shader_type);
    }

    string shader_source = ::read_file(source_path);
    auto shader_handle_data = shader_source.data();
    glShaderSource(shader_handle, number_of_sources, static_cast<GLchar **>(&shader_handle_data), source_lengths);
    glCompileShader(shader_handle);

    GLint compiled = -1;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);

    GLsizei log_bytes_to_allocate = -1;
    glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_bytes_to_allocate);
    if (log_bytes_to_allocate > 1) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
        auto compilation_err_str = make_unique<GLchar[]>(log_bytes_to_allocate);
        glGetShaderInfoLog(shader_handle, log_bytes_to_allocate, nullptr, compilation_err_str.get());

        if (compiled == GL_TRUE) {
            err->warn("encountered error when compiling {0} shader: {1}", shader_type_to_string(shader_type),
                      compilation_err_str.get());
        }
        else {
            throw ShaderCompilationError(compilation_err_str.get(), shader_type);
        }
    }

    assert(compiled == GL_TRUE);
}
} // namespace

Shader::Shader(const path &source_path, GLenum shader_type)
    : shader_type(shader_type), shader_handle(glCreateShader(shader_type)),
      logger(spdlog::stderr_color_mt(format("shader {}", shader_type_to_string(shader_type)))),
      err(spdlog::stderr_color_mt(format("shader_err {}", shader_type_to_string(shader_type)))) {
    if (source_path.is_absolute()) {
        throw ShaderError("must specify path relative to shaders directory", shader_type);
    }
    else {
        // TODO: a lot of sanitization here
        ::do_shader_compilation(shader_handle, shader_type, number_of_sources, source_lengths,
                                current_path() / source_path, logger, err);
    }
}

Shader::Shader(const string &source_fn, GLenum shader_type) : Shader(path{"shaders"} / path{source_fn}, shader_type) {
}

Shader::Shader(const char *source_fn, GLenum shader_type) : Shader(path{"shaders"} / path{source_fn}, shader_type) {
}

Shader::~Shader() {
    glDeleteShader(shader_handle);
}

GLenum Shader::get_shader_type() const noexcept {
    return shader_type;
}
