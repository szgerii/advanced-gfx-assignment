#pragma once

#include <cstdint>

#include "render/common.h"
#include "utils/num_cast.h"
#include "utils/validate.h"
#include "vendor.h"

class RenderCommand {
public:
    static void set_clear_color(float r, float g, float b, float a = 1.f) {
        glClearColor(r, g, b, a);
    }

    static void clear(bool color_buffer = true, bool depth_buffer = false,
                      bool stencil_buffer = false) {
        GLbitfield flags = (color_buffer ? GL_COLOR_BUFFER_BIT : 0) |
                           (depth_buffer ? GL_DEPTH_BUFFER_BIT : 0) |
                           (stencil_buffer ? GL_STENCIL_BUFFER_BIT : 0);
        glClear(flags);
    }

    // TODO: use fixed sized integral types instead of GL-specific ones everywhere else too
    static void viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
        assert(width >= 0);
        assert(height >= 0);

        glViewport(x, y, width, height);
    }

    static void draw_mesh(const Mesh& mesh) {
        VALIDATE(mesh);

        if (mesh.idx_count != 0)
            glDrawElements(GL_TRIANGLES, num_cast<GLsizei>(mesh.idx_count), GL_UNSIGNED_INT,
                           nullptr);
        else
            glDrawArrays(GL_TRIANGLES, 0, num_cast<GLsizei>(mesh.vertex_count));
    }
};
