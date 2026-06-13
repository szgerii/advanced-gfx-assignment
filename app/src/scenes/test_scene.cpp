#include "scenes/test_scene.h"

#include <numbers>

#include "vendor.h"

void TestScene::update(const UpdateInfo& update_info) {
    suzanne_model_ =
        glm::rotate(suzanne_model_,
                    static_cast<float>(std::numbers::pi_v<float> * (update_info.delta_ms / 1000.f)),
                    glm::vec3(0.f, 1.f, 0.f));
}

void TestScene::record_cmds(RenderQueue& queue) const {
    queue.enqueue(DrawCommand{.mesh      = &skybox_cube_gpu_,
                              .interface = &skybox_iface_,
                              .world     = glm::translate(glm::identity<glm::mat4>(), camera.eye()),
                              .view_proj = camera.view_proj(),
                              .layer     = RenderLayer::Background});

    queue.enqueue(DrawCommand{.mesh      = &suzanne_gpu_,
                              .interface = &mesh_iface_,
                              .world     = suzanne_model_,
                              .view_proj = camera.view_proj(),
                              .layer     = RenderLayer::Opaque});
}

void TestScene::render_gui() {}
