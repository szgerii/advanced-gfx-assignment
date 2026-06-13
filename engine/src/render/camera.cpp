#include "render/camera.h"

namespace {
using SnapshotFlag  = CameraSnapshot::SnapshotFlag;
using SnapshotFlags = CameraSnapshot::SnapshotFlags;
} // namespace

void Camera::set_view(glm::vec3 eye, glm::vec3 at, glm::vec3 up) {
    data_.eye = eye;
    data_.at  = at;
    data_.up  = up;

    update_view();
}

void Camera::set_proj(float angle, float aspect, float z_near, float z_far) {
    data_.angle  = angle;
    data_.aspect = aspect;
    data_.z_near = z_near;
    data_.z_far  = z_far;

    update_proj();
}

void Camera::set_angle(float angle) {
    data_.angle = angle;
    update_proj();
}

void Camera::set_aspect(float aspect) {
    data_.aspect = aspect;
    update_proj();
}

void Camera::set_z_near(float z_near) {
    data_.z_near = z_near;
    update_proj();
}

void Camera::set_z_far(float z_far) {
    data_.z_far = z_far;
    update_proj();
}

void Camera::update_view() {
    view_ = glm::lookAt(data_.eye, data_.at, data_.up);
}

void Camera::update_proj() {
    proj_ = glm::perspective(data_.angle, data_.aspect, data_.z_near, data_.z_far);
}

void Camera::take_snapshot(bool view, bool proj) {
    SnapshotFlags flags{};
    if (view)
        flags |= SnapshotFlag::View;
    if (proj)
        flags |= SnapshotFlag::Proj;

    // we can just copy the entire camera data for simplicity
    // readback should ensure that only the things indicated by snapshot_type are processed
    snapshot_ = CameraSnapshot{.snapshot_type = flags, .data = data_};
}

void Camera::restore_snapshot() {
    if (snapshot_lock)
        return;

    if (snapshot_.snapshot_type.has(SnapshotFlag::View))
        set_view(snapshot_.data.eye, snapshot_.data.at, snapshot_.data.up);

    if (snapshot_.snapshot_type.has(SnapshotFlag::Proj))
        set_proj(snapshot_.data.angle, snapshot_.data.aspect, snapshot_.data.z_near,
                 snapshot_.data.z_far);
}
