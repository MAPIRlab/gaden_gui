#pragma once

#include "glm/vec3.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct Transform
{
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::quat rotation = glm::quat(1, 0, 0, 0);

    glm::vec3 forward() const
    {
        return rotation * glm::vec3(0, 0, -1);
    }

    glm::vec3 up() const
    {
        return rotation * glm::vec3(0, 1, 0);
    }

    glm::vec3 right() const
    {
        return rotation * glm::vec3(1, 0, 0);
    }

    glm::mat4 GetTransformMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::toMat4(rotation) * model;
        return model;
    }
};