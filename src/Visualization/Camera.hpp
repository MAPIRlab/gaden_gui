#pragma once
#include "Visualization/Transform.hpp"
#include "imgui.h"

struct Camera
{
    Transform transform;
    float movementSpeed = 4.f;
    float rotationSpeed = 0.7f;

    Camera();

    void HandleInput(float deltaTime);

    glm::mat4 GetViewMatrix()
    {
        return transform.GetTransformMatrix();
    }

private:
    ImVec2 previousMouseDelta = ImVec2(0, 0);
};