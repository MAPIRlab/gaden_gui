#include "ConfigurationMode.hpp"
#include "SimulationListMode.hpp"
#include "gaden/Preprocessing.hpp"
#include <regex>

void ConfigurationMode::OnGUI()
{
    ImGui::Text("Project: '%s'", g_app->project->GetRoot().c_str());
    ImGui::Text("%s", fmt::format("Editing configuration '{}'", configMetadata.GetName()).c_str());
    ImGui::Separator();
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
        ImGui::TextColored(Colors::AsVec(Colors::InfoText), "Preprocessed data available");
    else
        ImGui::TextColored(Colors::AsVec(Colors::ErrorText), "No preprocessed data available");

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
        g_app->PushMode(std::make_shared<SimulationListMode>(*this));
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
