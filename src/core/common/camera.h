//
// Created by william on 2022/10/23.
//

#ifndef LEARNMETALVULKAN_CAMERA_H
#define LEARNMETALVULKAN_CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace backend
{
struct Camera
{
public:
    enum class MovementType
    {
        Forward,
        BackWard,
        Left,
        Right
    };

public:
    Camera(glm::vec3 position = { 0.0f, 0.0f, 0.0f }, glm::vec3 up = { 0.0f, 1.0f, 0.0f }, float yaw = s_yaw, float pitch = s_pitch);
    ~Camera();
    void processKeyboard(MovementType direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    inline glm::mat4 viewMatrix() { return glm::lookAt(position, position + front, up); }

private:
    void updateCameraVectors();

public:
    static constexpr const float s_yaw = -90.0f;
    static constexpr const float s_pitch = 0.0f;
    static constexpr const float s_speed = 2.5f;
    static constexpr const float s_sensitivity = 0.1f;
    static constexpr const float s_zoom = 45.0f;

public:
    // camera Attributes
    glm::vec3 position{ 0.0f };
    glm::vec3 front{ 0.0f };
    glm::vec3 up{ 0.0f };
    glm::vec3 right{ 0.0f };
    glm::vec3 worldUp{ 0.0f };
    // euler Angles
    float yaw{};
    float pitch{};
    // camera options
    float movementSpeed{};
    float mouseSensitivity{};
    float zoom{};
};
} // namespace backend

#endif // LEARNMETALVULKAN_CAMERA_H
