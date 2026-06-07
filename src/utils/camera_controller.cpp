#include "utils/camera_controller.h"

#include "render/camera.h"

// source: lab code

CameraController::CameraController(Camera& target_camera)
    : target_cam_(target_camera) {
    set_state_from_camera();
}

CameraController::~CameraController() {}

void CameraController::set_state_from_camera() {
    center_ = target_cam_.at();

    glm::vec3 to_aim = center_ - target_cam_.eye();

    distance_ = glm::length(to_aim);

    u_ = atan2f(to_aim.z, to_aim.x);
    v_ = acosf(to_aim.y / distance_);

    world_up_ = target_cam_.up();
}

void CameraController::update(const UpdateInfo& update_info) {
    // updates the actual camera based on current state of CameraController

    glm::vec3 look_dir(cosf(u_) * sinf(v_), cosf(v_), sinf(u_) * sinf(v_));

    glm::vec3 eye = center_ - distance_ * look_dir;
    glm::vec3 up  = world_up_;

    // CLEANUP
    // minor optimization could be only redoing these calculations if look_dir (or world up)
    // actually changed since the last update and/or delta_pos won't be (0, 0, 0)

    glm::vec3 right = glm::normalize(glm::cross(look_dir, up));
    glm::vec3 fwd   = glm::cross(up, right);

    glm::vec3 delta_pos = (go_forward_ * fwd + go_right_ * right + go_up_ * up) * movement_speed_ *
                          (update_info.delta_ms / 1000.f);

    eye += delta_pos;
    center_ += delta_pos;

    target_cam_.set_view(eye, center_, world_up_);
}

// this should be called for EVERY frame a key is pressed down
// (just_pressed indicating the first)
void CameraController::process_keyboard_down(int key, bool just_pressed) {
    switch (key) {
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            if (just_pressed)
                movement_speed_ /= 4.f;
            break;
        case GLFW_KEY_W:
            go_forward_ = 1;
            break;
        case GLFW_KEY_S:
            go_forward_ = -1;
            break;
        case GLFW_KEY_A:
            go_right_ = -1;
            break;
        case GLFW_KEY_D:
            go_right_ = 1;
            break;
        case GLFW_KEY_E:
            go_up_ = 1;
            break;
        case GLFW_KEY_Q:
            go_up_ = -1;
            break;
    }
}

// this should be called ONCE on key release
void CameraController::process_keyboard_up(int key) {
    switch (key) {
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            movement_speed_ *= 4.0f;
            break;
        case GLFW_KEY_W:
        case GLFW_KEY_S:
            go_forward_ = 0;
            break;
        case GLFW_KEY_A:
        case GLFW_KEY_D:
            go_right_ = 0;
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_E:
            go_up_ = 0;
            break;
    }
}

void CameraController::process_mouse_move(float dx, float dy, bool left_btn_down,
                                          bool right_btn_down) {
    if (left_btn_down) {
        float du = dx / 100.f;
        float dv = dy / 100.f;

        u_ += du;
        v_ = glm::clamp<float>(v_ + dv, .1f, 3.1f);
    }

    if (right_btn_down)
        distance_ *= static_cast<float>(pow(0.9f, dy / 50.f));
}

void CameraController::process_mouse_scroll([[maybe_unused]] float x_offset, float y_offset) {
    distance_ *= powf(0.9f, static_cast<float>(y_offset));
}