#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "app/common.h"
#include "gui/scene_panel.h"
#include "render/camera.h"
#include "render/render_queue.h"
#include "utils/camera_controller.h"
#include "vendor.h"

class IScene {
    static constexpr auto INITIAL_EYE = glm::vec3{0.f};
    static constexpr auto INITIAL_AT  = glm::vec3{0.f, 0.f, -1.f};

public:
    std::string name;
    Camera camera;

private:
    friend class gui::ScenePanel;

    std::optional<CameraController> cam_controller_{};
    std::unique_ptr<gui::ScenePanel> scene_panel_;

public:
    explicit IScene(std::string name, std::unique_ptr<gui::ScenePanel> scene_panel,
                    bool use_cam_controller = true)
        : name{std::move(name)},
          camera{INITIAL_EYE, INITIAL_AT},
          scene_panel_{std::move(scene_panel)} {

        if (use_cam_controller)
            cam_controller_.emplace(camera);
    }

    explicit IScene(std::string name, bool use_cam_controller = true)
        : IScene{std::move(name), nullptr, use_cam_controller} {
        scene_panel_ = std::make_unique<gui::ScenePanel>(*this);
    }

    virtual ~IScene() = default;

    IScene(const IScene&)            = delete;
    IScene& operator=(const IScene&) = delete;
    IScene(IScene&&)                 = delete;
    IScene& operator=(IScene&&)      = delete;

    // handles any per-scene logic updates needed
    virtual void update(const UpdateInfo& update_info) = 0;

    // records draw commands into queue
    virtual void record_cmds(RenderQueue& queue) const = 0;

    // puts together the per-scene gui via ImGui (without frame init or render)
    virtual void render_gui() = 0;

    void render_scene_panel() { scene_panel_->render_gui(); }

    void update_camera_controller(const UpdateInfo& update_info) {
        if (cam_controller_.has_value())
            cam_controller_->update(update_info);
    }

    // exposing the camera controller this way allows scene owners (e.g. app) to modify or update
    // the controller's state, while forbidding them to add/remove or entirely swap out the
    // controller (that remains in the control of the given scene itself)
    using MaybeCamController = std::optional<std::reference_wrapper<CameraController>>;
    MaybeCamController cam_controller() {
        // when collapsed into a ternary, gcc fails to be able to reason out their "common" type
        if (cam_controller_.has_value())
            return *cam_controller_;

        return std::nullopt;
    }
};
