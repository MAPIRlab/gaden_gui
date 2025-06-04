#pragma once
#include "Application.hpp"
#include "Mode.hpp"
#include "Modes/ConfigurationMode.hpp"
#include "Project.hpp"
#include "Utils.hpp"
#include "imgui.h"

class ProjectMode : public Mode
{
public:
    ProjectMode(std::shared_ptr<Project> _project)
        : project(_project)
    {}

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
        ImGui::Text("Project: '%s'", project->GetRoot().c_str());

        for (gaden::EnvironmentConfigMetadata& config : project->configurations)
        {
            std::string buttonText = fmt::format("Open configuration {}", config.GetName());
            if (ImGui::Button(buttonText.c_str()))
            {
                g_app->PushMode(std::make_shared<ConfigurationMode>(config));
            }
        }

        if (ImGui::Button("Create new configuration"))
        {
            std::string name = Utils::TextInput("Create new configuration", "Configuration name");
            if (name == "")
            {
                GADEN_ERROR("Invalid configuration name. Using 'config' instead");
                name = "config";
            }
            gaden::EnvironmentConfigMetadata config(project->GetConfigurationPath(name));
            config.CreateTemplate();

            g_app->PushMode(std::make_shared<ConfigurationMode>(config));
        }

        if (ImGui::Button("Back"))
        {
            g_app->PopMode();
        }
    }

private:
    std::shared_ptr<Project> project;
};