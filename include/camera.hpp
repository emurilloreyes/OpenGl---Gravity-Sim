#pragma once

#include "math.hpp"

class OrbitCamera {
public:
    Vec3 target{0.0f, 0.0f, 0.0f};
    float yaw = 0.6f;
    float pitch = 0.35f;
    float distance = 12.0f;
    float fovY = 0.9f;
    float aspect = 1.0f;
    float nearZ = 0.1f;
    float farZ = 200.0f;

    void setAspect(float aspectRatio) { aspect = aspectRatio; }

    void orbit(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw;
        pitch += deltaPitch;
        const float maxPitch = 1.45f;
        if (pitch > maxPitch) pitch = maxPitch;
        if (pitch < -maxPitch) pitch = -maxPitch;
    }

    void zoom(float delta) {
        distance -= delta;
        if (distance < 2.0f) distance = 2.0f;
        if (distance > 80.0f) distance = 80.0f;
    }

    Vec3 eyePosition() const {
        float cp = std::cos(pitch);
        return target + Vec3{
            distance * cp * std::cos(yaw),
            distance * std::sin(pitch),
            distance * cp * std::sin(yaw)
        };
    }

    Mat4 viewMatrix() const {
        return lookAt(eyePosition(), target, Vec3{0.0f, 1.0f, 0.0f});
    }

    Mat4 projectionMatrix() const {
        return perspective(fovY, aspect, nearZ, farZ);
    }

    Mat4 viewProjection() const {
        return multiply(projectionMatrix(), viewMatrix());
    }
};
