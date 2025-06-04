#pragma once
#include "Application.hpp"
#include "Mode.hpp"
#include "Utils.hpp"
#include "gaden/EnvironmentConfigMetadata.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

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
        ImGui::Text("Editing configuration");

        ImGui::InputFloat("Cell size", &config.cellSize);
        ImGui::InputFloat3("Empty point", &config.emptyPoint.x);
        ImGui::Checkbox("Uniform Wind", &config.uniformWind);
        ImGui::InputText("Unprocessed Wind Files", &config.unprocessedWindFiles);

        ModelsList(config.envModels, "Obstacle Models");
        ModelsList(config.outletModels, "Outlet Models");

        if (ImGui::Button("Save Changes"))
        {
            config.WriteConfigYAML();
        }

        if (ImGui::Button("Back"))
        {
            g_app->PopMode();
        }
    }

private:
    void ModelsList(std::vector<gaden::Model3D>& models, const char* label)
    {
        ImGui::PushID(label);
        ImGui::Text("%s", label);
        for (size_t i = 0; i < models.size(); i++)
        {
            ImGui::PushID(i);
            gaden::Model3D& model = models.at(i);

            // path
            std::string temp = model.path.string();
            ImGui::InputText("Path", &temp);
            ImGui::SameLine();
            if (ImGui::Button("Find"))
                temp = Utils::FileDialog("STL models | *.stl");
            model.path = temp;

            // color
            if (i > 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("Copy previous color"))
                    model.color = models.at(i - 1).color;
            }
            ImGui::ColorEdit4("color", &model.color.r, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float);
            ImGui::PopID();
        }

        if (ImGui::Button("Add model"))
            models.push_back({.path = ""});
        ImGui::PopID();
    }

private:
    gaden::EnvironmentConfigMetadata config;
};