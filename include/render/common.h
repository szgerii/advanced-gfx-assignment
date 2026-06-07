#pragma once

#include <cstdint>
#include <memory>

#include "gl/buffer.h"
#include "gl/program_interface.h"
#include "gl/vertex_array.h"
#include "utils/validate.h"
#include "vendor.h"

enum class RenderLayer : uint8_t { FIRST = 0, Background = FIRST, Opaque, GUI, LAST = GUI, COUNT };

struct Mesh {
    std::shared_ptr<VertexBuffer> vbo;
    std::shared_ptr<IndexBuffer> ebo;
    VertexArray vao;
    uint32_t vertex_count = 0;
    uint32_t idx_count    = 0;

    DEFINE_VALIDATOR({
        assert(vbo != nullptr && "VBO in uninitialized state is not allowed");
        // allowed for now (used to indicate regular array draws)
        // assert(ebo != nullptr && "EBO in uninitialized state is not allowed");
        VALIDATE(vao);
        if (ebo == nullptr) {
            assert(idx_count == 0 && "mesh without an EBO cannot have a non-zero index count");
        } else {
            assert(idx_count != 0 && "mesh with an EBO cannot have an index count of zero");
        }
        assert(vertex_count != 0 && "mesh cannot have a vertex count of zero");
    })
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

template <typename VertexType = Vertex, typename IndexType = GLuint>
struct MeshCPU {
    std::vector<VertexType> vertices{};
    std::vector<IndexType> indices{};
};

struct DrawCommand {
    Mesh* mesh;
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
