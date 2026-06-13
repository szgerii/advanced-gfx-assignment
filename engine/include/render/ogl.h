#pragma once

#include <cstdint>

#include "core/logger.h"
#include "core/mesh.h"
#include "render/common.h"
#include "utils/num_cast.h"
#include "utils/validate.h"
#include "vendor.h"

class OGL {
    inline static uint32_t pack_alignment_   = 4;
    inline static uint32_t unpack_alignment_ = 4;

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

    // use these setters, do not configure via glPixelStore directly
    // helpers rely on keeping track of its current value for debug bounds checking
    static void set_pixel_unpack_alignment(uint32_t alignment) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, num_cast<GLint>(alignment));
        unpack_alignment_ = alignment;
    }
    static uint32_t unpack_alignment() { return unpack_alignment_; }

    static void set_pixel_pack_alignment(uint32_t alignment) {
        glPixelStorei(GL_PACK_ALIGNMENT, num_cast<GLint>(alignment));
        pack_alignment_ = alignment;
    }
    static uint32_t pack_alignment() { return pack_alignment_; }

    // TODO: use fixed sized integral types instead of GL-specific ones everywhere else too
    static void viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
        assert(width >= 0);
        assert(height >= 0);

        glViewport(x, y, width, height);
    }

    static void draw_mesh(const MeshGPU& mesh) {
        VALIDATE(mesh);

        if (mesh.mesh_data_type == MeshDataType::Indexed)
            glDrawElements(GL_TRIANGLES, num_cast<GLsizei>(mesh.index_count), GL_UNSIGNED_INT,
                           nullptr);
        else if (mesh.mesh_data_type == MeshDataType::Meshlets)
            Logger::critical_error("OGL", "Meshlet-based meshes are unsupported for now");
        else
            glDrawArrays(GL_TRIANGLES, 0, num_cast<GLsizei>(mesh.vertex_count));
    }
};
