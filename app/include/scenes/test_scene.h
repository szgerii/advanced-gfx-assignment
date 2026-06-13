#pragma once

#include "core/mesh.h"
#include "core/scene.h"
#include "render/common.h"
#include "render/passes/opaque_pass.h"
#include "render/render_pipeline.h"
#include "vendor.h"

class TestScene : public IScene {
    BasicMesh& suzanne_mesh_;
    MinimalMesh& skybox_cube_mesh_;

    ProgramInterface& mesh_iface_;
    ProgramInterface& skybox_iface_;

    glm::mat4 suzanne_model_ = glm::identity<glm::mat4>();

public:
    explicit TestScene(BasicMesh& suzanne_mesh, MinimalMesh& skybox_cube_mesh,
                       ProgramInterface& mesh_iface, ProgramInterface& skybox_iface)
        : IScene{"Suzanne Test Scene"},
          suzanne_mesh_{suzanne_mesh},
          skybox_cube_mesh_{skybox_cube_mesh},
          mesh_iface_{mesh_iface},
          skybox_iface_{skybox_iface} {}

    void update(const UpdateInfo& update_info) override;
    void record_cmds(RenderQueue& queue) const override;
    void render_gui() override;
};
