#pragma once
#include "Application.hpp"
#include "Mode.hpp"
#include "gaden/EnvironmentConfigMetadata.hpp"
#include "imgui.h"

class ConfigurationMode : public Mode
{
public:
    ConfigurationMode(gaden::EnvironmentConfigMetadata const& metadata)
        : config(metadata) {}
        
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
        ImGui::Text("I'm in configuration mode!");

        if (ImGui::Button("Save Changes"))
        {
            config.CreateTemplate();
        }

        if (ImGui::Button("Back"))
        {
            g_app->PopMode();
        }
    }

private:
    gaden::EnvironmentConfigMetadata config;
};