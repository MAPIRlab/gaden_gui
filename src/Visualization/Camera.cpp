#include "Camera.hpp"
#include "gaden/core/Logging.hpp"
#include "gaden/core/Vectors.hpp"
#include "imgui.h"

void Camera::HandleInput(float deltaTime)
{
    if (!ImGui::IsWindowFocused())
        return;

    float boost = 1.f;
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        boost = 3.f;

    if (ImGui::IsKeyDown(ImGuiKey_W))
        position += forward() * movementSpeed * boost * deltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_S))
        position += -forward() * movementSpeed * boost * deltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_A))
        position += -right() * movementSpeed * boost * deltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_D))
        position += right() * movementSpeed * boost * deltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_Q))
        position += up() * movementSpeed * boost * deltaTime;
    if (ImGui::IsKeyDown(ImGuiKey_E))
        position += -up() * movementSpeed * boost * deltaTime;

    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    //smoothing
    mouseDelta.x = std::lerp(previousMouseDelta.x, mouseDelta.x, 0.4);
    mouseDelta.y = std::lerp(previousMouseDelta.y, mouseDelta.y, 0.4);
    previousMouseDelta = mouseDelta;

    glm::quat pitchQuat(glm::vec3(-mouseDelta.y * rotationSpeed * deltaTime, 0, 0));
    glm::quat yawQuat(glm::vec3(0, mouseDelta.x * rotationSpeed * deltaTime, 0));

    rotation = yawQuat * rotation * pitchQuat;

    GADEN_INFO("Position:{}", position);
    GADEN_INFO("Forward:{}", forward());
    GADEN_INFO("Up:{}", up());
}
