#include "app/window.h"

#include "app/application.h"
#include "meta.h"
#include "render/render_command.h"
#include "vendor.h"

namespace {

void framebuffer_size_callback_fallback([[maybe_unused]] GLFWwindow* window, int width,
                                        int height) {
    RenderCommand::viewport(0, 0, width, height);
}

} // namespace

Window::Window(WindowProperties initial_properties, bool switch_context)
    : properties{std::move(initial_properties)}, last_window_mode_{properties.mode} {
    VALIDATE(properties);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, properties.resizable ? GLFW_TRUE : GLFW_FALSE);

#ifdef NDEBUG
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_FALSE);
#else
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif

    auto [monitor, mode] = resolve_display_state();
    last_window_mode_    = properties.mode;

    if (properties.mode == WindowMode::WindowedFullScreen) {
        assert(mode != nullptr);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    if (properties.maximized)
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window_ = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(),
                               monitor, nullptr);
    if (window_ == nullptr)
        Logger::critical_error(
            "Window", std::format("failed to create window! (title: '{}')", properties.title));

    glfwSetWindowUserPointer(window_, this);

    update_callbacks();

    if (switch_context) {
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(properties.vsync ? 1 : 0);
    }
}

void Window::sync_properties() {
    VALIDATE(properties);

    auto [monitor, mode] = resolve_display_state();

    glfwSetWindowTitle(window_, properties.title.c_str());
    glfwSetWindowAttrib(window_, GLFW_RESIZABLE, properties.resizable ? GLFW_TRUE : GLFW_FALSE);

    int pos_x = 0, pos_y = 0;
    if (properties.mode == WindowMode::Windowed) {
        if (last_window_mode_ != WindowMode::Windowed && last_windowed_width_ >= 0) {
            pos_x             = last_windowed_pos_x_;
            pos_y             = last_windowed_pos_y_;
            properties.width  = last_windowed_width_;
            properties.height = last_windowed_height_;
        } else {
            glfwGetWindowPos(window_, &pos_x, &pos_y);
        }

        int iconified = glfwGetWindowAttrib(window_, GLFW_ICONIFIED);
        int maximized = iconified ? 0 : glfwGetWindowAttrib(window_, GLFW_MAXIMIZED);

        if (!iconified && properties.iconified)
            glfwIconifyWindow(window_);
        if (iconified && !properties.iconified)
            glfwRestoreWindow(window_);

        if (!properties.iconified) {
            if (!maximized && properties.maximized)
                glfwMaximizeWindow(window_);
            else if (maximized && !properties.maximized)
                glfwRestoreWindow(window_);
        }
    }

    glfwSetWindowMonitor(window_, monitor, pos_x, pos_y, properties.width, properties.height,
                         mode != nullptr ? mode->refreshRate : GLFW_DONT_CARE);
    last_window_mode_ = properties.mode;

    if (glfwGetCurrentContext() == window_)
        glfwSwapInterval(properties.vsync ? 1 : 0);
}

void Window::dump_ogl_info() const {
    GLint major_ver, minor_ver;
    glGetIntegerv(GL_MAJOR_VERSION, &major_ver);
    glGetIntegerv(GL_MINOR_VERSION, &minor_ver);
    const char* full_ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    Logger::info("Window",
                 std::format("OpenGL version: {}.{} [{}]", major_ver, minor_ver, full_ver));

    const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    Logger::info("Window", std::format("Renderer: {} [{}]", renderer, vendor));

    const char* glsl_ver = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    Logger::info("Window", std::format("GLSL version: {}", glsl_ver));
}

