#pragma once
#include "Application.hpp"
#include "ImGuiUtils.hpp"
#include "Modes/ConfigurationMode.hpp"
#include "Modes/Mode.hpp"
#include "Utils.hpp"
#include "gaden/datatypes/sources/CylinderSource.hpp"
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

        sourceTypeSelected = std::find(gaden::GasSource::sourceTypeNames.begin(), gaden::GasSource::sourceTypeNames.end(), params.source->Type()) - gaden::GasSource::sourceTypeNames.begin();
        gasType = params.source->gasType;
        sourcePosition = params.source->sourcePosition;
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
        ImGui::VerticalSpace(10);
        ImGui::Separator();

        ImGui::BeginDisabled(runSimThread.has_value());
        {
            ImGui::Combo("Source type", &sourceTypeSelected, ConcatenatedNames().c_str());
            {
                if (gaden::GasSource::sourceTypeNames.at(sourceTypeSelected) == "point")
                {
                    if (!Is<gaden::PointSource>(params.source))
                        params.source = std::make_shared<gaden::PointSource>();
                }
                else if (gaden::GasSource::sourceTypeNames.at(sourceTypeSelected) == "sphere")
                {
                    if (!Is<gaden::SphereSource>(params.source))
                        params.source = std::make_shared<gaden::SphereSource>();
                    ImGui::DragFloat("Source radius", &As<gaden::SphereSource>(params.source)->radius, 0.05f);
                }
                else if (gaden::GasSource::sourceTypeNames.at(sourceTypeSelected) == "box")
                {
                    if (!Is<gaden::BoxSource>(params.source))
                        params.source = std::make_shared<gaden::BoxSource>();
                    ImGui::DragFloat3("Source size", &As<gaden::BoxSource>(params.source)->size.x, 0.05f);
                }
                else if (gaden::GasSource::sourceTypeNames.at(sourceTypeSelected) == "line")
                {
                    if (!Is<gaden::LineSource>(params.source))
                        params.source = std::make_shared<gaden::LineSource>();
                    ImGui::DragFloat3("Line end", &As<gaden::LineSource>(params.source)->lineEnd.x, 0.05f);
                }
                else if (gaden::GasSource::sourceTypeNames.at(sourceTypeSelected) == "cylinder")
                {
                    if (!Is<gaden::CylinderSource>(params.source))
                        params.source = std::make_shared<gaden::CylinderSource>();
                    ImGui::DragFloat("Source Radius", &As<gaden::CylinderSource>(params.source)->radius, 0.05f);
                    ImGui::DragFloat("Source Height", &As<gaden::CylinderSource>(params.source)->height, 0.05f);
                }
            }
            ImGui::DragFloat3("Source Position", &sourcePosition.x, 0.05f, 0.0f, 0.0f, "%.2f");
            params.source->sourcePosition = sourcePosition;
            ImGui::SameLine();
            if (ImGui::Button("View in scene"))
                ToggleSceneView();
            DrawSource();

            if (gasType == gaden::GasType::unknown)
            {
                ImGui::ScopedStyle bgstyle(ImGuiCol_FrameBg, Colors::RequiredField);
                ImGui::Combo("Gas Type", (int*)&gasType, ConcatenatedGasTypes().c_str());
            }
            else
                ImGui::Combo("Gas Type", (int*)&gasType, ConcatenatedGasTypes().c_str());
            params.source->gasType = gasType;

            ImGui::VerticalSpace(10);

            // clang-format off
            ImGui::InputFloat("Delta Time",             &params.deltaTime); 
            ImGui::InputFloat("Wind iteration deltaT",  &params.windIterationDeltaTime); 
            ImGui::InputFloat("Temperature (K)",        &params.temperature);                       
            ImGui::InputFloat("Pressure (atm)",         &params.pressure);                             
            ImGui::InputFloat("Filament_ppm_center",    &params.filamentPPMcenter_initial);       
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
            
            ImGui::Checkbox("Save results", &params.saveResults);
            ImGui::InputFloat("Save deltaT",    &params.saveDeltaTime);
            ImGui::Checkbox("Pre-calculate concentration", &params.preCalculateConcentrations);
            ImGui::SameLine();
            ImGui::HelpMarker("This will make the simulation really slow! Don't use it unless you know you need it.");

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

        ImGui::VerticalSpace(10);

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
            ImGui::VerticalSpace(ImGui::GetTextLineHeight());
            ImGui::TextColored(Colors::AsVec(Colors::ErrorText), "Need to run preprocessing before simulations!");
        }
        else
        {
            // keep the same spacing
            ImGui::VerticalSpace(ImGui::GetTextLineHeight());
            ImGui::TextColored(Colors::AsVec(Colors::InfoText), "Ready for simulation");
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

            if (sim.GetCurrentTime() >= simDuration)
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

    gaden::Vector3 sourcePosition;
    int sourceTypeSelected;
    gaden::GasType gasType;

    std::string ConcatenatedNames()
    {
        std::string s = "";
        for (size_t i = 0; i < gaden::GasSource::sourceTypeNames.size(); i++)
            s += gaden::GasSource::sourceTypeNames.at(i) + '\0';
        return s;
    }

    std::string ConcatenatedGasTypes()
    {
        std::string s = "";
        for (size_t i = 0; i < 14; i++)
            s += gaden::to_string(gaden::GasType(i)) + '\0';
        return s;
    }

    void DrawSource()
    {
        if (Is<gaden::PointSource>(params.source))
            g_app->vizScene->DrawSphere(params.source->sourcePosition, 0.1f);
        else if (Is<gaden::SphereSource>(params.source))
            g_app->vizScene->DrawSphere(params.source->sourcePosition, As<gaden::SphereSource>(params.source)->radius);
        else if (Is<gaden::BoxSource>(params.source))
            g_app->vizScene->DrawCube(params.source->sourcePosition, As<gaden::BoxSource>(params.source)->size);
        else if (Is<gaden::LineSource>(params.source))
            g_app->vizScene->DrawLine(params.source->sourcePosition, As<gaden::LineSource>(params.source)->lineEnd, 0.02f);
        else if (Is<gaden::CylinderSource>(params.source))
            g_app->vizScene->DrawCylinder(params.source->sourcePosition,
                                          As<gaden::CylinderSource>(params.source)->radius,
                                          As<gaden::CylinderSource>(params.source)->height);
    }
};