#pragma once

#include "core/common.h"
#include "render/camera.h"
#include "vendor.h"

// source: lab code

class CameraController {
public:
    CameraController(Camera& target_camera);

    ~CameraController();

    void set_state_from_camera();
    void update(const UpdateInfo& update_info);

    inline void set_speed(float speed) { movement_speed_ = speed; }
    inline float speed() const noexcept { return movement_speed_; }

    void process_keyboard_down(int key, bool just_pressed);
    void process_keyboard_up(int key);
    void process_mouse_move(float dx, float dy, bool left_btn_state, bool right_btn_state);
    void process_mouse_scroll(float x_offset, float y_offset);

private:
    Camera& target_cam_;

    // spherical coordinates
    float u_ = 0.f, v_ = 0.f;

    // sphere center
    glm::vec3 center_ = glm::vec3(0.f);

    // distance between camera at and eye
    float distance_ = 0.f;

    glm::vec3 world_up_ = glm::vec3(0.f, 1.f, 0.f);

    // TODO: mouse_speed_ ?
    float movement_speed_ = 16.f;

    // per-direction movement indicators
    float go_forward_ = 0.f;
    float go_right_   = 0.f;
    float go_up_      = 0.f;
};