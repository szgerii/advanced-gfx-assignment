#pragma once

#include <cstdint>
#include <memory>

#include "core/mesh.h"
#include "gl/program_interface.h"
#include "utils/validate.h"
#include "vendor.h"

enum class RenderLayer : uint8_t { FIRST = 0, Background = FIRST, Opaque, LAST = Opaque, COUNT };

struct DrawCommand {
    const MeshGPU* mesh;
    ProgramInterface* interface;
    glm::mat4 world;
    glm::mat4 view_proj;
    RenderLayer layer = RenderLayer::Opaque;

    DEFINE_VALIDATOR({
        assert(mesh != nullptr && "mesh is uninitialized");
        VALIDATE(*mesh);
        assert(interface != nullptr && "interface is uninitialized");
        VALIDATE(*interface);
        VALIDATE_ENUM(RenderLayer, layer);
    })
};
