#include "gl/debug.h"

#include "core/logger.h"

#include <format>
#include <string_view>

#include <debugbreak.h>

#ifndef NDEBUG
    #ifdef _WIN32
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #define WIN32_LEAN_AND_MEAN
        #include <Windows.h>
    #elif defined(__linux__)
        #include <fstream>
        #include <string>
    #endif
#endif

// source for linux technique: https://stackoverflow.com/a/24969863
bool is_debugger_present() {
#ifdef NDEBUG
    return false;
#else
    #ifdef _WIN32
    return IsDebuggerPresent();
    #elif defined(__linux__)
    // assumes a debugger is not attached mid-run
    static const bool is_present = [] {
        std::ifstream status_file{"/proc/self/status"};
        std::string line{};
        while (std::getline(status_file, line)) {
            if (line.starts_with("TracerPid:")) {
                try {
                    return std::stoi(line.substr(11)) != 0;
                } catch (...) {
                    return false;
                }
            }
        }

        return false;
    }();

    return is_present;
    #else
    return false;
    #endif
#endif
}

namespace {

constexpr std::string_view gl_debug_src_str(GLenum src) {
    // clang-format off
    switch (src) {
        case GL_DEBUG_SOURCE_API:             return "GL API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return "3rd Party";
        case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
        case GL_DEBUG_SOURCE_OTHER:           return "Other";
        default:                              return "UNKNOWN (NOTE: if you're seeing this, gl_debug_src_str is incomplete)";
    }
    // clang-format on
}

constexpr std::string_view gl_debug_type_str(GLenum type) {
    // clang-format off
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
        case GL_DEBUG_TYPE_MARKER:              return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
        case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
        case GL_DEBUG_TYPE_OTHER:               return "Other";
        default:                                return "UNKNOWN (NOTE: if you're seeing this, gl_debug_type_str is incomplete)";
    }
    // clang-format on
}

constexpr std::string_view gl_debug_severity_str(GLenum severity) {
    // clang-format off
    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW:          return "Low";
        case GL_DEBUG_SEVERITY_MEDIUM:       return "Medium";
        case GL_DEBUG_SEVERITY_HIGH:         return "High";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        default:                             return "UNKNOWN (NOTE: if you're seeing this, gl_debug_severity_str is incomplete)";
    }
    // clang-format on
}

} // namespace

void GLAPIENTRY ogl_debug_msg_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                       [[maybe_unused]] GLsizei length, const GLchar* msg,
                                       const void* user_param) {
    assert(user_param != nullptr &&
           "opengl debug config was not provided when initializing the debug callback");
    const auto* config = reinterpret_cast<const OpenGLDebugCallbackConfig*>(user_param);

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && !config->show_notifications)
        return;

    LoggerCategory log_level =
        severity != GL_DEBUG_SEVERITY_NOTIFICATION ? LoggerCategory::Error : LoggerCategory::Info;

    // don't report src info for opengl errors (it'll just point to the log call below)
    bool has_loc = Logger::instance().loc_filter.has(log_level);
    Logger::instance().loc_filter.disable(log_level);

    Logger::instance().log(log_level, "OpenGL",
                           std::format("debug message received:\n"
                                       "Severity: {}\n"
                                       "Source: {}\n"
                                       "Type: {}\n"
                                       "ID: {}\n"
                                       "Message:\n {}",
                                       gl_debug_severity_str(severity), gl_debug_src_str(src),
                                       gl_debug_type_str(type), id, msg));

    if (has_loc)
        Logger::instance().loc_filter.enable(log_level);

    if (is_debugger_present() && severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        debug_break();
}

#undef WIN32_LEAN_AND_MEAN
