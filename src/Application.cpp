#include "Application.hpp"
#include "Modes/DefaultMode.hpp"
#include "imgui_gl/imgui_gl.h"
#include "imgui_gl/fonts.hpp"
#include <chrono>
#include <gaden/internal/Time.hpp>
#include <thread>

void Application::Run()
{
    ImguiGL imgui;
    imgui.Setup(nullptr,
                "Gaden",
                900,
                900,
                imgui.FlagsFixedLayout() | ImGuiConfigFlags_ViewportsEnable); // enable multi-viewports so we can render the geometry on a popup window!

    // load fonts
    {
        Fonts::body = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)ImGui::Fonts::Roboto_Variable, ImGui::Fonts::Roboto_Variable_size, 18.f);
        Fonts::header = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)ImGui::Fonts::Roboto_Variable, ImGui::Fonts::Roboto_Variable_size, 16.f);
        Fonts::logo = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)ImGui::Fonts::Revalia_Regular, ImGui::Fonts::Revalia_Regular_size, 125.f);
        ImGui::GetIO().Fonts->Build();
    }

    auto defaultMode = std::make_shared<DefaultMode>();
    PushMode(defaultMode);

    gaden::Utils::Time::Clock clock;
    gaden::Utils::Time::TimePoint lastIteration = clock.now();
    while (!shouldClose && !imgui.ShouldClose())
    {
        // calculate deltaTime
        deltaT = gaden::Utils::Time::toSeconds(clock.now()-lastIteration);
        lastIteration = clock.now();
        
        //start rendering
        imgui.StartFrame();
        ImGui::PushFont(Fonts::body);

        imgui.SetNextWindowFullscreen();
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::Background);
        ImGui::Begin("Main", NULL, imgui.FlagsFixedLayout() | ImGuiWindowFlags_NoTitleBar);
        {
            DrawHeader();
            ImGui::VerticalSpace(10.f);
            modeStack.top()->OnGUI();
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopFont();

        imgui.Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    imgui.Close();
}

void Application::PushMode(std::shared_ptr<Mode> mode)
{
    if (!modeStack.empty())
        modeStack.top()->OnLoseFocus();

    modeStack.push(mode);

    modeStack.top()->OnPush();
    modeStack.top()->OnGainFocus();
}

void Application::PopMode()
{
    modeStack.top()->OnLoseFocus();
    modeStack.top()->OnPop();

    modeStack.pop();

    if (!modeStack.empty())
        modeStack.top()->OnGainFocus();
}

std::shared_ptr<Mode> Application::GetCurrentMode()
{
    return modeStack.top();
}

void Application::DrawHeader()
{
    ImGui::PushFont(Fonts::header);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::Header);
    DrawInChildSpanHorizontal("header",
                              {
                                  ImGui::VerticalSpace(10.f);
                                  ImGui::HorizontalSpace(10.f);
                                  ImGui::Text("%s", fmt::format("GadenGUI v{}.{}", 0, 1).c_str());
                                  ImGui::HorizontalSpace(10.f);
                                  ImGui::Text("%s", fmt::format("Gaden v{}.{}", gaden::version_major, gaden::version_minor).c_str());
                                  ImGui::VerticalSpace(10.f);
                              });
    ImGui::PopStyleColor();
    ImGui::PopFont();
}