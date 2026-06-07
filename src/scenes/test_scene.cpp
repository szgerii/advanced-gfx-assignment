#include "scenes/test_scene.h"

void TestScene::update(const UpdateInfo& update_info) {
    suzanne_model_ =
        glm::rotate(suzanne_model_, static_cast<float>(M_PI * (update_info.delta_ms / 1000.f)),
                    glm::vec3(0.f, 1.f, 0.f));
}

void TestScene::record_cmds(RenderQueue& queue) const {
    queue.enqueue(DrawCommand{.mesh      = &suzanne_gpu_,
                              .interface = prog_iface_.get(),
                              .world     = suzanne_model_,
                              .view_proj = camera.view_proj()});
}

void TestScene::render_gui() {}
