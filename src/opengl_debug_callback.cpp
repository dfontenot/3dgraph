#include "opengl_debug_callback.hpp"
#include "glad/glad.h"

#include <string>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace {
    using std::string;
    using std::unordered_set;

auto const opengl_debug = spdlog::stdout_color_mt("opengl_debug");

// manually filter out things to ignore
// 131185: https://stackoverflow.com/a/62249363
const unordered_set<GLuint> message_ids_to_ignore{131185};

// original source code: https://gist.github.com/liam-middlebrook/c52b069e4be2d87a6d2f
// public domain license
void APIENTRY gl_debug_msg_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                    const GLchar *msg, const void *data) {
    string _source;
    string _type;
    string _severity;

    if (message_ids_to_ignore.contains(id)) {
        return;
    }

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

    default:
        _source = "UNKNOWN";
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

    case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

    case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

    default:
        _type = "UNKNOWN";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;

    case GL_DEBUG_SEVERITY_LOW:
        _severity = "LOW";
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;

    default:
        _severity = "UNKNOWN";
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        opengl_debug->error("{0}: {1} of {2} severity, raised from {3}: {4}", id, _type, _severity, _source, msg);
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    case GL_DEBUG_TYPE_PORTABILITY:
        opengl_debug->warn("{0}: {1} of {2} severity, raised from {3}: {4}", id, _type, _severity, _source, msg);
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
    case GL_DEBUG_TYPE_OTHER:
        opengl_debug->info("{0}: {1} of {2} severity, raised from {3}: {4}", id, _type, _severity, _source, msg);
        break;

    case GL_DEBUG_TYPE_MARKER:
        opengl_debug->debug("{0}: {1} of {2} severity, raised from {3}: {4}", id, _type, _severity, _source, msg);
        break;
    }
}
} // namespace

void init_opengl_debug() {

#if OPENGL_DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#else
    glEnable(GL_DEBUG_OUTPUT);
#endif
    glDebugMessageCallback(::gl_debug_msg_callback, nullptr);
}
