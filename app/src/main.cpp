#include "core/glfw_ctx.h"
#include "main_app.h"

int main() {
    // RAII singleton for automatic GLFW lifetime management and error logging
    GlfwCtx::instance(true);

    MainApp app{};
    app.init();
    app.run();

    return EXIT_SUCCESS;
}
