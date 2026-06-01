#include "app/window.h"

Window::Window(WindowProperties initial_properties, bool switch_context)
    : properties{std::move(initial_properties)} {
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
    glfwWindowHint(GLFW_RESIZABLE, properties.resizable ? GLFW_TRUE : GLFW_FALSE);

#ifdef NDEBUG
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_FALSE);
#else
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif

    auto [monitor, mode] = resolve_display_state();

    if (properties.mode == WindowMode::WindowedFullScreen) {
        assert(mode != nullptr);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    window = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(),
                              monitor, nullptr);
    if (window == nullptr)
        Logger::instance().critical_error(
            "Window", std::format("failed to create window! (title: '{}')", properties.title));

    glfwSetWindowUserPointer(window, this);

    if (switch_context) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(properties.vsync ? 1 : 0);
    }
}

void Window::sync_properties() {

    VALIDATE(properties);

    auto [monitor, mode] = resolve_display_state();

    glfwSetWindowTitle(window, properties.title.c_str());
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, properties.resizable ? GLFW_TRUE : GLFW_FALSE);

    int pos_x = 0, pos_y = 0;
    if (properties.mode == WindowMode::Windowed)
        glfwGetWindowPos(window, &pos_x, &pos_y);

    glfwSetWindowMonitor(window, monitor, pos_x, pos_y, properties.width, properties.height,
                         mode != nullptr ? mode->refreshRate : GLFW_DONT_CARE);

    if (glfwGetCurrentContext() == window)
        glfwSwapInterval(properties.vsync ? 1 : 0);
}

void Window::dump_ogl_info() const {
    auto& logger = Logger::instance();

    GLint major_ver, minor_ver;
    glGetIntegerv(GL_MAJOR_VERSION, &major_ver);
    glGetIntegerv(GL_MINOR_VERSION, &minor_ver);
    const char* full_ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    logger.info("Window",
                std::format("OpenGL version: {}.{} [{}]", major_ver, minor_ver, full_ver));

    const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    logger.info("Window", std::format("Renderer: {} [{}]", renderer, vendor));

    const char* glsl_ver = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    logger.info("Window", std::format("GLSL version: {}", glsl_ver));
}

void Window::set_ogl_version_title() {
    GLint major_ver, minor_ver;
    glGetIntegerv(GL_MAJOR_VERSION, &major_ver);
    glGetIntegerv(GL_MINOR_VERSION, &minor_ver);

    properties.title = std::format("OpenGL {}.{}", major_ver, minor_ver);
    sync_properties();
}

std::pair<GLFWmonitor*, const GLFWvidmode*> Window::resolve_display_state() {
    VALIDATE(properties);

    GLFWmonitor* monitor = nullptr;
    if (properties.mode != WindowMode::Windowed) {
        monitor = glfwGetPrimaryMonitor();

        if (monitor == nullptr) {
            Logger::instance().error(
                "Window",
                std::format("non-windowed display mode was requested, but primary "
                            "monitor couldn't be retrieved from GLFW. Switching to windowed mode.",
                            properties.mode == WindowMode::WindowedFullScreen ? "windowed " : ""));

            properties.mode = WindowMode::Windowed;
        }
    }

    const GLFWvidmode* mode = nullptr;
    if (properties.mode == WindowMode::WindowedFullScreen ||
        properties.mode == WindowMode::FullScreen) {

        assert(monitor != nullptr);
        mode = glfwGetVideoMode(monitor);

        if (mode == nullptr) {
            Logger::instance().error(
                "Window", "non-windowed display mode was requested, but current video mode "
                          "couldn't be retrieved from GLFW. Switching to windowed mode.");

            properties.mode = WindowMode::Windowed;
            monitor         = nullptr;
        } else if (properties.mode == WindowMode::WindowedFullScreen) {
            properties.width  = mode->width;
            properties.height = mode->height;
        }
    }

    return {monitor, mode};
}
