#pragma once

// source: mostly identical to camera code from labs

#include "utils/bit_flags.h"
#include "vendor.h"

struct CameraData {
    glm::vec3 eye{};
    glm::vec3 at{};
    glm::vec3 up{};

    float angle  = glm::radians(27.f);
    float aspect = 1.f;
    float z_near = 0.1f;
    float z_far  = 1000.f;
};

struct CameraSnapshot {
    enum class SnapshotFlag : uint8_t { None = 0, View = 1 << 0, Proj = 1 << 1 };
    using SnapshotFlags = BitFlags<SnapshotFlag>;

    SnapshotFlags snapshot_type{};
    CameraData data{};
};

class Camera {
private:
    CameraData data_;

    glm::mat4 view_{};
    glm::mat4 proj_{};

    // expected to be initialized to the initial camera state
    // make sure to init properly if adding new ctors
    CameraSnapshot snapshot_{};

public:
    // disables restoring snapshots (restore_snapshot() call becomes a no-op)
    // does not prevent snapshot taking with snapshot_view()
    // could be useful for animations and whatnot
    bool snapshot_lock = false;

    explicit Camera(glm::vec3 eye, glm::vec3 at, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f))
        : data_{eye, at, up} {
        update_view();
        update_proj();

        take_snapshot(true, false);
    }

    // modification to data directly is not allowed, use the designated setters
    const CameraData& data() const { return data_; }

    // same goes for snapshot (use take_snapshot/restore_snapshot)
    const CameraSnapshot& snapshot() const { return snapshot_; }

    glm::mat4 view() const { return view_; }
    glm::mat4 proj() const { return proj_; }
    glm::mat4 view_proj() const { return proj_ * view_; }

    glm::vec3 eye() const { return data_.eye; }
    glm::vec3 at() const { return data_.at; }
    glm::vec3 up() const { return data_.up; }

    float angle() const { return data_.angle; }
    float aspect() const { return data_.aspect; }
    float z_near() const { return data_.z_near; }
    float z_far() const { return data_.z_far; }

    void set_view(glm::vec3 eye, glm::vec3 at, glm::vec3 up);

    void set_proj(float angle, float aspect, float z_near, float z_far);
    void set_angle(float angle);
    void set_aspect(float aspect);
    void set_z_near(float z_near);
    void set_z_far(float z_far);

    void take_snapshot(bool view, bool proj);
    void restore_snapshot();

private:
    void update_view();
    void update_proj();
};
