#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Modes/ConfigurationMode.hpp"
#include "Modes/RunSimMode.hpp"
#include "SimulationInfo.hpp"
#include "Utils.hpp"
#include "gaden/Scene.hpp"
#include <gaden/internal/Time.hpp>

class SceneMode : public RunSimMode
{
public:
    SceneMode(ConfigurationMode& configMode, std::string const& name)
        : RunSimMode(configMode), sceneName(name)
    {
        metadata = configMode.configMetadata.GetRunningScene(name);
        std::vector simNames = configMode.configMetadata.GetSimulationNamesInScene(name);
        for (size_t i = 0; i < metadata.params.size(); i++)
            simInfos.emplace_back(metadata.params.at(i), configMode.configMetadata, simNames.at(i));
    }

    void OnPush() override
    {}

    void OnPop() override
    {}

    void OnGainFocus() override
    {}

    void OnLoseFocus() override
    {
        g_app->vizScene->active = false;
        g_app->vizScene->filamentsViz.Clear();
    }

    void OnGUI() override
    {
        ImGui::Text("Project: '%s'", g_app->project->GetRoot().c_str());
        ImGui::Text("%s", fmt::format("Configuration '{}'", configMode.configMetadata.GetName()).c_str());
        ImGui::Text("%s", fmt::format("Editing scene '{}'", sceneName).c_str());
        ImGui::VerticalSpace(10);
        ImGui::Separator();

        for (size_t i = 0; i < simInfos.size(); i++)
        {
            auto& simInfo = simInfos.at(i);
            if (ImGui::CollapsingHeader(simInfo.name.c_str()))
            {
                simInfo.DrawGUI(i == 0);
                {
                    ImGui::ScopedStyle style(ImGuiCol_Button, Colors::Save);
                    if (ImGui::Button("Save Changes"))
                        simInfo.params.WriteToYAML(simInfo.configMetadata.GetSimulationFilePath(simInfo.name));
                }
            }
        }

        if (!simInfos.empty())
            DrawTimeControlsGUI(simInfos.at(0).params.deltaTime);
        DrawSimulationStateGUI();
        DrawButtonsGUI();
    }

protected:
    void Run() override
    {
        try
        {
            currentSimTime = 0;
            metadata.params.clear();
            metadata.gasDisplayColors.clear();
            
            for (auto const& sim : simInfos)
            {
                metadata.params.push_back(sim.params);
                metadata.gasDisplayColors.push_back(sim.displayColor);
            }

            gaden::Scene scene(metadata, configMode.config);

            float r = 0;
            if (isRateLimited)
                r = rateLimit;
            gaden::Utils::Time::Rate rate(r);

            auto simulations = scene.GetSimulations();
            auto firstSim = As<gaden::RunningSimulation>(simulations.at(0));
            while (firstSim->GetCurrentTime() < simDuration && canRun)
            {
                scene.AdvanceTimestep();
                currentSimTime = firstSim->GetCurrentTime();

                // this is not very efficient, because we are potentially copying the filament positions multiple times on a single frame, when only the last copy matters
                // however, avoiding that would be a bit of a mess
                //  if speed is a concern you can always run the simulation without visualization
                if (g_app->vizScene->active)
                {
                    g_app->vizScene->filamentsViz.Clear();
                    for (size_t i = 0; i < scene.GetSimulations().size(); i++)
                        g_app->vizScene->filamentsViz.DrawFilaments(simulations.at(i)->GetFilaments(), scene.GetColors().at(i));
                }

                if (r != 0)
                    rate.sleep();
            }

            if (firstSim->GetCurrentTime() >= simDuration)
                Utils::DisplayInfo("Simulation completed!");

            canRun = true;
        }
        catch (std::exception const& e)
        {
            GADEN_ERROR("Caught exception while running simulation: '{}'", e.what());
            Utils::DisplayError("Simulation failed! See logs for more info");
        }

        simDone = true;
    }

private:
    std::vector<SimulationInfo> simInfos;
    std::string sceneName;
    gaden::RunningSceneMetadata metadata;
};