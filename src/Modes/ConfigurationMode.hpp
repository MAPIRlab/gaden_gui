#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Mode.hpp"
#include "Utils.hpp"
#include "gaden/EnvironmentConfigMetadata.hpp"
#include "gaden/Preprocessing.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <regex>

class ConfigurationMode : public Mode
{
public:
    ConfigurationMode(gaden::EnvironmentConfigMetadata& metadata)
        : configMetadata(metadata) {}

    void OnPush() override
    {
        config = gaden::EnvironmentConfiguration::ReadDirectory(configMetadata.rootDirectory);
    }

    void OnPop() override
    {}

    void OnGainFocus() override
    {}

    void OnLoseFocus() override
    {}

    void OnGUI() override
    {
        ImGui::Text("Project: '%s'", g_app->project->GetRoot().c_str());
        ImGui::Text("%s", fmt::format("Editing configuration '{}'", configMetadata.GetName()).c_str());
        ImGui::VerticalSpace(10);

        ImGui::InputFloat("Cell size", &configMetadata.cellSize, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat3("Empty point", &configMetadata.emptyPoint.x, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::Checkbox("Uniform Wind", &configMetadata.uniformWind);
        ImGui::InputText("Unprocessed Wind Files", &configMetadata.unprocessedWindFiles);
        ImGui::SameLine();
        if (ImGui::Button("Find"))
        {
            std::filesystem::path path = Utils::FileDialog("OpenFOAM vector clouds | *.csv", g_app->project->rootDirectory / "");
            std::cmatch m;
            if (std::regex_match(path.c_str(), m, std::regex("(.*)_\\d+.csv")))
                configMetadata.unprocessedWindFiles = m[1];
            else
                Utils::DisplayError("Selected file does not have the correct name pattern (ending in _[i].csv)");
        }
        ImGui::VerticalSpace(20);

        //------------------------
        if (ImGui::CollapsingHeader("Obstacle Models"))
        {
            ModelsList(configMetadata.envModels, "Obstacle Models");
            ImGui::VerticalSpace(20);
        }
        if (ImGui::CollapsingHeader("Outlet Models"))
            ModelsList(configMetadata.outletModels, "Outlet Models");

        //------------------------

        ImGui::VerticalSpace(30);
        if (config)
            ImGui::TextColored(ImVec4(0, 0.7, 0.6, 1), "Preprocessed data available");
        else
            ImGui::TextColored(ImVec4(0.7, 0.2, 0.1, 1), "No preprocessed data available");

        //------------------------
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Save);
        if (ImGui::Button("Save Changes"))
        {
            configMetadata.WriteConfigYAML();
        }

        //------------------------
        ImGui::SameLine();
        if (ImGui::Button("Run Preprocessing"))
        {
            config = gaden::Preprocessing::Preprocess(configMetadata);
            if (config && config->WriteToDirectory(configMetadata.rootDirectory))
            {
                Utils::DisplayInfo("Preprocessing completed!");
            }
            else
                Utils::DisplayError("Preprocessing failed! See logs for more info");
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(!config);
        if (ImGui::Button("Go to 'Simulations'"))
        {
        }
        ImGui::EndDisabled();
        ImGui::PopStyleColor();

        //------------------------
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Back);
        if (ImGui::Button("Back"))
        {
            g_app->PopMode();
        }
        ImGui::PopStyleColor();
    }

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

private:
    gaden::EnvironmentConfigMetadata& configMetadata;
    std::optional<gaden::EnvironmentConfiguration> config;
};