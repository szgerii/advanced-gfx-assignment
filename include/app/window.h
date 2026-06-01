#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "vendor.h"

#include "logger.h"
#include "validate.h"

enum class WindowMode : uint8_t {
    FIRST,
    FullScreen = FIRST,
    WindowedFullScreen,
    Windowed,
    LAST = Windowed
};

struct WindowProperties {
    std::string title;
    WindowMode mode;
    int width;
    int height;
    bool resizable;
    bool vsync;

    DEFINE_VALIDATOR({
        assert(width > 0 && "window width must be greater than zero");
        assert(height > 0 && "window height must be greater than zero");
        assert(!title.empty() && "window title cannot be empty");
        VALIDATE_ENUM(WindowMode, mode, "invalid window mode value");
    });
};

class Window {
    GLFWwindow* window;

public:
    WindowProperties properties;

    explicit Window(WindowProperties initial_properties, bool switch_context = true);

    ~Window() {
        if (window != nullptr)
            glfwDestroyWindow(window);
    }

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept
        : window{other.window}, properties{std::move(other.properties)} {
        other.window = nullptr;
        if (window)
            glfwSetWindowUserPointer(window, this);
    }

    Window& operator=(Window&& other) noexcept {
        if (this == &other)
            return *this;

        if (window)
            glfwDestroyWindow(window);

        window       = other.window;
        other.window = nullptr;
        glfwSetWindowUserPointer(window, this);

        properties = std::move(other.properties);

        return *this;
    }

    bool should_close() const { return glfwWindowShouldClose(window); }

    void sync_properties();

    void dump_ogl_info() const;
    void set_ogl_version_title();

    void swap_buffers() { return glfwSwapBuffers(window); }

private:
    std::pair<GLFWmonitor*, const GLFWvidmode*> resolve_display_state();
};