void Window::set_info_title(bool include_app_name, bool include_ogl_version,
                            bool include_glfw_version) {
    std::string new_title;

    auto title_append = [&new_title](std::string_view part) {
        if (!new_title.empty())
            new_title += " | ";

        new_title += part;
    };

    if (include_app_name)
        title_append(meta::APP_NAME);

    if (include_ogl_version) {
        GLint major_ver, minor_ver;
        glGetIntegerv(GL_MAJOR_VERSION, &major_ver);
        glGetIntegerv(GL_MINOR_VERSION, &minor_ver);

        title_append(std::format("OpenGL {}.{}", major_ver, minor_ver));
    }

    if (include_glfw_version)
        title_append(std::format("GLFW {}.{}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR));

    if (!new_title.empty()) {
        properties.title = std::move(new_title);
    } else {
        Logger::warn(
            "Window",
            std::format("set_info_title called with all title parts disabled, defaulting to '{}'",
                        WindowProperties::DEFAULT_WINDOW_TITLE));

        properties.title = WindowProperties::DEFAULT_WINDOW_TITLE;
    }

    sync_properties();
}

std::pair<GLFWmonitor*, const GLFWvidmode*> Window::resolve_display_state() {
    VALIDATE(properties);

    GLFWmonitor* monitor = nullptr;
    if (properties.mode != WindowMode::Windowed) {
        monitor = glfwGetPrimaryMonitor();

        if (monitor == nullptr) {
            Logger::error(
                "Window",
                std::format("non-windowed display mode was requested, but primary "
                            "monitor couldn't be retrieved from GLFW. Switching to windowed mode.",
                            properties.mode == WindowMode::WindowedFullScreen ? "windowed " : ""));

            properties.mode = WindowMode::Windowed;
        }
    } else if (window_ != nullptr) {
    }

    const GLFWvidmode* mode = nullptr;
    if (properties.mode == WindowMode::WindowedFullScreen ||
        properties.mode == WindowMode::FullScreen) {

        assert(monitor != nullptr);
        mode = glfwGetVideoMode(monitor);

        if (mode == nullptr) {
            Logger::error("Window",
                          "non-windowed display mode was requested, but current video mode "
                          "couldn't be retrieved from GLFW. Switching to windowed mode.");

            properties.mode = WindowMode::Windowed;
            monitor         = nullptr;
        } else {
            if (last_window_mode_ == WindowMode::Windowed) {
                last_windowed_width_  = properties.width;
                last_windowed_height_ = properties.height;
                glfwGetWindowPos(window_, &last_windowed_pos_x_, &last_windowed_pos_y_);
            }

            properties.width  = mode->width;
            properties.height = mode->height;
        }
    }

    return {monitor, mode};
}

void Window::update_callbacks() {
    if (window_ == nullptr)
        return;

    // non-delegated callbacks

    glfwSetWindowSizeCallback(window_, window_size_callback);
    glfwSetWindowMaximizeCallback(window_, window_maximize_callback);
    glfwSetWindowIconifyCallback(window_, window_iconify_callback);

    // (potentially) delegated callbacks

    if (framebuffer_size_cb_.bound())
        glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    else if (use_framebuffer_size_fallback)
        glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback_fallback);
    else
        glfwSetFramebufferSizeCallback(window_, nullptr);

    if (cursor_pos_cb_.bound())
        glfwSetCursorPosCallback(window_, cursor_position_callback);
    else
        glfwSetCursorPosCallback(window_, nullptr);

    if (scroll_cb_.bound())
        glfwSetScrollCallback(window_, scroll_callback);
    else
        glfwSetScrollCallback(window_, nullptr);

    if (mouse_btn_cb_.bound())
        glfwSetMouseButtonCallback(window_, mouse_button_callback);
    else
        glfwSetMouseButtonCallback(window_, nullptr);

    if (key_cb_.bound())
        glfwSetKeyCallback(window_, key_callback);
    else
        glfwSetKeyCallback(window_, nullptr);
}

// CALLBACKS

void Window::window_size_callback(GLFWwindow* window, int width, int height) {
    if (auto win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
        win_ptr->properties.width  = width;
        win_ptr->properties.height = height;
    }
}

void Window::window_maximize_callback(GLFWwindow* window, int maximized) {
    if (auto win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        win_ptr->properties.maximized = maximized;
}

void Window::window_iconify_callback(GLFWwindow* window, int iconified) {
    if (auto win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        win_ptr->properties.iconified = iconified;
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (auto win_ptr = glfwGetWindowUserPointer(window))
        static_cast<Window*>(win_ptr)->framebuffer_size_cb_(width, height);
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (auto win_ptr = glfwGetWindowUserPointer(window))
        static_cast<Window*>(win_ptr)->cursor_pos_cb_(xpos, ypos);
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (auto win_ptr = glfwGetWindowUserPointer(window))
        static_cast<Window*>(win_ptr)->scroll_cb_(xoffset, yoffset);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (auto win_ptr = glfwGetWindowUserPointer(window))
        static_cast<Window*>(win_ptr)->mouse_btn_cb_(button, action, mods);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (auto win_ptr = glfwGetWindowUserPointer(window))
        static_cast<Window*>(win_ptr)->key_cb_(key, scancode, action, mods);
}
