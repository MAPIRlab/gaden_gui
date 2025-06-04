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
        ImGui::Text("I'm in default mode!");
        if (ImGui::Button("Create New Gaden Project"))
        {
            auto path = Utils::DirectoryDialog();
            auto project = std::make_shared<Project>(path);
            project->CreateTemplate();

            g_app->PushMode(std::make_shared<ProjectMode>(project));
        }

        if (ImGui::Button("Load Existing Gaden Project"))
        {
            auto path = Utils::DirectoryDialog();
            auto project = std::make_shared<Project>(path);
            project->ReadDirectory();

            g_app->PushMode(std::make_shared<ProjectMode>(project));
        }

        if (ImGui::Button("Exit Gaden"))
        {
            g_app->shouldClose = true;
        }
    }
};