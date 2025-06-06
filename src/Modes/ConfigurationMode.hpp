#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Mode.hpp"
#include "Utils.hpp"
#include "gaden/EnvironmentConfigMetadata.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

class ConfigurationMode : public Mode
{
public:
    ConfigurationMode(gaden::EnvironmentConfigMetadata& metadata)
        : configMetadata(metadata)
    {
        config = gaden::EnvironmentConfiguration::ReadDirectory(configMetadata.rootDirectory);
    }

    void OnPush() override
    {
    }

    void OnPop() override
    {}

    void OnGainFocus() override
    {}

    void OnLoseFocus() override
    {}

    void OnGUI() override;

private:
    void ModelsList(std::vector<gaden::Model3D>& models, const char* label)
    {
        ImGui::PushID(label);
        // ImGui::Text("%s", label);
        for (size_t i = 0; i < models.size(); i++)
        {
            ImGui::PushID(i);
            gaden::Model3D& model = models.at(i);

            // path
            std::string temp = model.path.string();
            ImGui::InputText("Path", &temp);
            ImGui::SameLine();
            if (ImGui::Button("Find"))
                temp = Utils::FileDialog("STL models | *.stl", model.path);
            model.path = temp;
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                models.erase(models.begin() + i);
                i--;
                ImGui::PopID();
                continue;
            }

            // color
            ImGui::SetNextItemWidth(200);
            ImGui::ColorEdit4("color", &model.color.r, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float);
            if (i > 0)
            {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, Colors::Secondary);
                if (ImGui::Button("Copy previous color"))
                    model.color = models.at(i - 1).color;
                ImGui::PopStyleColor();
            }
            ImGui::PopID();
        }

        if (ImGui::Button("Add models"))
        {
            auto paths = Utils::MultiFileDialog("STL models | *.stl", g_app->project->rootDirectory / "");
            for (auto& path : paths)
                models.push_back({.path = path});
        }
        ImGui::PopID();
    }

public:
    gaden::EnvironmentConfigMetadata& configMetadata;
    std::optional<gaden::EnvironmentConfiguration> config;
};