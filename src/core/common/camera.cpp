//
// Created by william on 2022/10/23.
//

#include "camera.h"

namespace backend
{
Camera::Camera(const glm::vec3& _position, const glm::vec3& _up, float _yaw, float _pitch) :
    front({ 0.0f, 0.0f, -1.0f }), movementSpeed(s_speed), mouseSensitivity(s_sensitivity), zoom(s_zoom)
{
    position = _position;
    worldUp = _up;
    yaw = _yaw;
    pitch = _pitch;
}

Camera::~Camera() = default;

void Camera::updateCameraVectors()
{
    glm::vec3 _front{};
    // calculate the new Front vector
    _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    _front.y = sin(glm::radians(pitch));
    _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(_front);
    // also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    yaw += xoffset;
    pitch += yoffset;
    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }
    updateCameraVectors();
}

void Camera::processKeyboard(MovementType direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    switch (direction)
    {
    case MovementType::Forward:
        position += front * velocity;
        break;
    case MovementType::BackWard:
        position -= front * velocity;
        break;
    case MovementType::Left:
        position -= right * velocity;
        break;
    case MovementType::Right:
        position += right * velocity;
        break;
    }
}

void Camera::processMouseScroll(float yoffset)
{
    zoom -= yoffset;
    zoom = glm::clamp(zoom, 1.0f, 45.0f);
}

} // namespace backend