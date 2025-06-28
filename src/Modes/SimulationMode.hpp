#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Modes/ConfigurationMode.hpp"
#include "Modes/Mode.hpp"
#include "Utils.hpp"
#include "gaden/internal/Pointers.hpp"
#include "imgui.h"
#include <atomic>
#include <gaden/RunningSimulation.hpp>
#include <gaden/internal/Time.hpp>
#include <thread>

#include "gaden/datatypes/sources/BoxSource.hpp"
#include "gaden/datatypes/sources/LineSource.hpp"
#include "gaden/datatypes/sources/SphereSource.hpp"

class SimulationMode : public Mode
{
public:
    SimulationMode(ConfigurationMode& _configMode,
                   gaden::RunningSimulation::Parameters& _params,
                   std::string_view _name)
        : configMode(_configMode), params(_params), name(_name)
    {
        params.saveDataDirectory = configMode.configMetadata.GetSimulationFilePath(name).parent_path() / "result";
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
        g_app->vizScene->filamentsViz.CleanUp();
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
            // GasType gasType = GasType::unknown; //TODO
            static int type_selected = std::find(sourceTypeNames.begin(), sourceTypeNames.end(), params.source->Type()) - sourceTypeNames.begin();
            ImGui::Combo("Source type", &type_selected, ConcatenatedNames().c_str());
            {
                if (type_selected == 0)
                {
                    if (!Is<gaden::PointSource>(params.source))
                        params.source = std::make_shared<gaden::PointSource>();
                    g_app->vizScene->DrawSphere(params.source->sourcePosition, 0.1f);
                }
                else if (type_selected == 1)
                {
                    if (!Is<gaden::SphereSource>(params.source))
                        params.source = std::make_shared<gaden::SphereSource>();
                    float r = As<gaden::SphereSource>(params.source)->GetRadius();
                    ImGui::DragFloat("Source radius", &r, 0.05f);
                    As<gaden::SphereSource>(params.source)->SetRadius(r);
                    g_app->vizScene->DrawSphere(params.source->sourcePosition, As<gaden::SphereSource>(params.source)->GetRadius());
                }
                else if (type_selected == 2)
                {
                    if (!Is<gaden::BoxSource>(params.source))
                        params.source = std::make_shared<gaden::BoxSource>();
                    ImGui::DragFloat3("Source size", &As<gaden::BoxSource>(params.source)->size.x, 0.05f);
                    g_app->vizScene->DrawCube(params.source->sourcePosition, As<gaden::BoxSource>(params.source)->size);
                }
                else if (type_selected == 3)
                {
                    if (!Is<gaden::LineSource>(params.source))
                        params.source = std::make_shared<gaden::LineSource>();
                    ImGui::DragFloat3("Line end", &As<gaden::LineSource>(params.source)->lineEnd.x, 0.05f);
                    g_app->vizScene->DrawLine(params.source->sourcePosition, As<gaden::LineSource>(params.source)->lineEnd, 0.02f);
                }
            }

            ImGui::DragFloat3("Source Position", &params.source->sourcePosition.x, 0.05f, 0.0f, 0.0f, "%.2f");
            ImGui::SameLine();
            if (ImGui::Button("View in scene"))
                ToggleSceneView();

            ImGui::VerticalSpace(10);

            // clang-format off
            ImGui::InputFloat("Delta Time",             &params.deltaTime); 
            ImGui::InputFloat("Wind iteration deltaT",  &params.windIterationDeltaTime); 
            ImGui::InputFloat("Temperature (K)",        &params.temperature);                       
            ImGui::InputFloat("Pressure (atm)",         &params.pressure);                             
            ImGui::InputFloat("Filament_ppm_center",    &params.filamentPPMcenter);       
            ImGui::InputFloat("Filament_initial_sigma", &params.filamentInitialSigma); 
            ImGui::InputFloat("Filament_growth_gamma",  &params.filamentGrowthGamma);   
            ImGui::InputFloat("Filament_noise_std",     &params.filamentNoise_std);                          
            ImGui::InputFloat("Filaments/sec",          &params.numFilaments_sec);
            ImGui::InputFloat("Simulation Duration",    &simDuration);
            
            ImGui::VerticalSpace(10);
            ImGui::Checkbox("Loop wind", &params.windLoop.loop);
            ImGui::BeginDisabled(!params.windLoop.loop);
            ImGui::InputScalar("Loop from", ImGuiDataType_U64, &params.windLoop.from, NULL);
            ImGui::InputScalar("Loop to", ImGuiDataType_U64, &params.windLoop.to, NULL);
            ImGui::EndDisabled();
            
            params.saveResults = true; // it never makes sense to run simulation through this GUI and *not* save results            
            ImGui::InputFloat("Save deltaT",    &params.saveDeltaTime);

            ImGui::VerticalSpace(10);
            ImGui::Checkbox("Limit simulation rate", &limitRate);
            ImGui::SameLine();
            ImGui::HelpMarker("Make the simulation run slower for easier visualization");
            ImGui::BeginDisabled(!limitRate);    
            {
                ImGui::InputFloat("Simulation rate", &rateLimit);
                ImGui::SameLine();
                if(ImGui::Button("Match deltaT"))
                rateLimit = 1 / params.deltaTime;
            }        
            ImGui::EndDisabled();
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
        else if (!configMode.config)
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
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Save);
            {
                // Save
                //------------------------
                if (ImGui::Button("Save Changes"))
                {
                    params.WriteToYAML(configMode.configMetadata.GetSimulationFilePath(name));
                }

                // Run
                //------------------------
                ImGui::SameLine();

                ImGui::BeginDisabled(!configMode.config); // need a valid environment configuration to run the simulation
                if (ImGui::Button("Run Simulation"))
                {
                    simDone = false;
                    if (runSimThread)
                        runSimThread->join();

                    runSimThread.emplace([this]()
                                         {
                                             Run();
                                         });
                }
                ImGui::EndDisabled();
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();

        // Open view button
        //----------------
        {
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Save);
            ImGui::SameLine();
            if (ImGui::Button("Toggle Scene View"))
                ToggleSceneView();
            ImGui::SameLine();
            ImGui::HelpMarker("Open a view of the scene where you can see the filaments moving.\nThis has a performance impact on the simulation.");
            ImGui::PopStyleColor();
        }

