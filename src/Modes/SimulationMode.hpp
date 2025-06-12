#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Modes/ConfigurationMode.hpp"
#include "Modes/Mode.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include <atomic>
#include <gaden/RunningSimulation.hpp>
#include <thread>

class SimulationMode : public Mode
{
public:
    SimulationMode(ConfigurationMode& _configMode,
                   gaden::RunningSimulation::Parameters& _params,
                   std::string_view _name)
        : configMode(_configMode), params(_params), name(_name)
    {
        params.saveDataDirectory = configMode.configMetadata.GetSimulationFilePath(name).parent_path();
    }

    void OnPush() override
    {}

    void OnPop() override
    {}

    void OnGainFocus() override
    {}

    void OnLoseFocus() override
    {
        if (configMode.scene)
            configMode.scene->active = false;
    }

    void OnGUI() override
    {
        ImGui::Text("Project: '%s'", g_app->project->GetRoot().c_str());
        ImGui::Text("%s", fmt::format("Configuration '{}'", configMode.configMetadata.GetName()).c_str());
        ImGui::Text("%s", fmt::format("Editing simulation '{}'", name).c_str());
        ImGui::Separator();
        ImGui::VerticalSpace(10);

        ImGui::BeginDisabled(runSimThread.has_value());
        {
            // clang-format off
            // GasType gasType = GasType::unknown;
            ImGui::DragFloat3("Source Position",        &params.sourcePosition.x, 0.05f, 0.0f, 0.0f, "%.2f");
            ImGui::SameLine();
            if (ImGui::Button("View in scene"))
                configMode.CreateScene();
            if (configMode.scene && configMode.scene->active)
                configMode.scene->Render(params.sourcePosition);

            ImGui::InputFloat("Delta Time",             &params.deltaTime); 
            ImGui::InputFloat("Wind iteration deltaT",  &params.windIterationDeltaTime); 
            ImGui::InputFloat("Temperature (K)",        &params.temperature);                       
            ImGui::InputFloat("Pressure (atm)",         &params.pressure);                             
            ImGui::InputFloat("Filament_ppm_center",    &params.filament_ppm_center);       
            ImGui::InputFloat("Filament_initial_sigma", &params.filament_initial_sigma); 
            ImGui::InputFloat("Filament_growth_gamma",  &params.filament_growth_gamma);   
            ImGui::InputFloat("Filament_noise_std",     &params.filament_noise_std);                          
            ImGui::InputFloat("Filaments/sec",          &params.numFilaments_sec);
            ImGui::InputFloat("Simulation Duration",    &simDuration);
            
            ImGui::Checkbox("Loop wind", &params.windLoop.loop);
            ImGui::BeginDisabled(!params.windLoop.loop);
            ImGui::InputScalar("Loop from", ImGuiDataType_U64, &params.windLoop.from, NULL);
            ImGui::InputScalar("Loop to", ImGuiDataType_U64, &params.windLoop.to, NULL);
            ImGui::EndDisabled();
            
            params.saveResults = true; // it never makes sense to run simulation through this GUI and *not* save results            
            ImGui::InputFloat("Save deltaT",    &params.saveDeltaTime);
            // clang-format on
        }
        ImGui::EndDisabled();

        ImGui::VerticalSpace(20);

        if (runSimThread)
        {
            ImVec4 color = Colors::AsVec(Colors::InfoText);
            ImGui::TextColored(color, "Simulation running...");
            ImGui::TextColored(color, "Progress: %.2f/%.2fs", (float)currentSimTime, simDuration);
            if (simDone)
            {
                runSimThread->join();
                runSimThread = std::nullopt;
            }
        }
        else if(!configMode.config)
        {
            ImGui::TextColored(Colors::AsVec(Colors::ErrorText), "Need to run preprocessing before simulations!");
            ImGui::Text("");
        }
        else
        {
            // keep the same spacing
            ImGui::Text("");
            ImGui::Text("");
        }

        ImGui::BeginDisabled(runSimThread.has_value());
        {
            //------------------------
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Save);
            {
                if (ImGui::Button("Save Changes"))
                {
                    params.WriteToYAML(configMode.configMetadata.GetSimulationFilePath(name));
                }

                //------------------------
                ImGui::SameLine();

                ImGui::BeginDisabled(!configMode.config);
                if (ImGui::Button("Run Simulation"))
                {
                    simDone = false;
                    runSimThread.emplace([this]()
                                         {
                                             Run();
                                         });
                }
                ImGui::EndDisabled();
            }
            ImGui::PopStyleColor();
        }

        //------------------------
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Back);
        if (ImGui::Button("Back"))
        {
            g_app->PopMode();
        }
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
    }

private:
    void Run()
    {
        try
        {
            currentSimTime = 0;
            gaden::RunningSimulation sim(params, *configMode.config);
            while (sim.GetCurrentTime() < simDuration)
            {
                sim.AdvanceTimestep();
                currentSimTime = sim.GetCurrentTime();
            }
            Utils::DisplayInfo("Simulation completed!");
        }
        catch (std::exception const& e)
        {
            Utils::DisplayError("Simulation failed! See logs for more info");
        }

        simDone = true;
    }

private:
    ConfigurationMode& configMode;
    gaden::RunningSimulation::Parameters& params;
    std::string name;
    float simDuration = 300.f;

    std::optional<std::thread> runSimThread;
    std::atomic<float> currentSimTime;
    bool simDone = false;
};