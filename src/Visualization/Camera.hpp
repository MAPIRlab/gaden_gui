#pragma once
#include "glm/gtc/quaternion.hpp"
#include "glm/vec3.hpp"
#include "imgui.h"

struct Camera
{
    glm::vec3 position = glm::vec3(0, 1, 0);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    float movementSpeed = 4.f;
    float rotationSpeed = 1.f;

    Camera()
    {
        // look towards (0,0,0)
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraDirection = glm::normalize(position - cameraTarget);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(up, cameraDirection)) > 0.99f)
            up = glm::vec3(0, 0, 1);
        glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

        // rotation = glm::lookAt(position, cameraTarget, cameraUp);
    }

    void HandleInput(float deltaTime);
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + forward(), -up());
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
        return rotation * glm::vec3(-1, 0, 0);
    }

private:
    ImVec2 previousMouseDelta = ImVec2(0, 0);
};