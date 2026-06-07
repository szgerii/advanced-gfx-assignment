#include "app/application.h"
#include "app/window.h"
#include "gl/buffer.h"
#include "gl/debug.h"
#include "gl/shader_program.h"
#include "gl/vertex_array.h"
#include "glfw_ctx.h"
#include "logger.h"
#include "render/render_queue.h"
#include "vendor.h"

#include <array>

struct VertexCol {
    glm::vec2 pos;
    glm::vec3 color;
};

// clang-format off
static constexpr std::array vertices{
    VertexCol{{-0.5f,  0.5f}, {1.f, 0.f, 0.f}}, // top left
    VertexCol{{ 0.5f,  0.5f}, {0.f, 1.f, 0.f}}, // top right
    VertexCol{{ 0.5f, -0.5f}, {0.f, 0.f, 1.f}}, // bottom right
    VertexCol{{-0.5f, -0.5f}, {1.f, 1.f, 1.f}}, // bottom left
};
// clang-format on

static constexpr std::array<uint32_t, 6> indices{0, 3, 2, 2, 1, 0};

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
        Logger::critical_error("GLAD", "failed to initialize GLAD for current OpenGL context");

    GLint ctx_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &ctx_flags);
    if (ctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        static OpenGLDebugCallbackConfig debug_cb_config{.show_notifications = false};

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // for breakpoints
        glDebugMessageCallback(ogl_debug_msg_callback, &debug_cb_config);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    main_win.dump_ogl_info();
    // main_win.set_info_title();

    Application app{main_win};
    app.run();

    return EXIT_SUCCESS;
}
