#pragma once

#include "core/scene.h"
#include "render/common.h"
#include "render/passes/opaque_pass.h"
#include "render/render_pipeline.h"
#include "vendor.h"

class TestScene : public IScene {
    Mesh& suzanne_gpu_;
    Mesh& skybox_cube_gpu_;

    ProgramInterface& mesh_iface_;
    ProgramInterface& skybox_iface_;

    glm::mat4 suzanne_model_ = glm::identity<glm::mat4>();

public:
    explicit TestScene(Mesh& suzanne_gpu, Mesh& skybox_cube_gpu, ProgramInterface& mesh_iface,
                       ProgramInterface& skybox_iface)
        : IScene{"Suzanne Test Scene"},
          suzanne_gpu_{suzanne_gpu},
          skybox_cube_gpu_{skybox_cube_gpu},
          mesh_iface_{mesh_iface},
          skybox_iface_{skybox_iface} {}

    void update(const UpdateInfo& update_info) override;
    void record_cmds(RenderQueue& queue) const override;
    void render_gui() override;
};
