#include "Camera.hpp"
#include <algorithm>

namespace Aetheria {

Camera::Camera() {
}

void Camera::setPosition(const Vec3& pos) {
    position = pos;
}

void Camera::addYawPitch(float yawOffset, float pitchOffset) {
    yaw += yawOffset;
    pitch += pitchOffset;
    pitch = std::clamp(pitch, -89.0f, 89.0f);
}

Vec3 Camera::getForward() const {
    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;

    Vec3 front;
    front.x = std::cos(pitchRad) * std::cos(yawRad);
    front.y = std::sin(pitchRad);
    front.z = std::cos(pitchRad) * std::sin(yawRad);
    return front.normalized();
}

Vec3 Camera::getRight() const {
    return getForward().cross(Vec3(0.0f, 1.0f, 0.0f)).normalized();
}

Vec3 Camera::getUp() const {
    return getRight().cross(getForward()).normalized();
}

Vec3 Camera::getRenderPosition() const {
    if (!isThirdPerson) {
        return position + Vec3(0.0f, eyeHeight, 0.0f);
    } else {
        Vec3 eyePos = position + Vec3(0.0f, eyeHeight, 0.0f);
        return eyePos - getForward() * thirdPersonDistance;
    }
}

Mat4 Camera::getViewMatrix() const {
    Vec3 eye = getRenderPosition();
    Vec3 target = eye + getForward();
    return Mat4::lookAt(eye, target, Vec3(0.0f, 1.0f, 0.0f));
}

Mat4 Camera::getProjectionMatrix(float aspect) const {
    return Mat4::perspectiveVK(currentFOV * DEG2RAD, aspect, 0.05f, 500.0f);
}

void Camera::updateCrouch(bool crouching, float dt) {
    targetEyeHeight = crouching ? 1.26f : 1.62f;
    eyeHeight += (targetEyeHeight - eyeHeight) * std::clamp(dt * 12.0f, 0.0f, 1.0f);
}

void Camera::updateZoom(float baseFOV, float dt, bool isSprinting) {
    fov = baseFOV;
    float targetFOV;
    if (isZooming) {
        targetFOV = 24.0f;
    } else if (isSprinting) {
        targetFOV = baseFOV + 6.0f; // Dynamic sprint FOV kick!
    } else {
        targetFOV = baseFOV;
    }
    float zoomSpeed = isZooming ? 14.0f : 10.0f;
    currentFOV += (targetFOV - currentFOV) * std::clamp(dt * zoomSpeed, 0.0f, 1.0f);
}

} // namespace Aetheria
