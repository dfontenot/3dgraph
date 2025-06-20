#include "gl_inspect.hpp"

#include "glad/glad.h"

#include <cassert>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

using std::make_optional;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::nullopt;

string shader_type_to_string(GLenum shader_type) {
    if (shader_type == GL_VERTEX_SHADER) {
        return string{"vertex"};
    }
    else if (shader_type == GL_TESS_CONTROL_SHADER) {
        return string{"tsc"};
    }
    else if (shader_type == GL_TESS_EVALUATION_SHADER) {
        return string{"tes"};
    }
    else if (shader_type == GL_GEOMETRY_SHADER) {
        return string{"geometry"};
    }
    else if (shader_type == GL_FRAGMENT_SHADER) {
        return string{"fragment"};
    }
    else {
        // TODO: support more as needed
        throw runtime_error("unknown shader type");
    }
}

string gl_get_error_string(GLenum err) {

    stringstream ss;
    ss << "0x" << std::hex << err << std::dec << ": ";
    switch (err) {
    case GL_NO_ERROR:
        ss << "no error";
        break;
    case GL_INVALID_ENUM:
        ss << "invalid enum";
        break;
    case GL_INVALID_VALUE:
        ss << "invalid value";
        break;
    case GL_INVALID_OPERATION:
        ss << "invalid operation";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        ss << "invalid framebuffer operation";
        break;
    case GL_OUT_OF_MEMORY:
        ss << "out of memory";
        break;
    default:
        ss << "unknown";
        break;
    }

    return ss.str();
}

string gl_get_error_string() {
    return gl_get_error_string(glGetError());
}

std::optional<GLuint> get_max_tessellation_level() {
#if OPENGL_ES
    return nullopt;
#else
    GLint max_level;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &max_level);

    assert(max_level > 0);
    return make_optional(static_cast<GLuint>(max_level));
#endif
}
