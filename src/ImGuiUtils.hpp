#pragma once
#include "imgui.h"
#include <cstdint>
#include <string>

namespace Colors
{
    constexpr uint32_t Back = 0xff828078;
    constexpr uint32_t Secondary = 0xff7f6352;

    constexpr uint32_t CreateNew = 0xff8e5b52;
    constexpr uint32_t Save = 0xff7b7c17;

    constexpr uint32_t Background = 0xff33281f;
    constexpr uint32_t Header = 0xff724427;

    constexpr uint32_t InfoText = 0xffbedb1a;
    constexpr uint32_t ErrorText = 0xff2e48c9;

    constexpr uint32_t RequiredField = 0xff2e4899;

    inline ImVec4 AsVec(uint32_t hex)
    {
        ImVec4 vec;
        vec.w = (hex >> 24 & 0xff) / 255.f;
        vec.z = (hex >> 16 & 0xff) / 255.f;
        vec.y = (hex >> 8 & 0xff) / 255.f;
        vec.x = (hex >> 0 & 0xff) / 255.f;
        return vec;
    }
} // namespace Colors

namespace Fonts
{
    inline ImFont* body;
    inline ImFont* header;

    inline ImFont* logo;
} // namespace Fonts

#include "imgui_gl/utils.hpp"

namespace ImGui
{
    inline void TextInputRequired(const char* label, std::string* text)
    {
        if (text->empty())
        {
            ScopedStyle bgstyle(ImGuiCol_FrameBg, Colors::RequiredField);
            ScopedStyle textstyle(ImGuiCol_TextDisabled, 0xccffffff);
            ImGui::InputTextWithHint(label, "Required!", text);
        }
        else
            ImGui::InputText(label, text);
    }
} // namespace ImGui