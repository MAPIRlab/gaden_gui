#pragma once
#include "gaden/core/Logging.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/vec3.hpp"
#include "imgui.h"

struct Camera
{
    glm::vec3 position = glm::vec3(5, 5, 5);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    float movementSpeed = 4.f;
    float rotationSpeed = 1.f;

    Camera();

    void HandleInput(float deltaTime);
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + forward(), up());
    }

    glm::vec3 forward()
    {
        return rotation * glm::vec3(0, 0, -1);
    }

    glm::vec3 up()
    {
        return rotation * glm::vec3(0, 1, 0);
    }

    glm::vec3 right()
    {
        return rotation * glm::vec3(1, 0, 0);
    }

private:
    ImVec2 previousMouseDelta = ImVec2(0, 0);
};