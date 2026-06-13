#include "gui/scene_panel.h"

#include "core/scene.h"
#include "vendor.h"

namespace {

using namespace gui;

static constexpr float CAM_SLIDER_SPEED = 0.3f;

} // namespace

namespace gui {

ScenePanel::ScenePanel(IScene& scene, ImGuiWindowFlags window_flags)
    : IPanel{std::format("Scene ({})", scene.name), window_flags}, scene_{scene} {}

void ScenePanel::render_base() {
    ImGui::SeparatorText("Scene Info");
    ImGui::Text("Name: %s", scene_.name.c_str());

    ImGui::SeparatorText("Camera Controls");

    Camera& cam      = scene_.camera;
    bool any_updated = false;

    float eye_buffer[3] = {cam.eye().x, cam.eye().y, cam.eye().z};
    ImGui::Text("Eye:");
    any_updated |= ImGui::InputFloat3("##Eye Input", eye_buffer, "%.3f");
    any_updated |= ImGui::DragFloat3("##Eye Drag", eye_buffer, CAM_SLIDER_SPEED);

    float at_buffer[3] = {cam.at().x, cam.at().y, cam.at().z};
    ImGui::Text("At:");
    any_updated |= ImGui::InputFloat3("##At Input", at_buffer, "%.3f");
    any_updated |= ImGui::DragFloat3("##At Drag", at_buffer, CAM_SLIDER_SPEED);

    float up_buffer[3] = {cam.up().x, cam.up().y, cam.up().z};
    ImGui::Text("Up:");
    any_updated |= ImGui::InputFloat3("##Up Input", up_buffer, "%.3f");
    any_updated |= ImGui::DragFloat3("##Up Drag", up_buffer, CAM_SLIDER_SPEED);

    float proj_buffer[4] = {cam.angle(), cam.aspect(), cam.z_near(), cam.z_far()};
    any_updated |= ImGui::InputFloat("Angle", &proj_buffer[0]);
    any_updated |= ImGui::InputFloat("Aspect", &proj_buffer[1]);
    any_updated |= ImGui::InputFloat("Z near", &proj_buffer[2]);
    any_updated |= ImGui::InputFloat("Z far", &proj_buffer[3]);

    if (any_updated) {
        glm::vec3 eye_vec(eye_buffer[0], eye_buffer[1], eye_buffer[2]);
        glm::vec3 at_vec(at_buffer[0], at_buffer[1], at_buffer[2]);
        glm::vec3 up_vec(up_buffer[0], up_buffer[1], up_buffer[2]);

        cam.set_view(eye_vec, at_vec, up_vec);
        cam.set_proj(proj_buffer[0], proj_buffer[1], proj_buffer[2], proj_buffer[3]);
        if (scene_.cam_controller_.has_value())
            scene_.cam_controller_->set_state_from_camera();
    }

    const auto& cur_snapshot = cam.snapshot();
    if (!cur_snapshot.snapshot_type.empty()) {
        ImGui::Text("Snapshot:");

        {
            ImGuiDisabledRAII disable{cam.snapshot_lock};
            if (ImGui::Button("Restore")) {
                cam.restore_snapshot();
                if (scene_.cam_controller_.has_value())
                    scene_.cam_controller_->set_state_from_camera();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Snapshot view"))
            cam.take_snapshot(true, false);

        ImGui::SameLine();
        if (ImGui::Button("Snapshot"))
            cam.take_snapshot(true, true);

        ImGuiDisabledRAII disable{};

        if (cur_snapshot.snapshot_type.has(CameraSnapshot::SnapshotFlag::View)) {
            ImGui::Text("View:");
            auto snap_eye = cur_snapshot.data.eye;
            auto snap_at  = cur_snapshot.data.at;
            auto snap_up  = cur_snapshot.data.up;

            ImGui::DragFloat3("Eye##Snapshot", glm::value_ptr(snap_eye));
            ImGui::DragFloat3("At##Snapshot", glm::value_ptr(snap_at));
            ImGui::DragFloat3("Up##Snapshot", glm::value_ptr(snap_up));
        }

        if (cur_snapshot.snapshot_type.has(CameraSnapshot::SnapshotFlag::Proj)) {
            float snap_proj[4] = {cur_snapshot.data.angle, cur_snapshot.data.aspect,
                                  cur_snapshot.data.z_near, cur_snapshot.data.z_far};
            ImGui::InputFloat("Angle##Snapshot", &snap_proj[0]);
            ImGui::InputFloat("Aspect##Snapshot", &snap_proj[1]);
            ImGui::InputFloat("Z near##Snapshot", &snap_proj[2]);
            ImGui::InputFloat("Z far##Snapshot", &snap_proj[3]);
        }
    }
}

} // namespace gui
