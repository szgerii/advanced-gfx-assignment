#include "app/window.h"
#include "gl/debug.h"
#include "logger.h"
#include "wrapper/glfw_ctx.h"

int main() {
    // RAII singleton for automatic GLFW lifetime management and error logging
    GlfwCtx::instance(true);

    WindowProperties main_win_props{.title     = "My Window",
                                    .mode      = WindowMode::Windowed,
                                    .width     = 1280,
                                    .height    = 720,
                                    .resizable = true,
                                    .vsync     = true};
    Window main_win{main_win_props};

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
        Logger::instance().critical_error("GLAD",
                                          "failed to initialize GLAD for current OpenGL context");

    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR) {
        Logger::instance().info("Info", std::format("error code: {}", gl_error));

        gl_error = glGetError();
    }

    GLint ctx_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &ctx_flags);
    if (ctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // for breakpoints
        glDebugMessageCallback(ogl_debug_msg_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    main_win.dump_ogl_info();
    main_win.set_ogl_version_title();

    glEnable(GL_INVALID_ENUM);

    glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
    while (!main_win.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);

        main_win.swap_buffers();

        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
