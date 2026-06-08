#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "glfw_ctx.h"
#include "logger.h"
#include "render/ogl.h"
#include "utils/callback.h"
#include "utils/validate.h"
#include "vendor.h"

class Application;

enum class WindowMode : uint8_t {
    FIRST,
    FullScreen = FIRST,
    WindowedFullScreen,
    Windowed,
    LAST = Windowed
};

struct WindowProperties {
    static constexpr std::string_view DEFAULT_WINDOW_TITLE = "OpenGL Application";

    std::string title{DEFAULT_WINDOW_TITLE};
    WindowMode mode = WindowMode::Windowed;
    int width       = 1280;
    int height      = 720;
    bool resizable  = true;
    bool vsync      = true;
    bool iconified  = false;
    bool maximized  = false;

    DEFINE_VALIDATOR({
        assert(width > 0 && "window width must be greater than zero");
        assert(height > 0 && "window height must be greater than zero");
        assert(!title.empty() && "window title cannot be empty");
        VALIDATE_ENUM(WindowMode, mode);
    });
};

// decoupled for potential non-GLFW backend support later
using FramebufferSizeCallback = Callback<void, int /* width */, int /* height */>;
using CursorPosCallback       = Callback<void, double /* xpos */, double /* ypos */>;
using ScrollCallback          = Callback<void, double /* xoffset */, double /* yoffset */>;
using MouseButtonCallback     = Callback<void, int /* button */, int /* action */, int /* mods */>;
using KeyCallback =
    Callback<void, int /* key */, int /* scancode */, int /* action */, int /* mods */>;

class Window {
    GLFWwindow* window_{};

public:
    WindowProperties properties;

    // if set to true, when no manual framebuffer size callback is bound to the window, it will
    // implicitly fall back to one simply updating the graphics context's viewport size
    // NOTE: when a callback is provided, it's still the callback's job to update the viewport
    bool use_framebuffer_size_fallback = true;

private:
    FramebufferSizeCallback framebuffer_size_cb_{};
    CursorPosCallback cursor_pos_cb_{};
    ScrollCallback scroll_cb_{};
    MouseButtonCallback mouse_btn_cb_{};
    KeyCallback key_cb_{};

    WindowMode last_window_mode_;
    int last_windowed_width_  = -1;
    int last_windowed_height_ = -1;
    int last_windowed_pos_x_  = -1;
    int last_windowed_pos_y_  = -1;

public:
    explicit Window(WindowProperties initial_properties, bool switch_context = true);

    ~Window() {
        if (window_ != nullptr)
            glfwDestroyWindow(window_);
    }

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept
        : window_{other.window_}, properties{std::move(other.properties)} {
        other.window_ = nullptr;

        glfwSetWindowUserPointer(window_, this);
        update_callbacks();
    }

    Window& operator=(Window&& other) noexcept {
        if (this == &other)
            return *this;

        if (window_)
            glfwDestroyWindow(window_);

        window_       = other.window_;
        other.window_ = nullptr;

        properties = std::move(other.properties);

        glfwSetWindowUserPointer(window_, this);
        update_callbacks();

        return *this;
    }

    GLFWwindow* get_handle() { return window_; }
    const GLFWwindow* get_handle() const { return window_; }

    bool should_close() const {
        return glfwWindowShouldClose(window_) || GlfwCtx::instance().terminate_next_frame();
    }

    void sync_properties();

    void dump_ogl_info() const;

#ifndef NDEBUG
    void set_info_title(bool include_app_name = true, bool include_ogl_version = true,
                        bool include_glfw_version = true);
#else
    void set_info_title(bool include_app_name = true, bool include_ogl_version = false,
                        bool include_glfw_version = false);
#endif

    void swap_buffers() { glfwSwapBuffers(window_); }

    void update_viewport() {
        int width, height;
        glfwGetWindowSize(window_, &width, &height);
        OGL::viewport(0, 0, width, height);
    }

    void set_cursor_pos_callback(CursorPosCallback cursor_pos_cb) {
        cursor_pos_cb_ = cursor_pos_cb;
        update_callbacks();
    }

    void set_scroll_callback(ScrollCallback scroll_cb) {
        scroll_cb_ = scroll_cb;
        update_callbacks();
    }

    void set_mouse_button_callback(MouseButtonCallback mouse_button_cb) {
        mouse_btn_cb_ = mouse_button_cb;
        update_callbacks();
    }

    void set_key_callback(KeyCallback key_cb) {
        key_cb_ = key_cb;
        update_callbacks();
    }

    void set_framebuffer_size_callback(FramebufferSizeCallback framebuffer_size_cb) {
        framebuffer_size_cb_ = framebuffer_size_cb;
        update_callbacks();
    }

private:
    void update_callbacks();

    static void window_size_callback(GLFWwindow* window, int width, int height);
    static void window_maximize_callback(GLFWwindow* window, int maximized);
    static void window_iconify_callback(GLFWwindow* window, int iconified);

    // these wrap the actual callbacks set up by the app/user
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::pair<GLFWmonitor*, const GLFWvidmode*> resolve_display_state();
};