        // stop button
        //----------------
        ImGui::BeginDisabled(!runSimThread.has_value());
        {
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Back);
            ImGui::SameLine();
            if (ImGui::Button("Stop Simulation"))
                canRun = false;
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();

        // Back
        //------------------------
        ImGui::BeginDisabled(runSimThread.has_value());
        {
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Back);
            {
                if (ImGui::Button("Back"))
                    g_app->PopMode();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndDisabled();
    }

private:
    void Run()
    {
        try
        {
            currentSimTime = 0;
            gaden::RunningSimulation sim(params, *configMode.config);

            float r = 0;
            if (limitRate)
                r = rateLimit;
            gaden::Utils::Time::Rate rate(r);

            while (sim.GetCurrentTime() < simDuration && canRun)
            {
                sim.AdvanceTimestep();
                currentSimTime = sim.GetCurrentTime();

                // this is not very efficient, because we are potentially copying the filament positions multiple times on a single frame, when only the last copy matters
                // however, avoiding that would be a bit of a mess
                //  if speed is a concern you can always run the simulation without visualization
                if (g_app->vizScene->active)
                    g_app->vizScene->filamentsViz.SetFilaments(sim.GetFilaments());

                if (r != 0)
                    rate.sleep();
            }
            Utils::DisplayInfo("Simulation completed!");
            canRun = true;
        }
        catch (std::exception const& e)
        {
            Utils::DisplayError("Simulation failed! See logs for more info");
        }

        simDone = true;
    }

    void ToggleSceneView()
    {
        if (!g_app->vizScene->active)
            configMode.CreateScene();
        else
            g_app->vizScene->active = false;
    }

private:
    ConfigurationMode& configMode;
    gaden::RunningSimulation::Parameters& params;
    std::string name;
    float simDuration = 300.f;
    bool limitRate = false;
    float rateLimit = 0;
    bool canRun = true;

    std::optional<std::thread> runSimThread;
    std::atomic<float> currentSimTime;
    bool simDone = false;

    const std::vector<std::string> sourceTypeNames =
        {
            "point",
            "sphere",
            "box",
            "line",
        };

    std::string ConcatenatedNames()
    {
        std::string s = "";
        for (size_t i = 0; i < sourceTypeNames.size(); i++)
            s += sourceTypeNames.at(i) + '\0';
        return s;
    }
};