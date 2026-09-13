#pragma once

#include "Math.hpp"

namespace Aetheria {

class Camera {
public:
    Camera();

    void setPosition(const Vec3& pos);
    const Vec3& getPosition() const { return position; }

    void addYawPitch(float yawOffset, float pitchOffset);
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }

    void toggleThirdPerson() { isThirdPerson = !isThirdPerson; }
    void setThirdPerson(bool tp) { isThirdPerson = tp; }
    bool getIsThirdPerson() const { return isThirdPerson; }
    void setThirdPersonDistance(float dist) { thirdPersonDistance = dist; }

    Vec3 getForward() const;
    Vec3 getRight() const;
    Vec3 getUp() const;
    Vec3 getRenderPosition() const;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspect) const;

    void setFOV(float fovDeg) { fov = fovDeg; }
    float getFOV() const { return fov; }

    void setZooming(bool zooming) { isZooming = zooming; }
    bool getIsZooming() const { return isZooming; }
    void updateZoom(float baseFOV, float dt, bool isSprinting = false);
    float getCurrentFOV() const { return currentFOV; }
    float getZoomRatio() const { return (fov > 0.0f) ? (currentFOV / fov) : 1.0f; }

    void updateCrouch(bool isCrouching, float dt);
    float getEyeHeight() const { return eyeHeight; }

private:
    Vec3 position{0.0f, 32.0f, 0.0f};
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 75.0f;
    float currentFOV = 75.0f;
    float eyeHeight = 1.62f;
    float targetEyeHeight = 1.62f;
    bool isZooming = false;
    bool isThirdPerson = false;
    float thirdPersonDistance = 4.0f;
};

} // namespace Aetheria
