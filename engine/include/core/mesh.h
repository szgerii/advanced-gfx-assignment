#pragma once

#include <format>
#include <memory>

#include "core/logger.h"
#include "gl/buffer.h"
#include "gl/vertex_array.h"
#include "utils/validate.h"
#include "vendor.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex;
};

// TODO
struct Meshlet {};

template <typename TVertex = Vertex, typename TIdx = GLuint, typename TMeshlet = Meshlet>
struct MeshCPU {
    std::vector<TVertex> vertices{};
    std::vector<TIdx> indices{};
    std::vector<TMeshlet> meshlets{};
};

// NOTE: Meshlets implies indexed vertices (for now)
enum class MeshDataType : uint8_t { VertexArray, Indexed, Meshlets };

// smart POD meant for the render pipeline, so doesn't care about access and data integrity
// use with care outside the base engine/renderer
// long story short: Mesh allows general use with different vertex (and index) base types on the cpu
// during init/modifications/etc., while the command queue can keep a non-template, raw list of gpu
// representations (since vao contains all of the necessary data format stuff needed for drawing)
struct MeshGPU {
    std::shared_ptr<VertexBuffer> vbo;
    std::shared_ptr<IndexBuffer> ibo;
    std::shared_ptr<ShaderStorageBuffer> meshlet_buffer;

    std::shared_ptr<VertexArray> vao;

    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t meshlet_count;

    MeshDataType mesh_data_type;

    DEFINE_VALIDATOR({
        assert(vao != nullptr);
        VALIDATE(*vao);

        assert(vbo != nullptr && "mesh with uninitialized VBO");
        assert(vertex_count != 0 && "mesh with a vertex count of zero");

        if (mesh_data_type != MeshDataType::VertexArray) {
            assert(index_count != 0 && "indexed or meshlet-based mesh with index count of zero");
            assert(ibo != nullptr && "indexed or meshlet-based mesh with uninitialized IBO");
        } else {
            assert(index_count == 0 && "non-indexed mesh with non-zero index count");
            assert(ibo == nullptr && "non-indexed mesh with assigned IBO");
        }

        if (mesh_data_type == MeshDataType::Meshlets) {
            assert(meshlet_buffer != nullptr &&
                   "meshlet-based mesh with uninitialized meshlet buffer");
            assert(meshlet_count != 0 && "meshlet-based mesh with meshlet count of zero");
        } else {
            assert(meshlet_buffer == nullptr && "non-meshlet-based mesh with meshlet buffer");
            assert(meshlet_count == 0 && "non-meshlet-based mesh with non-zero meshlet count");
        }
    })
};

class IMesh {
protected:
    MeshGPU mesh_gpu_;
    bool dirty_verts_    = true;
    bool dirty_idx_      = true;
    bool dirty_meshlets_ = true;

public:
    explicit IMesh(MeshGPU mesh_gpu)
        : mesh_gpu_{std::move(mesh_gpu)} {}

    virtual ~IMesh() = default;

    bool dirty() const { return dirty_verts_ || dirty_idx_ || dirty_meshlets_; }
    bool dirty_vertices() const { return dirty_verts_; }
    bool dirty_indices() const { return dirty_idx_; }
    bool dirty_meshlets() const { return dirty_meshlets_; }

    // 2-stage upload to avoid virtual call when no buffer is dirty, which should be most of the
    // frames
    void commit() {
        if (dirty())
            upload();
    }

    const auto& mesh_gpu() const { return mesh_gpu_; }

protected:
    // implementation for the actual CPU -> GPU data upload
    // can assume dirty() == true
    virtual void upload() = 0;

    void clear_dirty_flags() {
        dirty_verts_    = false;
        dirty_idx_      = false;
        dirty_meshlets_ = false;
    }
};

// "asset-wrapper" for meshes
// central interface for managing vertex-based geometry
// supports raw vertex lists, indexed drawing, meshlet-based drawing (and meshlet gen), etc.
// useful for meshes that can get updated on the CPU side, since they only need to get actually
// uploaded to the GPU if their content is changed (is_dirty_)
// reason for IMesh is to be able to virtual dispatch on update ONLY when an upload is needed, but
// not during a render pass, or a non-modifying update
template <typename TVertex = Vertex, typename TIdx = GLuint, typename TMeshlet = Meshlet>
class Mesh final : public IMesh {
    using TMeshCPU = MeshCPU<TVertex, TIdx, TMeshlet>;

    TMeshCPU mesh_cpu_;

public:
    explicit Mesh(TMeshCPU mesh_cpu, MeshGPU mesh_gpu, bool needs_upload = true)
        : IMesh{std::move(mesh_gpu)}, mesh_cpu_{std::move(mesh_cpu)} {
        VALIDATE(mesh_gpu_);

        if (needs_upload)
            upload();
        else
            clear_dirty_flags();
    }

    const auto& mesh_cpu() const { return mesh_cpu_; }
    const auto& vertices() const { return mesh_cpu_.vertices; }
    const auto& indices() const { return mesh_cpu_.indices; }
    const auto& meshlets() const { return mesh_cpu_.meshlets; }

    auto& modify_vertices() {
        dirty_verts_ = true;
        return mesh_cpu_.vertices;
    }

    auto& modify_indices() {
        dirty_idx_ = true;
        return mesh_cpu_.indices;
    }

    auto& modify_meshlets() {
        dirty_meshlets_ = true;
        return mesh_cpu_.meshlets;
    }

    MeshDataType mesh_data_type() const { return mesh_gpu_.mesh_data_type; }

protected:
    void upload() override {
        if (dirty_verts_) {
            if (mesh_gpu_.vbo == nullptr)
                Logger::critical_error("Mesh",
                                       "Trying to commit to a VBO before it has been initialized");

            mesh_gpu_.vbo->upload(mesh_cpu_.vertices.data(), 0);
        }

        if (dirty_idx_ && mesh_data_type() != MeshDataType::VertexArray) {
            if (mesh_gpu_.ibo == nullptr)
                Logger::critical_error("Mesh",
                                       "Trying to commit to an IBO before it has been initialized");

            mesh_gpu_.ibo->upload(mesh_cpu_.indices.data(), 0);
        }

        if (dirty_meshlets_ && mesh_data_type() == MeshDataType::Meshlets) {
            if (mesh_gpu_.meshlet_buffer == nullptr)
                Logger::critical_error(
                    "Mesh", "Trying to commit to a meshlet buffer before it has been initialized");

            mesh_gpu_.meshlet_buffer->upload(mesh_cpu_.meshlets.data(), 0);
        }

        clear_dirty_flags();
    }
};

using MinimalMesh = Mesh<glm::vec3>;
using BasicMesh   = Mesh<>;
