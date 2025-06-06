#pragma once
#include "Application.hpp"
#include "Mode.hpp"
#include "Project.hpp"
#include "ProjectMode.hpp"
#include "Utils.hpp"
#include "imgui.h"

class DefaultMode : public Mode
{
    void OnPush() override
    {}

    void OnPop() override
    {}

    void OnGainFocus() override
    {}

    void OnLoseFocus() override
    {}

    void OnGUI() override
    {
        ImGui::VerticalSpace(50);
        ImGui::PushFont(Fonts::logo);
        ImGui::TextCenteredOnLine("gaden");
        ImGui::PopFont();

        ImGui::VerticalSpace(105);
        ImGui::Separator();

        if (ImGui::ButtonCenteredOnLine("Load Existing Gaden Project"))
        {
            auto path = Utils::DirectoryDialog();
            if (path.is_absolute() && std::filesystem::exists(path))
            {
                auto project = std::make_shared<Project>(path);
                if (project->ReadDirectory())
                    g_app->PushMode(std::make_shared<ProjectMode>(project));
                else
                    Utils::DisplayError(fmt::format("'{}' is not the path of a valid gaden project", path));
            }
        }

        ImGui::PushStyleColor(ImGuiCol_Button, Colors::CreateNew);
        if (ImGui::ButtonCenteredOnLine("Create New Gaden Project"))
        {
            auto path = Utils::DirectoryDialog();
            if (path.is_absolute() && std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                auto project = std::make_shared<Project>(path);
                project->CreateTemplate();
                project->ReadDirectory();

                g_app->PushMode(std::make_shared<ProjectMode>(project));
            }
            else
                Utils::DisplayError(fmt::format("Directory '{}' does not exist", path));
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Back);
        if (ImGui::ButtonCenteredOnLine("Exit GadenGUI"))
        {
            g_app->shouldClose = true;
        }
        ImGui::PopStyleColor();
    }
};