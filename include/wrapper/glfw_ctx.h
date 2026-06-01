#pragma once

#include <stdexcept>

#include "logger.h"
#include "vendor.h"

#include <format>
#include <iostream>
#include <string_view>

namespace detail {

struct GlfwErrorInfo {
    std::string_view name;
    std::string_view general_description;
};

// source: https://www.glfw.org/docs/3.4/group__errors.html
constexpr GlfwErrorInfo get_glfw_error_info(int code) {
    switch (code) {
        case GLFW_NO_ERROR:
            return {"NO_ERROR", "No error has occurred (NOTE: if you're seeing this in an "
                                "error message, something went wrong)"};

        case GLFW_NOT_INITIALIZED:
            return {"NOT_INITIALIZED", "GLFW has not been initialized"};

        case GLFW_NO_CURRENT_CONTEXT:
            return {"NO_CURRENT_CONTEXT", "No context is current for this thread"};

        case GLFW_INVALID_ENUM:
            return {"INVALID_ENUM",
                    "One of the arguments to the function was an invalid enum value"};

        case GLFW_INVALID_VALUE:
            return {"INVALID_VALUE", "One of the arguments to the function was an invalid value"};

        case GLFW_OUT_OF_MEMORY:
            return {"OUT_OF_MEMORY", "A memory allocation failed"};

        case GLFW_API_UNAVAILABLE:
            return {"API_UNAVAILABLE",
                    "GLFW could not find support for the requested API on the system"};

        case GLFW_VERSION_UNAVAILABLE:
            return {"VERSION_UNAVAILABLE",
                    "The requested OpenGL or OpenGL ES version is not available"};

        case GLFW_PLATFORM_ERROR:
            return {"PLATFORM_ERROR", "A platform-specific error occurred that does not match any "
                                      "of the more specific categories"};

        case GLFW_FORMAT_UNAVAILABLE:
            return {"FORMAT_UNAVAILABLE", "The requested format is not supported or available"};

        case GLFW_NO_WINDOW_CONTEXT:
            return {"NO_WINDOW_CONTEXT",
                    "The specified window does not have an OpenGL or OpenGL ES context"};

        case GLFW_CURSOR_UNAVAILABLE:
            return {"CURSOR_UNAVAILABLE", "The specified cursor shape is not available"};

        case GLFW_FEATURE_UNAVAILABLE:
            return {"FEATURE_UNAVAILABLE", "The requested feature is not provided by the platform"};

        case GLFW_FEATURE_UNIMPLEMENTED:
            return {"FEATURE_UNIMPLEMENTED",
                    "The requested feature is not implemented for the platform"};

        case GLFW_PLATFORM_UNAVAILABLE:
            return {"PLATFORM_UNAVAILABLE",
                    "Platform unavailable or no matching platform was found"};

        default:
            assert(false && "unhandled case in get_glfw_error_info");
            return {"UNKNOWN_ERROR", "Unknown GLFW error type (NOTE: if you're seeing this, "
                                     "get_glfw_error_info is incomplete)"};
    }
}

} // namespace detail

class GlfwCtx {
    template <bool InstanceInitialized>
    static void error_callback(int code, const char* description) {
        auto err_info = detail::get_glfw_error_info(code);

        Logger::instance().error("GLFW",
                                 std::format("An unexpected GLFW error occurred!\n"
                                             "Error code: {} ({})\n"
                                             "General description: {}\n"
                                             "Specific description: {}",
                                             err_info.name, code, err_info.general_description,
                                             description ? description : "not provided by GLFW"));

        // avoid referencing the current instance if error is reported during instance construction
        if constexpr (InstanceInitialized) {
            if (GlfwCtx::instance().errors_as_fatal)
                GlfwCtx::instance()._terminate_next_frame = true;
        }
    }

    // used for propagating errors to the main loop
    // throwing in error_callback would prevent proper logging of multiple, same-operation errors
    bool _terminate_next_frame = false;

public:
    // NOTE: errors_as_fatal is only respected on first call to instance()
    // set the instance's member to modify after creation
    static GlfwCtx& instance(bool errors_as_fatal = false) {
        static GlfwCtx instance{errors_as_fatal};

        return instance;
    }

    // can be modified/toggled later to achieve error critical sections of code
    bool errors_as_fatal;

    bool terminate_next_frame() const { return _terminate_next_frame; }

private:
    explicit GlfwCtx(bool errors_as_fatal)
        : errors_as_fatal{errors_as_fatal} {
        glfwSetErrorCallback(error_callback<false>);

        if (glfwInit() != GLFW_TRUE)
            throw std::runtime_error("Failed to initialize GLFW!");

        glfwSetErrorCallback(error_callback<true>);
    }

    ~GlfwCtx() { glfwTerminate(); }
};
