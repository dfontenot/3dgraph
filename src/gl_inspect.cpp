#include "gl_inspect.hpp"

#include "glad/glad.h"

#include <stdexcept>
#include <string>
#include <sstream>

using std::runtime_error;
using std::string;
using std::stringstream;

string shader_type_to_string(GLenum shader_type) {
    if (shader_type == GL_VERTEX_SHADER) {
        return string { "vertex" };
    }
    else if (shader_type == GL_FRAGMENT_SHADER) {
        return string { "fragment" };
    }
    else {
        // TODO: support more as needed
        throw runtime_error("unknown shader type");
    }
}

string gl_get_error_string(GLenum err) {

    stringstream ss;
    ss << "0x" << std::hex << err << std::dec << ": ";
    switch(err) {
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
        case GL_STACK_UNDERFLOW:
            ss << "stack underflow";
            break;
        case GL_STACK_OVERFLOW:
            ss << "stack overflow";
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
