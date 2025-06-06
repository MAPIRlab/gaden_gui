#pragma once
#include "imgui.h"
#include <cstdint>

namespace Colors
{
    constexpr uint32_t Back = 0xff828078; 
    constexpr uint32_t Secondary = 0xff7f6352;

    constexpr uint32_t CreateNew = 0xff8e5b52; 
    constexpr uint32_t Save = 0xff7b7c17; 
}

namespace ImGui
{
    inline bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
        float avail = ImGui::GetContentRegionAvail().x;

        float off = (avail - size) * alignment;
        if (off > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        return ImGui::Button(label);
    }

    inline void HorizontalSpace(float space)
    {
        ImGui::Dummy(ImVec2(space, 0.f));
        ImGui::SameLine();
    }

    inline void VerticalSpace(float space)
    {
        ImGui::Dummy(ImVec2(0.f, space));
    }

} // namespace ImGui