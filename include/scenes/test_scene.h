#pragma once

#include "render/common.h"
#include "render/passes/opaque_pass.h"
#include "render/render_pipeline.h"
#include "scene.h"
#include "vendor.h"

class TestScene : public IScene {
    Mesh& suzanne_gpu_;
    std::shared_ptr<ProgramInterface> prog_iface_;

    glm::mat4 suzanne_model_ = glm::identity<glm::mat4>();

public:
    explicit TestScene(Mesh& suzanne_gpu, std::shared_ptr<ProgramInterface> program_interface)
        : IScene{"Suzanne Test Scene"}, suzanne_gpu_{suzanne_gpu}, prog_iface_{program_interface} {}

    void update(const UpdateInfo& update_info) override;
    void record_cmds(RenderQueue& queue) const override;
    void render_gui() override;
};
